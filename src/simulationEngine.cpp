#include "simulationEngine.h"
#include "constants.h"
#include "curveUtils.h"

#include <cmath>
#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

namespace BDraw {

  SimulationEngine::SimulationEngine() {
    reset();

    spdlog::debug("{} SimulationEngine inizializzato correttamente", logTag);
  }

  void SimulationEngine::reset() {
    state.zeros();
    stateOptimal.zeros();

    mainBallPos = QPointF(0.0, 0.0);
    optimalBallPos = QPointF(0.0, 0.0);

    mainBallFinished = false;
    optimalBallFinished = false;

    totSimulationSeconds = 0.0;
    mainSimulationSeconds = 0.0;
    optimalSimulationSeconds = 0.0;
    physicsAccumulator = 0.0;
  }

  void SimulationEngine::init(const QList<QPointF>& optimalCurvePoints) {
    reset();

    // se vengono disegnate curve che hanno il punto finale più in alto del punto iniziale allora la cicloide non viene generata.
    if (optimalCurvePoints.isEmpty())
      optimalBallFinished = true;

    spdlog::info("{} Simulazione avviata", logTag);
  }

  bool SimulationEngine::updatePhysics(double frameTime,
                                       const QList<QPointF>& points,
                                       const std::vector<double>& cumulativeDistance,
                                       const QPainterPath& curve,
                                       const QList<QPointF>& optimalCurvePoints,
                                       const std::vector<double>& cumulativeDistanceOptimal,
                                       const QPainterPath& optimalPath,
                                       double metersPerPixel) {
    // protezione contro lag improvvisi del sistema (visivamente)
    if (frameTime > Constants::maxTimeElapsed)
      frameTime = Constants::deltaTimeSeconds;

    physicsAccumulator += frameTime;

    double sine = 0.0;
    double sineOpt = 0.0;

    // Consuma il tempo a passi RIGIDAMENTE FISSI e COSTANTI
    while (physicsAccumulator >= Constants::fixedSubDT) {
      stepSymplecticEuler(Constants::gravity,
                          sine,
                          sineOpt,
                          points,
                          cumulativeDistance,
                          optimalCurvePoints,
                          cumulativeDistanceOptimal);

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
      mainSimulationSeconds = NAN;
      optimalSimulationSeconds = NAN;

      return true;
    }

    state(0) = CurveUtils::clampDistance(state(0), cumulativeDistance);
    double s = state(0);
    double L = cumulativeDistance.back();
    mainBallPos = updateBallPosition(s, curve, points, cumulativeDistance, metersPerPixel);

    double sOpt = 0.0;
    double LOpt = 0.0;
    if (!optimalCurvePoints.isEmpty() && !optimalBallFinished) {
      stateOptimal(0) = CurveUtils::clampDistance(stateOptimal(0), cumulativeDistanceOptimal);
      sOpt = stateOptimal(0);
      LOpt = cumulativeDistanceOptimal.back();
      optimalBallPos = updateBallPosition(sOpt,
                                          optimalPath,
                                          optimalCurvePoints,
                                          cumulativeDistanceOptimal,
                                          metersPerPixel);
    }

    if (mainBallFinished == false)
      spdlog::debug("{} x(k + 1) = [{}, {}]^T , sine: {}", logTag, state(0), state(1), sine);
    if (optimalBallFinished == false)
      spdlog::debug("{} x_opt(k + 1) = [{}, {}]^T , sine: {}", logTag, stateOptimal(0), stateOptimal(1), sineOpt);

    // Controllo di arrivo con tolleranza cinematica adattiva alla scala di zoom.
    // Moltiplicando per metersPerPixel (m/px), la tolleranza in metri scala in modo continuo con lo zoom:
    //   - A 100 px/m (metersPerPixel = 0.01 m/px): kinematicThreshold = 0.01 * 0.01 = 1e-4 m
    //   - A 10 px/m  (metersPerPixel = 0.1 m/px):  kinematicThreshold = 0.01 * 0.1  = 1e-3 m
    const double kinematicThreshold = Constants::kinematicBaseThreshold * metersPerPixel;

    mainBallFinished = std::abs(s - L) <= kinematicThreshold;
    if (!optimalCurvePoints.isEmpty()) // considero l'avanzare sulla curva ottima solo se essa esiste
      optimalBallFinished = std::abs(sOpt - LOpt) <= kinematicThreshold;

    if (mainBallFinished && mainSimulationSeconds == 0.0)
      mainSimulationSeconds = totSimulationSeconds;

    if (optimalBallFinished && optimalSimulationSeconds == 0.0)
      optimalSimulationSeconds = totSimulationSeconds;

    // fine della simulazione (hanno entrambe raggiunto la fine)
    if (mainBallFinished && optimalBallFinished) {
      spdlog::info("{} Simulazione terminata in {} s", logTag, totSimulationSeconds);
      return true;
    }

    return false;
  }

