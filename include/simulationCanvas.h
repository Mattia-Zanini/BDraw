#ifndef SIMULATIONCANVAS_H
#define SIMULATIONCANVAS_H

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QGraphicsPathItem>
#include <QPen>

class SimulationCanvas : public QGraphicsView
{
    Q_OBJECT

    // Metodi
public:
    explicit SimulationCanvas(QWidget *parent = nullptr);
    ~SimulationCanvas() override;

    QList<QPointF> getSampledPoints() const;           // ritorna la lista dei punti campionati della curva disegnata
    void redrawCurve(const QList<QPointF> &newPoints); // ridisegna la curva
    void clearScene();                                 // pulisce la scena e resetta i dati
    void drawLine();                                   // disegna una retta
    void drawCycloid();                                // disegna una cicloide
    void drawCircle();                                 // disegna un arco di circonferenza

private:
    const QString pointToString(const QPointF &) const;         // funzione per DEBUG e LOGGING
    const QString pointsToString(const QList<QPointF> &) const; // funzione per DEBUG e LOGGING

signals:
    void drawingFinished();    // segnala è terminato il disegno
    void simulationFinished(); // segnala il termine della simulazione

    // Attributi
private:
    const std::string TAG = this->metaObject()->className(); // nome della classe
    const std::string stdTAG = "[" + TAG + "]";

    QGraphicsScene *myScene;
    QPainterPath curve;
    QPen myPen;
    QGraphicsPathItem *pathItem;
    QList<QPointF> points;
    bool isDrawing;
};

#endif // SIMULATIONCANVAS_H