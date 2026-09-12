// cspell:ignore exprtk toms748 pchip
#ifndef CURVEUTILS_H
#define CURVEUTILS_H

#include "constants.h"

#include <QList>
#include <QPointF>
#include <QString>
#include <string>
#include <vector>

namespace BDraw {

  class CurveUtils {
  public:
    CurveUtils() = delete;
    CurveUtils(const CurveUtils&) = delete;
    CurveUtils& operator=(const CurveUtils&) = delete;
    CurveUtils(CurveUtils&&) = delete;
    CurveUtils& operator=(CurveUtils&&) = delete;

    static const std::string pointToString(const QPointF& p);             // converte un punto nel formato stringa "(x, y)" (DEBUG)
    static const std::string pointsToString(const QList<QPointF>& pList); // scrive come lista, su ogni riga, il punto (x, y) in stringa (DEBUG)

    static const double applyScale(const double pixels, const double metersPerPixel);                               // Converte un valore da pixel a metri basandosi sulla scala impostata
    static const double getScaledPointsDistance(const QPointF& p1, const QPointF& p2, const double metersPerPixel); // calcola la distanza euclidea fra 2 punti

    static QList<QPointF> generateLinePoints(const double targetX, const double targetY);
    static QList<QPointF> generateCirclePoints(const double radius, const int numPoints);
    static QList<QPointF> generateCycloidPoints(const QPointF& target, const QPointF& startPoint, const int numPoints); // genera i punti di una cicloide che termina sul target

    static QList<QPointF> postProcessingCurve(const QList<QPointF>& points, const double xMaxValue, const double negMargin);               // tolgo i punti che non rispettano la crescita monotona in X.
    static QList<QPointF> upsampleDrawnCurve(const QList<QPointF>& points, const int newNumPoints);                                        // interpola la curva disegnata per raggiungere un numero fisso di campioni
    static QList<QPointF> drawCurveFromFormula(const QString& formulaStr, const int numPoints, const double targetX, const double limitY); // genera i punti della curva da una formula matematica

    static void computeCumulativeDistance(const QList<QPointF>& pts, std::vector<double>& cumDist, const double metersPerPixel);                                                                    // calcola le distanze cumulative dei segmenti della curva
    static const int getSegmentIndex(const double s, const std::vector<double>& cumDist);                                                                                                           // ritorna l'indice del segmento rispetto alla distanza cumulativa
    static const double clampDistance(const double s, const std::vector<double>& cumDist);                                                                                                          // ritorna il valore della distanza in modo che rispetti il dominio [0, L]
    static const double getSineAt(const double s, const std::vector<double>& cumDist, const QList<QPointF>& pts);                                                                                   // ritorna il seno dell'inclinazione del segmento corrente in cui si trova la pallina
    static const double computeTheoreticalTime(const QList<QPointF>& pts, const double metersPerPixel, const double& gravity = Constants::gravity);                                                 // calcola il tempo teorico di una curva
    static const double computeBestTheoreticalTime(const QPointF& target, const QPointF& startPoint, const int numPoints, const double metersPerPixel, const double& gravity = Constants::gravity); // calcola il tempo teorico della cicloide passante per il target

  private:
    inline static const std::string logTag = "[CurveUtils]"; // prefisso identificativo per i messaggi di log
  };

} // namespace BDraw

#endif // CURVEUTILS_H
