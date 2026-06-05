// cspell:ignore SIMULATIONCANVAS qreal

#include "simulationCanvas.h"

SimulationCanvas::~SimulationCanvas()
{
  clearScene();
  spdlog::debug("{} SimulationCanvas distrutto correttamente", logTag);
}

// Inizializza il widget e configura la scena grafica e la fisica.
SimulationCanvas::SimulationCanvas(QWidget *parent) : QGraphicsView(parent)
{
  // imposto la scena
  scene = new QGraphicsScene(this);
  setScene(scene);

  // abilito l'antialiasing per rendere fluide le linee della curva e della pallina
  setRenderHint(QPainter::Antialiasing);

  // disabilito le barre di scorrimento per mantenere la vista fissa sul piano cartesiano
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // definisco l'area logica della scena in modo che sia identica alle dimensioni fisiche attuali della vista in cui viene mostrata
  scene->setSceneRect(viewport()->rect());
  // imposto l'allineamento della scena in modo tale che l'angolo in alto a sinistra corrisponda al punto (x=0, y=0)
  setAlignment(Qt::AlignLeft | Qt::AlignTop);

  // init delle variabili
  isUserDrawing = false;
  pen = QPen(Qt::cyan, 2);
  state = arma::vec2(arma::fill::zeros);
  curveItem = nullptr;
  ballItem = new QGraphicsEllipseItem(0, 0, ballRadius * 2, ballRadius * 2);
  ballItem->setBrush(QBrush(Qt::white)); // Pallina bianca
  ballItem->setZValue(100);
  ballItem->hide();
  scene->addItem(ballItem);
  metersPerPixel = 0.01;
  totSimulationSeconds = 0.0;
  simulationClock = new QTimer(this);
  connect(simulationClock, &QTimer::timeout, this, &SimulationCanvas::updatePhysics); // connetto il clock all'update della simulazione

  spdlog::debug("{} SimulationCanvas inizializzato correttamente", logTag);
}

const double SimulationCanvas::getSimulationTime() const { return totSimulationSeconds; }

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
  // più metersPerPixel è grande, più metri corrispondono a quel pixel
  return pixels * metersPerPixel;
}

void SimulationCanvas::clearScene()
{
  points.clear();
  simulationClock->stop();
  totSimulationSeconds = 0.0;
  cumulativeDistance.clear();
  scene->removeItem(ballItem); // tolgo la pallina dalla scena senza distruggerla
  scene->clear();              // cancello in sicurezza tutti gli altri elementi
  scene->addItem(ballItem);    // reinserisco la palla nella scena per la prossima simulazione
  ballItem->hide();
  curveItem = nullptr;
  spdlog::debug("{} Scena pulita", logTag);
}

const double SimulationCanvas::clampDistance(const double s) const
{
  DEBUG_ASSERT(cumulativeDistance.size() != 0, "Deve esistere almeno un segmento");
  return std::clamp(s, 0.0, cumulativeDistance.back());
}

const double SimulationCanvas::getScaledPointsDistance(const QPointF &p1, const QPointF &p2) const
{
  double x1 = applyScale(p1.x());
  double y1 = applyScale(p1.y());
  double x2 = applyScale(p2.x());
  double y2 = applyScale(p2.y());

  return std::hypot(x2 - x1, y2 - y1);
}

void SimulationCanvas::computeCumulativeDistance()
{
  if (points.count() < 2)
    return;

  DEBUG_ASSERT(cumulativeDistance.empty() == true, "il vettore delle distanze cumulative deve essere pulito prima di calcolarne di nuove");
  cumulativeDistance.reserve(points.count() - 1); // da n punti ottengo n-1 segmenti
  double s = 0.0;

  for (int i = 1; i < points.count(); i++)
  {
    s += getScaledPointsDistance(points[i - 1], points[i]);
    cumulativeDistance.push_back(s);
  }
}

