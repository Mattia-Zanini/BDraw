// cspell:ignore SIMULATIONCANVAS qreal

#include "simulationCanvas.h"

SimulationCanvas::~SimulationCanvas()
{
  clearScene();
  spdlog::debug("{} SimulationCanvas distrutto correttamente", stdTAG);
}

const QString SimulationCanvas::pointToString(const QPointF &p) const
{
  return QString("(x = %1, y = %2)").arg(p.x()).arg(p.y());
}

const QString SimulationCanvas::pointsToString(const QList<QPointF> &pList) const
{
  QString result = QString("");

  for (QPointF p : pList)
  {
    result += pointToString(p);
    result += "\n";
  }

  return result;
}

const double SimulationCanvas::applyScale(const double pixels) const
{
  return pixels / pixelsPerMeter;
}

// Inizializza il widget e configura la scena grafica e la fisica.
SimulationCanvas::SimulationCanvas(QWidget *parent) : QGraphicsView(parent)
{
  // imposto la scena
  myScene = new QGraphicsScene(this);
  setScene(myScene);

  // abilito l'antialiasing per rendere fluide le linee della curva e della pallina
  setRenderHint(QPainter::Antialiasing);

  // disabilito le barre di scorrimento per mantenere la vista fissa sul piano cartesiano
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // definisco l'area logica della scena in modo che sia identica alle dimensioni fisiche attuali della vista in cui viene mostrata
  myScene->setSceneRect(viewport()->rect());
  // imposto l'allineamento della scena in modo tale che l'angolo in alto a sinistra corrisponda al punto (x=0, y=0)
  setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // init delle variabili
  myPen = QPen(Qt::cyan, 2);
  pathItem = nullptr;

  spdlog::debug("{} SimulationCanvas inizializzato correttamente", stdTAG);
}

void SimulationCanvas::clearScene()
{
  points.clear();
  myScene->clear();
  pathItem = nullptr; // resetto il puntatore per non avere un dangling pointer

  spdlog::debug("{} Scena pulita", stdTAG);
}

void SimulationCanvas::redrawCurve(const QList<QPointF> &newPoints)
{
  points = newPoints;
  curve = QPainterPath(); // fa da "contenitore" logico per le operazioni di disegno.

  if (!points.isEmpty())
  {
    curve.moveTo(points.first()); // mi sposto sul primo punto, senza disegnare
    for (int i = 1; i < points.size(); ++i)
      curve.lineTo(points[i]); // traccio un segmento dal punto precedente al successivo
  }

  if (pathItem)
  {
    pathItem->setPath(curve);
    spdlog::debug("{} pathItem settato", stdTAG);
  }
  else
  {
    // creo fisicamente l'elemento applicando il tratto della myPen,
    // trasferisco automaticamente l'ownership alla scena e mi salvo il puntatore
    pathItem = myScene->addPath(curve, myPen);
    spdlog::debug("{} pathItem aggiunto alla scena", stdTAG);
  }
}

void SimulationCanvas::drawLine()
{
  clearScene();

  // Punto finale dinamico basato sul viewport attuale con piccolo margine
  double targetX = viewport()->width() - 40;
  double targetY = viewport()->height() - 40;

  double targetMin = std::min(targetX, targetY);

  points.append(QPointF(0, 0));
  points.append(QPointF(targetMin, targetMin)); // così la retta termina esattamente dove finisce l'arco di circonferenza

  redrawCurve(points);

  spdlog::info(
      "{} Disegnata la RETTA con inizio: {} e fine: {}; utilizzando {} punti",
      stdTAG,
      pointToString(points.first()).toStdString(),
      pointToString(points.back()).toStdString(),
      points.count());

  emit drawingFinished();
}

void SimulationCanvas::drawCircle()
{
  clearScene();

  // Punto finale dinamico basato sul viewport attuale con piccolo margine
  double targetX = viewport()->width() - 40;
  double targetY = viewport()->height() - 40;

  double targetMin = std::min(targetX, targetY);

  double R = targetMin;
  double Rx = targetX;
  double Ry = targetY;

  int numPoints = 50;
  points.reserve(numPoints);
  points.append(QPointF(0.0, 0.0)); // punto iniziale

  for (double i = 1.0; i <= numPoints; i += 1.0)
  {
    double t_i = (i * boost::math::constants::half_pi<double>() / numPoints) + boost::math::constants::pi<double>();
    double x = R * std::cos(t_i) + R;
    double y = -R * std::sin(t_i); // il (-) è una correzione, dovuto a come viene rappresentato l'asse y nel graphics scene
    points.append(QPointF(x, y));
  }

  points.last() = QPointF(R, R);

  redrawCurve(points);

  spdlog::info(
      "{} Disegnato l'ARCO di circonferenza con inizio: {} e fine: {}; utilizzando {} punti",
      stdTAG,
      pointToString(points.first()).toStdString(),
      pointToString(points.back()).toStdString(),
      points.count());

  spdlog::debug("{} points.toString()\n{}", stdTAG, pointsToString(points).toStdString());

  emit drawingFinished();
}

