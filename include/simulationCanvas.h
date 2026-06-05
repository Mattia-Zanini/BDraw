// cspell:ignore SIMULATIONCANVAS qreal

#ifndef SIMULATIONCANVAS_H
#define SIMULATIONCANVAS_H

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include <armadillo>

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/beta.hpp>
#include <boost/math/tools/toms748_solve.hpp>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QGraphicsPathItem>
#include <QMouseEvent>
#include <QPen>
#include <QPainter>
#include <QBrush>
#include <QElapsedTimer>
#include <QTimer>
#include <vector>

class SimulationCanvas : public QGraphicsView
{
    Q_OBJECT

    // Metodi
public:
    explicit SimulationCanvas(QWidget *parent = nullptr);
    ~SimulationCanvas() override;

    void redrawCurve(const QList<QPointF> &newPoints); // ridisegna la curva
    void clearScene();                                 // pulisce la scena e resetta i dati
    void drawLine();                                   // disegna una retta
    void drawCycloid();                                // disegna una cicloide
    void drawCircle();                                 // disegna un arco di circonferenza
    const double computeTheoreticalTime() const;       // calcola il tempo teorico di una curva
    void startSimulation();                            // avvia la simulazione e l'animazione
    const double getSimulationTime() const;

signals:
    void drawingFinished();    // segnala è terminato il disegno
    void simulationFinished(); // segnala il termine della simulazione

public slots:
    void drawRedDot(bool show);

private:
    // Metodi per gestire gli eventi del mouse
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    const QString pointToString(const QPointF &) const;                           // converte un punto nel formato stringa "(x, y)" (DEBUG)
    const QString pointsToString(const QList<QPointF> &) const;                   // scrive come lista, su ogni riga, il punto (x, y) in stringa (DEBUG)
    void postProcessingCurve();                                                   // tolgo i punti che non rispettano la crescita monotona in X.
    const double applyScale(const double pixels) const;                           // Converte un valore da pixel a metri basandosi sulla scala impostata
    const double getScaledPointsDistance(const QPointF &, const QPointF &) const; // calcola la distanza euclidea fra 2 punti
    const double getSlopeSineAt(const double) const;                              // ritorna il seno dell'inclinazione del segmento corrente in cui si trova la pallina
    void computeCumulativeDistance();
    void updatePhysics();
    void updateBallPosition(const double s);
    const double clampDistance(const double) const;      // ritorna il valore della distanza in modo che rispetti il dominio [0, L]
    const int getSegmentIndex(const double) const; // ritorna l'indice del segmento rispetto alla distanza cumulativa

    // Attributi
    const std::string classTag = this->metaObject()->className(); // nome della classe
    const std::string logTag = "[" + classTag + "]";
    const double gravity = 9.81;
    const double threshold = 1e-6;      // soglia minima per le operazioni
    const double minMoveDistance = 5.0; // distanza minima fra un campione e l'altro (del disegno libero)
    const int margin = 40;              // margine dai bordi della scena
    const int ballRadius = 6;
    const int deltaTimeMilliseconds = 16;  // millisecondi tra un frame e il successivo, 16 ms ~= 60 FPS
    const double deltaTimeSeconds = 0.016; // espresso in secondi

    double pixelsPerMeter = 100.0;
    bool showTarget = false;

    QGraphicsScene *scene;
    QPainterPath curve;
    QPen pen;
    QGraphicsPathItem *curveItem;
    QGraphicsEllipseItem *ballItem;
    QList<QPointF> points;
    bool isUserDrawing;
    arma::vec2 state;                       // stato del sistema
    QTimer *simulationClock;                // è il timer che scatta ogni tot millisecondi per far progredire la simulazione
    QElapsedTimer elapsedTime;              // misura il tempo reale trascorso tra due frame successivi
    QElapsedTimer totalSimulationTime;      // misura la durata totale dell'intera simulazione
    double totSimulationSeconds;            // durata totale della simulazione, espressa in secondi
    std::vector<double> cumulativeDistance; // contiene le distanze cumulative della curva
};

#endif // SIMULATIONCANVAS_H