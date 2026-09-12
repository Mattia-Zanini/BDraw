// cspell:ignore SIMULATIONCANVAS qreal

#include "simulationCanvas.h"
#include "curveUtils.h"

namespace BDraw {

  // Inizializza il widget e configura la scena grafica e la fisica.
  SimulationCanvas::SimulationCanvas(QWidget* parent) : QGraphicsView(parent) {
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
    pen = QPen(Qt::blue);
    pen.setWidthF(2.5);
    bestPen = QPen(QColor(0, 152, 13));
    bestPen.setWidthF(2.5);
    state = arma::vec2(arma::fill::zeros);
    curve = QPainterPath();

    ballItem = new QGraphicsEllipseItem(0, 0, Constants::ballRadius * 2, Constants::ballRadius * 2);
    ballItem->setBrush(QBrush(Qt::white)); // Pallina bianca
    ballItem->setZValue(100);
    ballItem->hide();
    scene->addItem(ballItem);

    ballOptimal = new QGraphicsEllipseItem(0, 0, Constants::ballRadius * 2, Constants::ballRadius * 2);
    ballOptimal->setBrush(QBrush(Qt::white)); // Pallina bianca
    ballOptimal->setZValue(-1);
    ballOptimal->hide();
    scene->addItem(ballOptimal);

    simulationClock = new QTimer(this);

    // connetto il clock all'update della simulazione
    connect(simulationClock, &QTimer::timeout, this, &SimulationCanvas::updatePhysics);

    spdlog::debug("{} SimulationCanvas inizializzato correttamente", logTag);
  }

  SimulationCanvas::~SimulationCanvas() {
    clearScene();
    spdlog::debug("{} SimulationCanvas distrutto correttamente", logTag);
  }

  void SimulationCanvas::redrawCurve(const QList<QPointF>& newPoints) {
    points = newPoints;
    curve.clear();

    if (!points.isEmpty()) {
      curve.moveTo(points.first()); // mi sposto sul primo punto, senza disegnare
      for (int i = 1; i < points.size(); ++i)
        curve.lineTo(points[i]); // traccio un segmento dal punto precedente al successivo
    }

    if (curveItem) {
      curveItem->setPath(curve);
      curveItem->setZValue(0);
      spdlog::debug("{} curveItem settato", logTag);
    } else {
      // creo fisicamente l'elemento applicando il tratto della myPen,
      // trasferisco automaticamente l'ownership alla scena e mi salvo il puntatore
      curveItem = scene->addPath(curve, pen);
      curveItem->setZValue(0);
      spdlog::debug("{} curveItem aggiunto alla scena", logTag);
    }
  }

  void SimulationCanvas::clearScene() {
    points.clear();
    optimalCurvePoints.clear();
    isCycloid = false;
    simulationClock->stop();
    totSimulationSeconds = 0.0;
    mainSimulationSeconds = 0.0;
    optimalSimulationSeconds = 0.0;
    physicsAccumulator = 0.0;
    cumulativeDistance.clear();
    cumulativeDistanceOptimal.clear();

    scene->removeItem(ballItem); // tolgo la pallina dalla scena senza distruggerla
    scene->removeItem(ballOptimal);
    scene->clear();           // cancello in sicurezza tutti gli altri elementi
    scene->addItem(ballItem); // reinserisco la palla nella scena per la prossima simulazione
    scene->addItem(ballOptimal);
    ballItem->hide();
    ballOptimal->hide();

    curveItem = nullptr;
    optimalCurveItem = nullptr;
    spdlog::debug("{} Scena pulita", logTag);
  }

  void SimulationCanvas::drawLine() {
    clearScene();

    // Punto finale dinamico basato sul viewport attuale con piccolo margine
    points = CurveUtils::generateLinePoints(viewport()->width() - Constants::margin,
                                            viewport()->height() - Constants::margin);

    redrawCurve(points);

    spdlog::info("{} Disegnata la RETTA con inizio: {} e fine: {}; utilizzando {} punti", logTag, CurveUtils::pointToString(points.first()), CurveUtils::pointToString(points.back()), points.count());

    CurveUtils::computeCumulativeDistance(points, cumulativeDistance, metersPerPixel);
    isCycloid = false;
    updateOptimalCurve();

    emit drawingFinished();
  }

