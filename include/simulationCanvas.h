#ifndef SIMULATIONCANVAS_H
#define SIMULATIONCANVAS_H

#include "simulationEngine.h"

#include <string>
#include <vector>

#include <QElapsedTimer>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QTimer>

namespace BDraw {

  class SimulationCanvas : public QGraphicsView {
    Q_OBJECT

    // Metodi
  public:
    explicit SimulationCanvas(QWidget* parent = nullptr);
    ~SimulationCanvas() override;

    void redrawCurve(const QList<QPointF>& newPoints);    // ridisegna la curva
    void clearScene();                                    // pulisce la scena e resetta i dati
    void drawLine();                                      // disegna una retta
    void drawCycloid();                                   // disegna una cicloide
    void drawCircle();                                    // disegna un arco di circonferenza
    const double getTheoreticalTime() const;              // calcola il tempo teorico di una curva
    const double getBestTheoreticalTime() const;          // calcola il tempo teorico della cicloide passante per il target
    void startSimulation();                               // avvia la simulazione e l'animazione
    const double getSimulationTime() const;               // ritorna il tempo reale impiegato dalla pallina principale
    const double getOptimalSimulationTime() const;        // ritorna il tempo reale impiegato dalla pallina ottima
    void setMetersPerPixel(double val);                   // imposta la scala di conversione pixel/metri
    bool hasCurve() const;                                // controlla se è presente almeno una curva disegnata
    const double getCurveLength() const;                  // ritorna la lunghezza totale della curva in metri
    const QPointF getEndPoint() const;                    // ritorna l'ultimo punto della curva corrente
    void drawCurveFromFormula(const QString& formulaStr); // disegna la curva generata da una formula matematica

  signals:
    void drawingFinished();    // segnala è terminato il disegno
    void simulationFinished(); // segnala il termine della simulazione

  public slots:
    void drawRedDot(bool show);     // mostra o nasconde il punto target sulla scena
    void setShowOptimal(bool show); // imposta la visibilità della curva ottimale

  private:
    // Metodi per gestire gli eventi del mouse e della finestra
    void mousePressEvent(QMouseEvent* event) override;                   // gestisce la pressione del mouse per iniziare il tracciamento
    void mouseMoveEvent(QMouseEvent* event) override;                    // gestisce il movimento del mouse durante il tracciamento
    void mouseReleaseEvent(QMouseEvent* event) override;                 // gestisce il rilascio del mouse al termine del tracciamento
    void drawBackground(QPainter* painter, const QRectF& rect) override; // disegna lo sfondo e l'eventuale punto di arrivo (target)
    void resizeEvent(QResizeEvent* event) override;                      // gestisce il ridimensionamento della finestra

    int getScaledSampleCount(int basePoints = 3000) const { return basePoints * ((double)viewport()->width() / initWidth); } // ritorna il numero di punti da campionare di una cruva, il valore scala linearmente con la dimensione della finestra
    void updatePhysics();                                                                                                    // esegue il passo di simulazione ad ogni tick del timer
    void updateOptimalCurve();                                                                                               // calcola e disegna la curva ottima se abilitata

    // Attributi
    const std::string classTag = this->metaObject()->className(); // nome della classe
    const std::string logTag = "[" + classTag + "]";              // prefisso identificativo per i messaggi di log

    QGraphicsScene* scene = nullptr;                 // scena grafica principale che contiene tutti gli elementi visivi
    QPainterPath curve{};                            // percorso grafico (path) della curva disegnata dall'utente
    QPainterPath optimalPath{};                      // percorso grafico della curva ottima (cicloide)
    QPen pen{};                                      // penna usata per disegnare la curva principale
    QGraphicsPathItem* curveItem = nullptr;          // puntatore all'elemento grafico della curva principale nella scena
    QGraphicsEllipseItem* ballItem = nullptr;        // puntatore all'elemento grafico della pallina principale
    QGraphicsEllipseItem* ballOptimal = nullptr;     // puntatore all'elemento grafico della pallina ottima
    QList<QPointF> points{};                         // lista dei punti che compongono la curva disegnata
    bool isUserDrawing = false;                      // flag che indica se l'utente sta attualmente disegnando a mano libera
    QTimer* simulationClock = nullptr;               // è il timer che scatta ogni tot millisecondi per far progredire la simulazione
    QElapsedTimer elapsedTime{};                     // misura il tempo reale trascorso tra due frame successivi
    std::vector<double> cumulativeDistance{};        // contiene le distanze cumulative della curva
    std::vector<double> cumulativeDistanceOptimal{}; // contiene le distanze cumulative della curva ottima
    QList<QPointF> optimalCurvePoints{};             // lista dei punti che compongono la curva ottima
    QGraphicsPathItem* optimalCurveItem = nullptr;   // puntatore all'elemento grafico della curva ottima nella scena
    QPen bestPen{};                                  // penna usata per disegnare la curva ottima
    double metersPerPixel = 0.01;                    // fattore di conversione da pixel a metri
    bool showOptimal = false;                        // flag per mostrare o nascondere la curva ottima e la sua pallina
    bool isCycloid = false;                          // flag che indica se la curva attualmente disegnata è una cicloide generata
    int initWidth = 0;                               // valore di default che però verrà successivamente modificato appena il widget finisce di essere disegnato
    bool showTarget = false;                         // flag per mostrare o nascondere il punto di arrivo (pallino rosso)

    SimulationEngine engine{}; // motore per i calcoli della simulazione fisica
  };

} // namespace BDraw

#endif // SIMULATIONCANVAS_H