void SimulationCanvas::drawCycloid()
{
  clearScene();

  double targetX = viewport()->width() - 40;
  double targetY = viewport()->height() - 40;

  double targetMin = std::min(targetX, targetY);

  // ho inizializzato A uguale a B, in modo tale che la cicloide
  // termini esattamente dove termina l'arco di circonferenza
  double A = targetMin;
  double B = targetMin;

  double tau = 0.0;
  double c = 0.0;

  // (STEP 1) GESTIONE DEL CASO LIMITE
  // Se B è circa 0, la formula della bisezione esplode (A/B), in questo caso: tau = 2*PI e c = A / (2*PI).
  if (B < threshold)
  {
    tau = boost::math::constants::two_pi<double>();
    c = A / boost::math::constants::two_pi<double>();
  }
  else
  {
    // (STEP 2) CASO GENERALE: risoluzione numerica con TOMS748
    double q = A / B;

    // funzione f da azzerare:
    // phi(t) = ( t - sin(t) ) / ( 1 - cos(t) )
    // f(t) = phi(t) - q
    auto f = [q](double t) -> double
    {
      // dovrò risolvere l'equazione f(t) = 0 -> phi(t) - q = 0
      return (t - std::sin(t)) / (1.0 - std::cos(t)) - q;
    };

    // range di ricerca: sto strettamente dentro (0, 2*PI) per evitare le divisioni per 0 agli estremi
    double left = threshold;
    double right = boost::math::constants::two_pi<double>() - threshold;

    // imposto la precisione numerica
    boost::math::tools::eps_tolerance<double> tolerance(std::numeric_limits<double>::digits);
    std::uintmax_t maxIteration = 50;

    try
    {
      // eseguo TOMS748
      std::pair<double, double> result = boost::math::tools::toms748_solve(f, left, right, tolerance, maxIteration);

      // estraggo tau come punto medio del minuscolo intervallo risultante
      tau = (result.first + result.second) / 2.0;

      // ottenuto tau, ricavo c dalla seconda equazione parametrica
      c = B / (1.0 - std::cos(tau));
    }
    catch (const std::exception &e)
    {
      spdlog::error("{} Errore durante il calcolo di TOMS748: {}", stdTAG, e.what());
      emit drawingFinished();
      return;
    }
  }

  // (STEP 3) GENERAZIONE DEI PUNTI DELLA CURVA
  int numPoints = 50;
  points.reserve(numPoints);
  points.append(QPointF(0.0, 0.0));

  for (double i = 1.0; i <= numPoints; i += 1)
  {
    double t_i = (i / numPoints) * tau;

    double x = c * (t_i - std::sin(t_i));
    double y = c * (1.0 - std::cos(t_i));

    points.append(QPointF(x, y));
  }

  // correzione del punto finale
  points.last() = QPointF(A, B);

  redrawCurve(points);

  spdlog::info(
      "{} Disegnata la CICLOIDE (tau = {}, c = {}) con inizio: {} e fine: {}; utilizzando {} punti",
      stdTAG, tau, c,
      pointToString(points.first()).toStdString(),
      pointToString(points.back()).toStdString(),
      points.count());

  emit drawingFinished();
}

// GESTIONE DEL CLICK
void SimulationCanvas::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (points.isEmpty())
    {
      spdlog::debug("{} Inizio di un nuovo disegno", stdTAG);
      QPointF scenePoint = mapToScene(event->pos()); // converto la posizione del click dalla vista alla scena
      isDrawing = true;

      curve = QPainterPath();
      points.append(QPointF(0, 0)); // il primo punto deve essere SEMPRE l'origine
      curve.moveTo(points.first());

      points.append(scenePoint);
      curve.lineTo(points[1]); // disegno il tratto che va dall'origine al primo punto
      pathItem = myScene->addPath(curve, myPen);
    }
    else
    {
      spdlog::debug("{} Per disegnare un nuovo percorso bisogna prima pulire la scena", stdTAG);
    }
  }

  // è buona norma richiamare l'evento della classe base per non bloccare altri comportamenti di default
  QGraphicsView::mousePressEvent(event);
}