  void SimulationCanvas::drawCycloid() {
    clearScene();

    const double targetX = viewport()->width() - Constants::margin;
    const double targetY = viewport()->height() - Constants::margin;
    const double targetMin = std::min(targetX, targetY);

    const QPointF target(targetMin, targetMin);
    points = CurveUtils::generateCycloidPoints(target,
                                               QPointF(0, 0),
                                               getScaledSampleCount());

    redrawCurve(points);

    spdlog::info("{} Disegnata la CICLOIDE con inizio: {} e fine: {}; utilizzando {} punti", logTag, CurveUtils::pointToString(points.first()), CurveUtils::pointToString(points.back()), points.count());

    CurveUtils::computeCumulativeDistance(points, cumulativeDistance, metersPerPixel);
    isCycloid = true;
    updateOptimalCurve();
    emit drawingFinished();
  }

  void SimulationCanvas::drawCircle() {
    clearScene();

    // Punto finale dinamico basato sul viewport attuale con piccolo margine
    const double targetX = viewport()->width() - Constants::margin;
    const double targetY = viewport()->height() - Constants::margin;

    const double targetMin = std::min(targetX, targetY);

    points = CurveUtils::generateCirclePoints(targetMin, getScaledSampleCount());
    redrawCurve(points);

    spdlog::info("{} Disegnato l'ARCO di circonferenza con inizio: {} e fine: {}; utilizzando {} punti", logTag, CurveUtils::pointToString(points.first()), CurveUtils::pointToString(points.back()), points.count());

    CurveUtils::computeCumulativeDistance(points, cumulativeDistance, metersPerPixel);
    isCycloid = false;
    updateOptimalCurve();

    emit drawingFinished();
  }

  const double SimulationCanvas::computeTheoreticalTime(const QList<QPointF>& customPoints) const {
    return CurveUtils::computeTheoreticalTime(customPoints, metersPerPixel, Constants::gravity);
  }

  void SimulationCanvas::startSimulation() {
    if (points.count() < 2) // non esiste nemmeno un segmento
    {
      emit simulationFinished();
      return;
    }
    spdlog::info("{} Simulazione avviata", logTag, totSimulationSeconds);

    simulationClock->stop();
    state.zeros();
    stateOptimal.zeros();
    updateBallPosition(0.0, ballItem, curve, points, cumulativeDistance);

    // se vengono disegnate curve che hanno il punto finale più in alto del punto iniziale allora la cicloide non viene generata.
    if (!optimalCurvePoints.isEmpty()) {
      updateBallPosition(0.0, ballOptimal, optimalPath, optimalCurvePoints, cumulativeDistanceOptimal);
      optimalBallFinished = false;
    } else
      optimalBallFinished = true;

    mainBallFinished = false;
    ballItem->show();
    totSimulationSeconds = 0.0;
    mainSimulationSeconds = 0.0;
    optimalSimulationSeconds = 0.0;
    physicsAccumulator = 0.0;
    double curveTotalLength = cumulativeDistance.back();

    spdlog::debug("{} Dati inerenti alle curve", logTag);
    spdlog::debug("{} Punti della curva custom da simulare:\n{}", logTag, CurveUtils::pointsToString(points));
    spdlog::debug("{} Punti della curva ottima:\n{}", logTag, CurveUtils::pointsToString(optimalCurvePoints));

    // avvio i timer e il clock
    simulationClock->start(Constants::deltaTimeMilliseconds);
    elapsedTime.start();
  }

  void SimulationCanvas::setMetersPerPixel(double val) {
    DEBUG_ASSERT(val > 0.0, "Il fattore di scala metersPerPixel deve essere strettamente positivo", val);
    metersPerPixel = val;
    spdlog::debug("{} metersPerPixel impostato a: {} ({} px/m)", logTag, val, 1.0 / val);

    if (hasCurve()) {
      cumulativeDistance.clear();
      CurveUtils::computeCumulativeDistance(points, cumulativeDistance, metersPerPixel);
    }
  }

  bool SimulationCanvas::hasCurve() const { return points.count() >= 2; }

  const double SimulationCanvas::getCurveLength() const {
    if (cumulativeDistance.empty())
      return 0.0;
    return cumulativeDistance.back();
  }

  const QPointF SimulationCanvas::getEndPoint() const {
    if (points.isEmpty())
      return QPointF(0.0, 0.0);
    return points.last();
  }

  const double SimulationCanvas::computeBestTheoreticalTime(const QPointF& target) const {
    if (isCycloid)
      return computeTheoreticalTime(points);

    if (hasCurve() == false)
      return 0.0;

    return CurveUtils::computeBestTheoreticalTime(target, points.first(), getScaledSampleCount(), metersPerPixel, Constants::gravity);
  }