const int SimulationCanvas::getSegmentIndex(double s) const
{
  DEBUG_ASSERT(cumulativeDistance.size() > 0, "Deve essere presente almeno un segmento"); // 1 segmento => 2 punti
  auto it = std::upper_bound(cumulativeDistance.begin(), cumulativeDistance.end(), s);    // elemento per cui il valore è < s (minore STRETTO)
  int i = std::distance(cumulativeDistance.begin(), it);                                  // posizione del segmento nel vettore

  if (i >= cumulativeDistance.size())
    i = cumulativeDistance.size() - 1;

  DEBUG_ASSERT(i >= 0 && i < cumulativeDistance.size(), "L'index del segmento ottenuto è fuori dal range dei valori validi", i, cumulativeDistance.size(), cumulativeDistance, s);
  return i;
}

const double SimulationCanvas::getSineAt(double s) const
{
  DEBUG_ASSERT(cumulativeDistance.size() > 0, "Deve essere presente almeno un segmento");

  int index = getSegmentIndex(s);
  double dx = points[index + 1].x() - points[index].x();
  double dy = points[index + 1].y() - points[index].y();
  double ds = std::hypot(dx, dy);

  DEBUG_ASSERT(ds > 0, "Segmento di lunghezza nulla");
  return dy / ds;
}

void SimulationCanvas::redrawCurve(const QList<QPointF> &newPoints)
{
  points = newPoints;
  curve = QPainterPath();

  if (!points.isEmpty())
  {
    curve.moveTo(points.first()); // mi sposto sul primo punto, senza disegnare
    for (int i = 1; i < points.size(); ++i)
      curve.lineTo(points[i]); // traccio un segmento dal punto precedente al successivo
  }

  if (curveItem)
  {
    curveItem->setPath(curve);
    spdlog::debug("{} curveItem settato", logTag);
  }
  else
  {
    // creo fisicamente l'elemento applicando il tratto della myPen,
    // trasferisco automaticamente l'ownership alla scena e mi salvo il puntatore
    curveItem = scene->addPath(curve, pen);
    spdlog::debug("{} curveItem aggiunto alla scena", logTag);
  }
}

void SimulationCanvas::drawLine()
{
  clearScene();

  // Punto finale dinamico basato sul viewport attuale con piccolo margine
  double targetX = viewport()->width() - margin;
  double targetY = viewport()->height() - margin;

  double targetMin = std::min(targetX, targetY);

  points.append(QPointF(0, 0));
  points.append(QPointF(targetMin, targetMin)); // così la retta termina esattamente dove finisce l'arco di circonferenza

  redrawCurve(points);

  spdlog::info(
      "{} Disegnata la RETTA con inizio: {} e fine: {}; utilizzando {} punti",
      logTag,
      pointToString(points.first()).toStdString(),
      pointToString(points.back()).toStdString(),
      points.count());

  computeCumulativeDistance();

  emit drawingFinished();
}

void SimulationCanvas::drawCircle()
{
  clearScene();

  // Punto finale dinamico basato sul viewport attuale con piccolo margine
  double targetX = viewport()->width() - margin;
  double targetY = viewport()->height() - margin;

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
      logTag,
      pointToString(points.first()).toStdString(),
      pointToString(points.back()).toStdString(),
      points.count());

  spdlog::debug("{} points.toString()\n{}", logTag, pointsToString(points).toStdString());

  computeCumulativeDistance();

  emit drawingFinished();
}

void SimulationCanvas::drawCycloid()
{
  clearScene();

  double targetX = viewport()->width() - margin;
  double targetY = viewport()->height() - margin;

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
      spdlog::error("{} Errore durante il calcolo di TOMS748: {}", logTag, e.what());
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
      logTag, tau, c,
      pointToString(points.first()).toStdString(),
      pointToString(points.back()).toStdString(),
      points.count());

  computeCumulativeDistance();

  emit drawingFinished();
}

