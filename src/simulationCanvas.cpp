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

  emit drawingFinished(); // invia il segnale di termine disegno
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

  points.append(QPointF(0, 0));
  points.append(QPointF(targetX, targetY));

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

  double R = targetX;
  double Rx = R;
  double Ry = targetY;

  int numPoints = 50;
  points.reserve(numPoints);
  points.append(QPointF(0.0, 0.0)); // punto iniziale

  for (double i = 1; i < numPoints; i += 1.0)
  {
    double t_i = (i * M_PI_2 / numPoints) + M_PI;
    double x = Rx * std::cos(t_i) + Rx;
    double y = -Ry * std::sin(t_i); // il (-) è una correzzione, dovuto a come viene rappresentato l'asse y nel graphics scene
    points.append(QPointF(x, y));
  }

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

  spdlog::info("{} la CICLOIDE al momento non è supportata", stdTAG);
  /*
  spdlog::info(
      "{} Disegnata la CICLOIDE con inizio: {} e fine: {}; utilizzando {} punti",
      stdTAG,
      pointToString(points.first()).toStdString(),
      pointToString(points.back()).toStdString(),
      points.count());
  */

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
      points.append(scenePoint);
      curve.lineTo(scenePoint);
      pathItem->setPath(curve);
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
    if (points[i].x() >= min)
    {
      processedPoints.append(points[i]);
      min = points[i].x();
    }
  }
  points = processedPoints;

  redrawCurve(points);
  spdlog::debug("{} Curva processata, ora sono presenti {} punti", stdTAG, points.size());
}