  void SimulationCanvas::drawCurveFromFormula(const QString& formulaStr) {
    clearScene();

    const int numPoints = getScaledSampleCount();
    spdlog::debug("{} initWidth: {}", logTag, initWidth);
    spdlog::debug("{} viewport width: {}", logTag, (double)viewport()->width());
    spdlog::debug("{} numPoints: {}", logTag, numPoints);

    const double targetX = viewport()->width() - Constants::margin;
    const double limitY = viewport()->height() * 3.0;

    points = CurveUtils::drawCurveFromFormula(formulaStr, numPoints, targetX, limitY);

    points = CurveUtils::postProcessingCurve(points, viewport()->width() - Constants::threshold, -1.0 * Constants::margin);
    if (points.size() < 2) {
      clearScene();
      return;
    }

    redrawCurve(points);
    CurveUtils::computeCumulativeDistance(points, cumulativeDistance, metersPerPixel);
    updateOptimalCurve();

    if (points.isEmpty())
      spdlog::warn("{} La formula ha generato punti completamente fuori dai limiti del canvas o non validi", logTag);
    else
      spdlog::info("{} Curva generata da equazione con {} punti; punto iniziale: {}; punto finale: {}", logTag, points.count(), CurveUtils::pointToString(points.first()), CurveUtils::pointToString(points.last()));

    emit drawingFinished();
  }

  void SimulationCanvas::drawRedDot(bool show) {
    showTarget = show;
    spdlog::debug("{} Target visibile: {}", logTag, show);
    viewport()->update(); // richiede un aggiornamento grafico della vista
  }

  void SimulationCanvas::setShowOptimal(bool show) {
    showOptimal = show;
    spdlog::debug("{} Mostra curva ottimale impostato a: {}", logTag, show);
    updateOptimalCurve();
  }