// GESTIONE DEL CLICK
void SimulationCanvas::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (points.isEmpty())
    {
      spdlog::debug("{} Inizio di un nuovo disegno", logTag);
      QPointF scenePoint = mapToScene(event->pos()); // converto la posizione del click dalla vista alla scena
      isUserDrawing = true;

      curve = QPainterPath();
      points.append(QPointF(0, 0)); // il primo punto deve essere SEMPRE l'origine
      curve.moveTo(points.first());

      points.append(scenePoint);
      curve.lineTo(points[1]); // disegno il tratto che va dall'origine al primo punto
      curveItem = scene->addPath(curve, pen);
    }
    else
    {
      spdlog::debug("{} Per disegnare un nuovo percorso bisogna prima pulire la scena", logTag);
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
    if (isUserDrawing && curveItem)
    {
      QPointF scenePoint = mapToScene(event->pos());
      // Controllo che i punti siano almeno un pochino distanziati fra di loro
      if (std::hypot(scenePoint.x() - points.last().x(), scenePoint.y() - points.last().y()) > minMoveDistance)
      {
        points.append(scenePoint);
        curve.lineTo(scenePoint);
        curveItem->setPath(curve);
      }
    }
  }

  QGraphicsView::mouseMoveEvent(event);
}

// GESTIONE DEL RILASCIO
void SimulationCanvas::mouseReleaseEvent(QMouseEvent *event)
{
  // verifico se è stato rilasciato il tasto
  if (event->button() == Qt::LeftButton && isUserDrawing)
  {
    isUserDrawing = false; // il disegno è terminato
    spdlog::debug("{} Disegno terminato", logTag);
    postProcessingCurve();
    computeCumulativeDistance();

    emit drawingFinished();
  }

  QGraphicsView::mouseReleaseEvent(event);
}

void SimulationCanvas::postProcessingCurve()
{
  if (points.size() < 2) // non serve processare 1 punto solo (o 0)
    return;

  spdlog::debug("{} PRE: sono presenti {} punti", logTag, points.size());
  QList<QPointF> processedPoints;
  processedPoints.append(points.first());

  qreal min = 0;
  for (int i = 1; i < points.size(); i++)
  {
    if (points[i].x() >= min && points[i].y() >= threshold)
    {
      processedPoints.append(points[i]);
      min = points[i].x();
    }
  }
  points = processedPoints;

  redrawCurve(points);
  spdlog::debug("{} Curva processata, ora sono presenti {} punti", logTag, points.size());
}

