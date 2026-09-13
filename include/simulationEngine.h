#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H

#include <armadillo>
#include <string>
#include <vector>

#include <QList>
#include <QPainterPath>
#include <QPointF>

namespace BDraw {

  class SimulationEngine {
  public:
    SimulationEngine();
    ~SimulationEngine() = default;

    void reset();                                        // resetta lo stato della fisica e i tempi
    void init(const QList<QPointF>& optimalCurvePoints); // inizializza lo stato e posiziona le sfere all'inizio (s = 0)

    bool updatePhysics(double frameTime,
                       const QList<QPointF>& points,
                       const std::vector<double>& cumulativeDistance,
                       const QPainterPath& curve,
                       const QList<QPointF>& optimalCurvePoints,
                       const std::vector<double>& cumulativeDistanceOptimal,
                       const QPainterPath& optimalPath,
                       double metersPerPixel); // esegue l'integrazione numerica a passo fisso; ritorna true se la simulazione è terminata

    const double getSimulationTime() const { return mainSimulationSeconds; }           // ritorna il tempo reale impiegato dalla pallina principale
    const double getOptimalSimulationTime() const { return optimalSimulationSeconds; } // ritorna il tempo reale impiegato dalla pallina ottima
    const QPointF& getMainBallPos() const { return mainBallPos; }                      // ritorna la posizione grafica attuale della pallina principale
    const QPointF& getOptimalBallPos() const { return optimalBallPos; }                // ritorna la posizione grafica attuale della pallina ottima
    bool isFinished() const { return mainBallFinished && optimalBallFinished; }        // ritorna true se entrambe le palline hanno terminato

  private:
    void stepSymplecticEuler(const double inputValue, double& sineValue, double& sineOptimal,
                             const QList<QPointF>& points, const std::vector<double>& cumulativeDistance,
                             const QList<QPointF>& optimalCurvePoints, const std::vector<double>& cumulativeDistanceOptimal); // esegue un singolo passo di integrazione con il metodo di Eulero simplettico

    static QPointF updateBallPosition(const double s,
                                      const QPainterPath& path,
                                      const QList<QPointF>& pts,
                                      const std::vector<double>& cumDist,
                                      const double metersPerPixel); // calcola la posizione (x, y) della sfera sulla curva dato s

    // Attributi
    const std::string logTag = "[SimulationEngine]"; // prefisso identificativo per i messaggi di log

    double totSimulationSeconds = 0.0;     // durata totale della simulazione, espressa in secondi
    double mainSimulationSeconds = 0.0;    // tempo impiegato dalla pallina principale
    double optimalSimulationSeconds = 0.0; // tempo impiegato dalla pallina ottima
    double physicsAccumulator = 0.0;       // accumulatore temporale per il passo fisso della fisica

    arma::vec2 state{};        // stato del sistema
    arma::vec2 stateOptimal{}; // stato del sistema ottimo

    bool mainBallFinished = false;    // flag che indica se la pallina principale ha raggiunto la destinazione
    bool optimalBallFinished = false; // flag che indica se la pallina ottima ha raggiunto la destinazione

    QPointF mainBallPos{};    // posizione grafica calcolata della pallina principale
    QPointF optimalBallPos{}; // posizione grafica calcolata della pallina ottima
  };

} // namespace BDraw

#endif // SIMULATIONENGINE_H
