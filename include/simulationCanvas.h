// cspell:ignore SIMULATIONCANVAS qreal

#ifndef SIMULATIONCANVAS_H
#define SIMULATIONCANVAS_H

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

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

    const QString pointToString(const QPointF &) const;         // converte un punto nel formato stringa "(x, y)" (DEBUG)
    const QString pointsToString(const QList<QPointF> &) const; // scrive come lista, su ogni riga, il punto (x, y) in stringa (DEBUG)
    void postProcessingCurve();                                 // tolgo i punti che non rispettano la crescita monotona in X.
    const double applyScale(const double pixels) const;         // Converte un valore da pixel a metri basandosi sulla scala impostata

    // Attributi
    const std::string TAG = this->metaObject()->className(); // nome della classe
    const std::string stdTAG = "[" + TAG + "]";
    const double gravity = 9.81;
    const double threshold = 1e-6;      // soglia minima per le operazioni
    const double minMoveDistance = 5.0; // distanza minima fra un campione e l'altro (del disegno libero)
    const int viewportMargin = 40;      // margine dai bordi della scena

    double pixelsPerMeter = 100.0;
    bool showTargetPoint = false;

    QGraphicsScene *myScene;
    QPainterPath curve;
    QPen myPen;
    QGraphicsPathItem *pathItem;
    QList<QPointF> points;
    bool isDrawing;
};

#endif // SIMULATIONCANVAS_H