const double SimulationCanvas::computeTheoreticalTime() const
{
  double totalTime = 0.0;

  if (points.count() < 2)
    return 0.0;

  double yStart = applyScale(points[0].y());

  for (int i = 0; i < points.count() - 1; i++)
  {
    double y1 = applyScale(points[i].y());
    double y2 = applyScale(points[i + 1].y());

    // pitagora
    double d = getScaledPointsDistance(points[i], points[i + 1]);

    // VELOCITA': sqrt(2 * g * Δy), dalla conservazione dell'energia
    // il max, serve per proteggere i calcoli in caso di valori negativi
    double v1 = std::sqrt(std::max(0.0, 2.0 * gravity * (y1 - yStart))); // sqrt[2g(y1 - 0)] = ⎷(2g*y1)
    double v2 = std::sqrt(std::max(0.0, 2.0 * gravity * (y2 - yStart))); // sqrt(2g*y2)

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

  spdlog::debug("{} Calcolato il tempo teorico (scala 1:{}): {} s", logTag, metersPerPixel, totalTime);

  return totalTime;
}

void SimulationCanvas::drawRedDot(bool show)
{
  showTarget = show;
  viewport()->update(); // richiede un aggiornamento grafico della vista
}

void SimulationCanvas::drawBackground(QPainter *painter, const QRectF &rect)
{
  QGraphicsView::drawBackground(painter, rect); // esegue il disegno di sfondo predefinito

  if (showTarget)
  {
    double targetX = viewport()->width() - margin;
    double targetY = viewport()->height() - margin;
    double targetMin = std::min(targetX, targetY);

    painter->setBrush(QBrush(Qt::red));
    painter->setPen(Qt::NoPen); // no bordo

    // disegna il cerchio rosso centrato su targetMin con raggio di 5px
    painter->drawEllipse(QPointF(targetMin, targetMin), 5.0, 5.0);
  }
}

void SimulationCanvas::startSimulation()
{
  if (points.count() < 2) // non esiste nemmeno un segmento
  {
    emit simulationFinished();
    return;
  }
  spdlog::info("{} Simulazione avviata", logTag, totSimulationSeconds);

  simulationClock->stop();
  state.zeros();
  updateBallPosition(0.0);
  ballItem->show();
  totSimulationSeconds = 0.0;
  double curveTotalLength = cumulativeDistance.back();

  spdlog::debug("{} punti della curva\n{}", logTag, pointsToString(points).toStdString());

  // avvio i timer e il clock
  simulationClock->start(deltaTimeMilliseconds);
  totalSimulationTime.start();
  elapsedTime.start();
}

void SimulationCanvas::updatePhysics()
{
  double dt = elapsedTime.restart() / 1000.0; // calcolo del delta-time reale, in secondi

  // protezione contro lag improvvisi del sistema ( > 50 ms )
  if (dt > 0.05) // persi più di 3 frame
    dt = deltaTimeSeconds;

  double sine = getSineAt(clampDistance(state(0))); // sin(s(k))
  arma::mat A = {{1.0, dt},
                 {0.0, 1}};
  arma::vec2 B = {0.0, dt * sine}; // B = [ 0, dt * seno ]^T
  double u = gravity; // vettore d'ingresso u(k) = g

  // aggiorno lo stato del sistema, calcolo x(k + 1) = A * x(k) + B * u(k)
  state = A * state + B * u;

  state(0) = clampDistance(state(0));
  spdlog::debug("{} x(k + 1) = [{}, {}]^T , sine: {}", logTag, state(0), state(1), sine);
  double s = state(0);
  double velocity = state(1);
  double L = cumulativeDistance.back();

  updateBallPosition(s);

  if (s == L) // fine della simulazione
  {
    simulationClock->stop();
    totSimulationSeconds = totalSimulationTime.elapsed() / 1000.0;
    spdlog::info("{} Simulazione terminata in {} s", logTag, totSimulationSeconds);
    emit simulationFinished();
  }
}

void SimulationCanvas::updateBallPosition(const double s)
{
  // CALCOLO DELLA NORMALE ALLA CURVA
  arma::vec2 n; // vettore normale
  int indexSegment = getSegmentIndex(s);
  arma::vec2 pointA = {points[indexSegment].x(), points[indexSegment].y()};
  arma::vec2 pointB = {points[indexSegment + 1].x(), points[indexSegment + 1].y()};
  arma::vec2 v = pointB - pointA; // vettore direzione

  double dx = v(0);
  double dy = v(1);

  // seleziono la normale in base al segno di dx
  if (dx >= 0.0) // segmento verso destra o perfettamente verticale
    n = {dy, -dx};
  else // segmento verso sinistra
    n = {-dy, dx};

  n = arma::normalise(n); // normalizzo il vettore

  // UPDATE GRAFICO DELLA PALLA
  // calcolo dell'offset (raggio R + 1px) per poggiare SOPRA la curva
  double offsetX = n(0) * (ballRadius + 1);
  double offsetY = n(1) * (ballRadius + 1);

  // aggiornamento della posizione visiva
  double percent = curve.percentAtLength(s / metersPerPixel);
  QPointF pos = curve.pointAtPercent(percent); // ottengo la posizione (x,y) della pallina sulla curva (senza offset)
  ballItem->setPos(pos.x() + offsetX - ballRadius, pos.y() + offsetY - ballRadius);
}