  // GESTIONE DEL CLICK
  void SimulationCanvas::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
      if (points.isEmpty()) {
        spdlog::debug("{} Inizio di un nuovo disegno", logTag);
        QPointF scenePoint = mapToScene(event->pos()); // converto la posizione del click dalla vista alla scena
        isUserDrawing = true;

        curve = QPainterPath();
        points.append(QPointF(0, 0)); // il primo punto deve essere SEMPRE l'origine
        curve.moveTo(points.first());

        points.append(scenePoint);
        curve.lineTo(points[1]); // disegno il tratto che va dall'origine al primo punto
        curveItem = scene->addPath(curve, pen);
      } else {
        spdlog::debug("{} Per disegnare un nuovo percorso bisogna prima pulire la scena", logTag);
      }
    }

    // è buona norma richiamare l'evento della classe base per non bloccare altri comportamenti di default
    QGraphicsView::mousePressEvent(event);
  }

  // GESTIONE DEL MOVIMENTO
  void SimulationCanvas::mouseMoveEvent(QMouseEvent* event) {
    // verifico se il tasto sinistro è attualmente ancora premuto
    if (event->buttons() & Qt::LeftButton) {
      if (isUserDrawing && curveItem) {
        QPointF scenePoint = mapToScene(event->pos());
        // Controllo che i punti siano almeno un pochino distanziati fra di loro
        if (std::hypot(scenePoint.x() - points.last().x(), scenePoint.y() - points.last().y()) > Constants::minMoveDistance) {
          points.append(scenePoint);
          curve.lineTo(scenePoint);
          curveItem->setPath(curve);
        }
      }
    }

    QGraphicsView::mouseMoveEvent(event);
  }

  // GESTIONE DEL RILASCIO
  void SimulationCanvas::mouseReleaseEvent(QMouseEvent* event) {
    // verifico se è stato rilasciato il tasto
    if (event->button() == Qt::LeftButton && isUserDrawing) {
      isUserDrawing = false; // il disegno è terminato
      spdlog::debug("{} Disegno terminato", logTag);

      isCycloid = false;
      points = CurveUtils::postProcessingCurve(points, viewport()->width() - Constants::threshold, -1.0 * Constants::margin);
      if (points.size() < 2) {
        clearScene();
        return;
      }

      QList<QPointF> newPoints = CurveUtils::upsampleDrawnCurve(points, getScaledSampleCount());
      redrawCurve(newPoints);

      CurveUtils::computeCumulativeDistance(points, cumulativeDistance, metersPerPixel);
      updateOptimalCurve();

      emit drawingFinished();
    }

    QGraphicsView::mouseReleaseEvent(event);
  }

  void SimulationCanvas::drawBackground(QPainter* painter, const QRectF& rect) {
    QGraphicsView::drawBackground(painter, rect); // esegue il disegno di sfondo predefinito

    if (showTarget) {
      double targetX = viewport()->width() - Constants::margin;
      double targetY = viewport()->height() - Constants::margin;
      double targetMin = std::min(targetX, targetY);

      painter->setBrush(QBrush(Qt::red));
      painter->setPen(Qt::NoPen); // no bordo

      // disegna il cerchio rosso centrato su targetMin con raggio di 5px
      painter->drawEllipse(QPointF(targetMin, targetMin), 5.0, 5.0);
    }
  }

  void SimulationCanvas::resizeEvent(QResizeEvent* event) {
    QGraphicsView::resizeEvent(event);
    if (initWidth == 0 && viewport()->width() > 0)
      initWidth = viewport()->width();
  }

  void SimulationCanvas::updatePhysics() {
    double frameTime = elapsedTime.restart() / 1000.0; // calcolo del delta-time reale, in secondi

    // protezione contro lag improvvisi del sistema (visivamente)
    if (frameTime > Constants::maxTimeElapsed)
      frameTime = Constants::deltaTimeSeconds;

    physicsAccumulator += frameTime;

    double sine = 0.0;
    double sineOpt = 0.0;

    // Consuma il tempo a passi RIGIDAMENTE FISSI e COSTANTI
    while (physicsAccumulator >= Constants::fixedSubDT) {
      stepSymplecticEuler(Constants::gravity, sine, sineOpt);
      physicsAccumulator -= Constants::fixedSubDT;
      totSimulationSeconds += Constants::fixedSubDT;
    }

    bool illegalStateMain = state(0) < -0.01;
    bool illegalStateOptimal = (!optimalCurvePoints.isEmpty()) ? (stateOptimal(0) < -0.01) : false;

    // se la pallina torna indietro oltre l'inizio della curva termino la simulazione
    if (illegalStateMain || illegalStateOptimal) {
      mainBallFinished = true;
      optimalBallFinished = true;

      spdlog::warn("{} Una delle palline è tornata indietro oltre l'inizio della curva, termino la simulazione", logTag);
      simulationClock->stop();
      mainSimulationSeconds = totSimulationSeconds;
      optimalSimulationSeconds = totSimulationSeconds;
      emit simulationFinished();
      return;
    }

    state(0) = CurveUtils::clampDistance(state(0), cumulativeDistance);
    double s = state(0);
    double L = cumulativeDistance.back();
    updateBallPosition(s, ballItem, curve, points, cumulativeDistance);

    double sOpt = 0.0;
    double LOpt = 0.0;
    if (!optimalCurvePoints.isEmpty()) {
      stateOptimal(0) = CurveUtils::clampDistance(stateOptimal(0), cumulativeDistanceOptimal);
      sOpt = stateOptimal(0);
      LOpt = cumulativeDistanceOptimal.back();
      updateBallPosition(sOpt, ballOptimal, optimalPath, optimalCurvePoints, cumulativeDistanceOptimal);
    }

    if (mainBallFinished == false)
      spdlog::debug("{} x(k + 1) = [{}, {}]^T , sine: {}", logTag, state(0), state(1), sine);
    if (optimalBallFinished == false)
      spdlog::debug("{} x_opt(k + 1) = [{}, {}]^T , sine: {}", logTag, stateOptimal(0), stateOptimal(1), sineOpt);

    mainBallFinished = (s == L);
    if (!optimalCurvePoints.isEmpty())
      optimalBallFinished = (sOpt == LOpt); // considero l'avanzare sulla curva ottima solo se essa esiste

    if (mainBallFinished && mainSimulationSeconds == 0.0)
      mainSimulationSeconds = totSimulationSeconds;

    if (optimalBallFinished && optimalSimulationSeconds == 0.0)
      optimalSimulationSeconds = totSimulationSeconds;

    if (mainBallFinished && optimalBallFinished) // fine della simulazione (hanno entrambe raggiunto la fine)
    {
      simulationClock->stop();
      spdlog::info("{} Simulazione terminata in {} s", logTag, totSimulationSeconds);
      emit simulationFinished();
    }
  }

  void SimulationCanvas::stepSymplecticEuler(const double inputValue, double& sineValue, double& sineOptimal) {
    arma::mat A = { { 1.0, Constants::fixedSubDT }, { 0.0, 1 } };
    arma::vec2 B = { 0.0, 0.0 };
    // vettore d'ingresso: inputValue = u(k) = u = gravity

    // AGGIORNO LO STATO DEL DISEGNO DELL'UTENTE
    if (mainBallFinished == false) {
      sineValue = CurveUtils::getSineAt(CurveUtils::clampDistance(state(0), cumulativeDistance), cumulativeDistance, points); // sin(s(k))

      // B = { 0.0, dt * sineValue }; // B = [ 0, dt * sineValue ]^T
      B = { Constants::fixedSubDT * Constants::fixedSubDT * sineValue, Constants::fixedSubDT * sineValue }; // B = [ dt^2 * sineValue, dt * sineValue ]^T

      // aggiorno lo stato del sistema, calcolo x(k + 1) = A * x(k) + B * u(k)
      state = A * state + B * inputValue;
    }

    // AGGIORNO LO STATO DEL PERCORSO OTTIMO
    if (optimalBallFinished == false) {
      sineOptimal = CurveUtils::getSineAt(CurveUtils::clampDistance(stateOptimal(0), cumulativeDistanceOptimal), cumulativeDistanceOptimal, optimalCurvePoints);
      // B = { 0.0, dt * sineOptimal };
      B = { Constants::fixedSubDT * Constants::fixedSubDT * sineOptimal, Constants::fixedSubDT * sineOptimal };
      stateOptimal = A * stateOptimal + B * inputValue;
    }
  }

  void SimulationCanvas::updateBallPosition(const double s, QGraphicsEllipseItem* ball, const QPainterPath& path, const QList<QPointF>& pts, const std::vector<double>& cumDist) {
    DEBUG_ASSERT(s >= 0.0 && s <= cumDist.back(), "L'ascissa curvilinea s è fuori dai limiti della curva", s, cumDist.back());
    DEBUG_ASSERT(ball != nullptr, "L'oggetto della sfera non è stato creato", ball);

    // CALCOLO DELLA NORMALE ALLA CURVA
    arma::vec2 n; // vettore normale
    int indexSegment = CurveUtils::getSegmentIndex(s, cumDist);
    arma::vec2 pointA = { pts[indexSegment].x(), pts[indexSegment].y() };
    arma::vec2 pointB = { pts[indexSegment + 1].x(), pts[indexSegment + 1].y() };
    arma::vec2 v = pointB - pointA; // vettore direzione

    double dx = v(0);
    double dy = v(1);

    // seleziono la normale in base al segno di dx
    if (dx >= 0.0) // segmento verso destra o perfettamente verticale
      n = { dy, -dx };
    else // segmento verso sinistra
      n = { -dy, dx };

    n = arma::normalise(n); // normalizzo il vettore

    // UPDATE GRAFICO DELLA PALLA
    // calcolo dell'offset (raggio R + 1px) per poggiare SOPRA la curva
    double offsetX = n(0) * (Constants::ballRadius + 1);
    double offsetY = n(1) * (Constants::ballRadius + 1);

    // aggiornamento della posizione visiva
    double percent = path.percentAtLength(s / metersPerPixel);
    QPointF pos = path.pointAtPercent(percent); // ottengo la posizione (x,y) della pallina sulla curva (senza offset)
    ball->setPos(pos.x() + offsetX - Constants::ballRadius, pos.y() + offsetY - Constants::ballRadius);
  }

  void SimulationCanvas::updateOptimalCurve() {
    cumulativeDistanceOptimal.clear();
    optimalCurvePoints.clear();
    optimalPath.clear();

    if (hasCurve() == false) {
      if (optimalCurveItem)
        optimalCurveItem->hide();

      ballOptimal->hide();
      return;
    }

    QPointF target = getEndPoint();
    optimalCurvePoints = CurveUtils::generateCycloidPoints(target, points.first(), getScaledSampleCount());

    if (optimalCurvePoints.isEmpty()) {
      if (optimalCurveItem)
        optimalCurveItem->hide();

      ballOptimal->hide();
      return;
    }

    CurveUtils::computeCumulativeDistance(optimalCurvePoints, cumulativeDistanceOptimal, metersPerPixel);

    optimalPath.moveTo(optimalCurvePoints.first());
    for (int i = 1; i < optimalCurvePoints.size(); ++i)
      optimalPath.lineTo(optimalCurvePoints[i]);

    if (optimalCurveItem)
      optimalCurveItem->setPath(optimalPath);
    else {
      optimalCurveItem = scene->addPath(optimalPath, bestPen);
      optimalCurveItem->setZValue(-1);
    }

    ballOptimal->setVisible(showOptimal && !isCycloid);
    optimalCurveItem->setVisible(showOptimal && !isCycloid);
  }

} // namespace BDraw