// GESTIONE DEL MOVIMENTO
void SimulationCanvas::mouseMoveEvent(QMouseEvent *event)
{
  // verifico se il tasto sinistro è attualmente ancora premuto
  if (event->buttons() & Qt::LeftButton)
  {
    if (isDrawing && pathItem)
    {
      QPointF scenePoint = mapToScene(event->pos());
      // Controllo che i punti siano almeno un pochino distanziati fra di loro
      if (std::hypot(scenePoint.x() - points.last().x(), scenePoint.y() - points.last().y()) > minMoveDistance)
      {
        points.append(scenePoint);
        curve.lineTo(scenePoint);
        pathItem->setPath(curve);
      }
    }
  }

  QGraphicsView::mouseMoveEvent(event);
}

// GESTIONE DEL RILASCIO
void SimulationCanvas::mouseReleaseEvent(QMouseEvent *event)
{
  // verifico se è stato rilasciato il tasto
  if (event->button() == Qt::LeftButton && isDrawing)
  {
    isDrawing = false; // il disegno è terminato
    spdlog::debug("{} Disegno terminato", stdTAG);
    postProcessingCurve();

    emit drawingFinished();
  }

  QGraphicsView::mouseReleaseEvent(event);
}

void SimulationCanvas::postProcessingCurve()
{
  if (points.size() < 2) // non serve processare 1 punto solo (o 0)
    return;

  spdlog::debug("{} PRE: sono presenti {} punti", stdTAG, points.size());
  QList<QPointF> processedPoints;
  processedPoints.append(points.first());

  qreal min = 0;
  for (int i = 1; i < points.size(); i++)
  {
    if (points[i].x() >= min && points[i].y() >= 0)
    {
      processedPoints.append(points[i]);
      min = points[i].x();
    }
  }
  points = processedPoints;

  redrawCurve(points);
  spdlog::debug("{} Curva processata, ora sono presenti {} punti", stdTAG, points.size());
}

const double SimulationCanvas::computeTheoreticalTime() const
{
  double totalTime = 0.0;

  if (points.count() < 2)
    return 0.0;

  double yStart = points[0].y(); // non applico la scala, perché il primo punto è sempre l'origine

  for (int i = 0; i < points.count() - 1; i++)
  {
    double x1 = applyScale(points[i].x());
    double y1 = applyScale(points[i].y());
    double x2 = applyScale(points[i + 1].x());
    double y2 = applyScale(points[i + 1].y());

    // pitagora
    double d = std::hypot(x2 - x1, y2 - y1);

    // VELOCITA': sqrt(2 * g * Δy), dalla conservazione dell'energia
    // il max, serve per proteggere i calcoli in caso di valori negativi
    double v1 = std::sqrt(std::max(0.0, 2.0 * gravity) * (y1 - yStart)); // sqrt[2g(y1 - 0)] = ⎷(2g*y1)
    double v2 = std::sqrt(std::max(0.0, 2.0 * gravity) * (y2 - yStart)); // sqrt(2g*y2)

    // v1: la velocità con cui la pallina entra in quel segmento (basata su quanto è scesa dall'inizio fino al punto 1)
    // v2: la velocità con cui la pallina esce da quel segmento (basata su quanto è scesa dall'inizio fino al punto 2)

    // siccome il segmento è una linea retta, la pallina ha aumentato la sua velocità in modo perfettamente regolare
    double vMedia = (v1 + v2) / 2.0;

    // l'if serve per evitare divisioni per 0 o per valori impercettibili
    if ((v1 + v2) > threshold)
      totalTime += d / vMedia; // tempo = distanza / velocità
    else
    {
      // se il programma entra in questo else, significa una cosa sola: sono all'inizio del percorso e la pallina
      // sta partendo da ferma. Non posso usare la velocità per calcolare il tempo (perché non ce n'è ancora!)

      double dy = y2 - y1;
      if (dy > threshold) // se non rispetta l'if allora sto aggiungendo 0.0 al tempo
        // è la soluzione cinematica esatta per un corpo che scivola su un piano inclinato partendo da fermo
        totalTime += std::sqrt(2.0 * (d * d) / (gravity * dy));
    }
  }

  spdlog::debug("{} Calcolato il tempo teorico (scala 1:{}): {} s", stdTAG, pixelsPerMeter, totalTime);

  return totalTime;
}