  void SimulationEngine::stepSymplecticEuler(const double inputValue,
                                             double& sineValue,
                                             double& sineOptimal,
                                             const QList<QPointF>& points,
                                             const std::vector<double>& cumulativeDistance,
                                             const QList<QPointF>& optimalCurvePoints,
                                             const std::vector<double>& cumulativeDistanceOptimal) {
    arma::mat A = { { 1.0, Constants::fixedSubDT },
                    { 0.0, 1 } };
    arma::vec2 B = { 0.0, 0.0 };
    // vettore d'ingresso: inputValue = u(k) = u = gravity

    // AGGIORNO LO STATO DEL DISEGNO DELL'UTENTE
    if (mainBallFinished == false) {
      sineValue = CurveUtils::getSineAt(CurveUtils::clampDistance(state(0), cumulativeDistance),
                                        cumulativeDistance, points); // sin(s(k))

      // EULERO SEMPLICE: B = { 0.0, dt * sineValue }; // B = [ 0, dt * sineValue ]^T

      // EULERO SEMPLITTICO:  B = [ dt^2 * sineValue, dt * sineValue ]^T
      B = { Constants::fixedSubDTSquared * sineValue, Constants::fixedSubDT * sineValue };

      // aggiorno lo stato del sistema, calcolo x(k + 1) = A * x(k) + B * u(k)
      state = A * state + B * inputValue;
    }

    // AGGIORNO LO STATO DEL PERCORSO OTTIMO
    if (optimalBallFinished == false) {
      sineOptimal = CurveUtils::getSineAt(CurveUtils::clampDistance(stateOptimal(0), cumulativeDistanceOptimal), cumulativeDistanceOptimal, optimalCurvePoints);

      // EULERO SEMPLICE: B = { 0.0, dt * sineOptimal };

      B = { Constants::fixedSubDTSquared * sineOptimal, Constants::fixedSubDT * sineOptimal };
      stateOptimal = A * stateOptimal + B * inputValue;
    }
  }

  QPointF SimulationEngine::updateBallPosition(const double s,
                                               const QPainterPath& path,
                                               const QList<QPointF>& points,
                                               const std::vector<double>& cumDist,
                                               const double metersPerPixel) {
    DEBUG_ASSERT(s >= 0.0 && s <= cumDist.back(), "L'ascissa curvilinea s è fuori dai limiti della curva", s, cumDist.back());

    // CALCOLO DELLA NORMALE ALLA CURVA
    arma::vec2 n; // vettore normale
    const int indexSegment = CurveUtils::getSegmentIndex(s, cumDist);
    arma::vec2 pointA = { points[indexSegment].x(), points[indexSegment].y() };
    arma::vec2 pointB = { points[indexSegment + 1].x(), points[indexSegment + 1].y() };
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
    const double offsetX = n(0) * (Constants::ballRadius + 1);
    const double offsetY = n(1) * (Constants::ballRadius + 1);

    // aggiornamento della posizione visiva
    const double percent = path.percentAtLength(s / metersPerPixel);

    // ottengo la posizione (x,y) della pallina sulla curva (senza offset)
    QPointF pos = path.pointAtPercent(percent);

    return QPointF(pos.x() + offsetX - Constants::ballRadius, pos.y() + offsetY - Constants::ballRadius);
  }

} // namespace BDraw
