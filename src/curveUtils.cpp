// cspell:ignore exprtk toms748 pchip
#include "curveUtils.h"

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include <boost/math/constants/constants.hpp>
#include <boost/math/interpolators/pchip.hpp>
#include <boost/math/tools/toms748_solve.hpp>
#include <exprtk.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace BDraw {

  const std::string CurveUtils::pointToString(const QPointF& p) {
    return QString("(x = %1, y = %2)").arg(p.x()).arg(p.y()).toStdString();
  }

  const std::string CurveUtils::pointsToString(const QList<QPointF>& pList) {
    QString result = QString("");

    for (int i = 0; i < pList.count(); i++) {
      result += QString("[%1]: ").arg(i).toStdString() + pointToString(pList[i]);
      result += "\n";
    }

    return result.toStdString();
  }

  const double CurveUtils::applyScale(const double pixels, const double metersPerPixel) {
    // più metersPerPixel è grande, più metri corrispondono a quel pixel
    return pixels * metersPerPixel;
  }

  const double CurveUtils::getScaledPointsDistance(const QPointF& p1, const QPointF& p2, const double metersPerPixel) {
    double x1 = applyScale(p1.x(), metersPerPixel);
    double y1 = applyScale(p1.y(), metersPerPixel);
    double x2 = applyScale(p2.x(), metersPerPixel);
    double y2 = applyScale(p2.y(), metersPerPixel);

    return std::hypot(x2 - x1, y2 - y1);
  }

  QList<QPointF> CurveUtils::generateLinePoints(const double targetX, const double targetY) {
    QList<QPointF> linePoints{};

    const double targetMin = std::min(targetX, targetY);

    linePoints.append(QPointF(0, 0));
    linePoints.append(QPointF(targetMin, targetMin)); // così la retta termina esattamente dove finisce l'arco di circonferenza

    return linePoints;
  }

  QList<QPointF> CurveUtils::generateCirclePoints(const double radius, const int numPoints) {
    QList<QPointF> circlePoints{};

    circlePoints.reserve(numPoints);
    circlePoints.append(QPointF(0.0, 0.0)); // punto iniziale

    for (double i = 1.0; i <= numPoints; i += 1.0) {
      double t_i = (i * boost::math::constants::half_pi<double>() / numPoints) + boost::math::constants::pi<double>();
      double x = radius * std::cos(t_i) + radius;
      double y = -radius * std::sin(t_i); // il (-) è una correzione, dovuto a come viene rappresentato l'asse y nel graphics scene
      circlePoints.append(QPointF(x, y));
    }

    circlePoints.last() = QPointF(radius, radius);
    return circlePoints;
  }

  QList<QPointF> CurveUtils::generateCycloidPoints(const QPointF& target, const QPointF& startPoint, const int numPoints) {
    QPointF relativeTarget = target - startPoint;
    QList<QPointF> cycloidPoints;

    // CONTROLLO DI FATTIBILITA' FISICA:
    // se il target è a sinistra (relativeTarget.x < 0) o più in alto rispetto al punto di partenza (relativeTarget.y < 0,
    // ricordando che l'asse Y cresce verso il basso), una pallina che parte da ferma non può raggiungere il target per sola gravità.
    // In questi casi non è possibile calcolare una curva brachistocrona valida.
    if (relativeTarget.x() < 0.0 || relativeTarget.y() < 0.0) {
      spdlog::warn("{} Impossibile generare la cicloide: il target è più in alto del punto di partenza o a sinistra (x_diff = {}, y_diff = {})", logTag, relativeTarget.x(), relativeTarget.y());
      return cycloidPoints;
    }

    double A = relativeTarget.x();
    double B = relativeTarget.y();

    double tau = 0.0;
    double c = 0.0;

    // (STEP 1) GESTIONE DEL CASO LIMITE
    // Se B è circa 0, la formula della bisezione esplode (A/B), in questo caso: tau = 2*PI e c = A / (2*PI)
    if (B < Constants::threshold) {
      tau = boost::math::constants::two_pi<double>();
      c = A / boost::math::constants::two_pi<double>();
    } else {
      // (STEP 2) CASO GENERALE: risoluzione numerica con TOMS748
      double q = A / B;

      // funzione f da azzerare:
      // phi(t) = ( t - sin(t) ) / ( 1 - cos(t) )
      // f(t) = phi(t) - q
      auto f = [q](double t) -> double {
        // dovrò risolvere l'equazione f(t) = 0 -> phi(t) - q = 0
        return (t - std::sin(t)) / (1.0 - std::cos(t)) - q;
      };

      // range di ricerca: sto strettamente dentro (0, 2*PI) per evitare le divisioni per 0 agli estremi
      double left = Constants::threshold;
      double right = boost::math::constants::two_pi<double>() - Constants::threshold;

      // imposto la precisione numerica
      boost::math::tools::eps_tolerance<double> tolerance(std::numeric_limits<double>::digits);
      std::uintmax_t maxIteration = 50;

      try {
        // eseguo TOMS748
        std::pair<double, double> result = boost::math::tools::toms748_solve(f, left, right, tolerance, maxIteration);

        // estraggo tau come punto medio del minuscolo intervallo risultante
        tau = (result.first + result.second) / 2.0;

        // ottenuto tau, ricavo c dalla seconda equazione parametrica
        c = B / (1.0 - std::cos(tau));
      } catch (const std::exception& e) {
        spdlog::error("{} Errore durante il calcolo di TOMS748: {}", logTag, e.what());
        return cycloidPoints;
      }
    }

    // (STEP 3) GENERAZIONE DEI PUNTI DELLA CURVA
    cycloidPoints.reserve(numPoints);
    cycloidPoints.append(startPoint);

    for (double i = 1.0; i <= numPoints; i += 1) {
      double t_i = (i / numPoints) * tau;
      double x = c * (t_i - std::sin(t_i)) + startPoint.x();
      double y = c * (1.0 - std::cos(t_i)) + startPoint.y();
      cycloidPoints.append(QPointF(x, y));
    }

    // correzione del punto finale
    cycloidPoints.last() = target;

    spdlog::info("{} Generata CICLOIDE con tau = {} , c = {} , inizio = {} , fine = {}", logTag, tau, c, pointToString(cycloidPoints.first()), pointToString(cycloidPoints.last()));

    return cycloidPoints;
  }

  QList<QPointF> CurveUtils::postProcessingCurve(const QList<QPointF>& points, const double xMaxValue, const double negMargin) {
    if (points.size() < 2) // non serve processare 1 punto solo (o 0)
      return points;

    spdlog::debug("{} prima del processing sono presenti {} punti", logTag, points.size());
    QList<QPointF> processedPoints;
    processedPoints.append(points.first());

    qreal min = 0;
    for (int i = 1; i < points.size(); i++) {
      if (points[i].x() > min && points[i].y() >= negMargin && points[i].x() <= xMaxValue) {
        processedPoints.append(points[i]);
        min = points[i].x();
      }
    }

    if (processedPoints.size() < 2) {
      spdlog::warn("{} La curva processata non ha punti validi (es. fuori dominio), cancello la scena", logTag);
      return {};
    }

    spdlog::debug("{} Curva processata, ora sono presenti {} punti", logTag, processedPoints.size());
    return processedPoints;
  }

  QList<QPointF> CurveUtils::upsampleDrawnCurve(const QList<QPointF>& points, const int newNumPoints) {
    if (points.count() == newNumPoints) {
      spdlog::debug("{} punti non interpolati in quanto sono già abbastanza numerosi: {} punti", logTag, points.count());
      return points;
    }
    if (points.count() < 4) {
      spdlog::debug("{} non sono presenti abbastanza punti per interpolare, ne sono necessari almeno 4", logTag, points.count());
      return points;
    }

    std::vector<double> xValues, yValues;
    xValues.reserve(points.count());
    yValues.reserve(points.count());

    for (const auto& p : points) {
      xValues.push_back(p.x());
      yValues.push_back(p.y());
    }

    // creo l'oggetto per l'interpolazione
    boost::math::interpolators::pchip<std::vector<double>> spline(std::move(xValues), std::move(yValues));

    double step = (points.last().x() - points.first().x()) / newNumPoints;
    QList<QPointF> upsampledPoints;
    upsampledPoints.reserve(newNumPoints);

    double currentX = points.first().x();
    for (int i = 0; i < newNumPoints; i++) {
      upsampledPoints.push_back(QPointF(currentX, spline(currentX)));
      currentX += step;
    }
    spdlog::debug("{} la curva disegnata a mano è stata interpolata con nuovi punti: {}", logTag, newNumPoints);

    return upsampledPoints;
  }

  QList<QPointF> CurveUtils::drawCurveFromFormula(const QString& formulaStr, const int numPoints, const double targetX, const double limitY) {
    QList<QPointF> points;

    // ottengo l'espressione matematica
    std::string expression_string = formulaStr.toStdString();
    spdlog::info("{} Formula da disegnare: {}", logTag, expression_string);
    double x = 0.0;

    // configura la tabella dei simboli (associa la variabile "x" del testo alla variabile C++)
    exprtk::symbol_table<double> symbol_table;
    symbol_table.add_variable("x", x);
    symbol_table.add_constants();

    // registra la tabella nell'espressione
    exprtk::expression<double> expression;
    expression.register_symbol_table(symbol_table);

    // eseguo il parsing della stringa
    exprtk::parser<double> parser;
    if (!parser.compile(expression_string, expression)) {
      spdlog::error("{} Errore nel parsing della formula: {}", logTag, parser.error());
      return points;
    }

    double dx = targetX / numPoints;

    points.reserve(numPoints + 1);
    for (int i = 0; i <= numPoints; ++i) {
      x = i * dx;                    // aggiorno la variabile legata al parser
      double y = expression.value(); // valuto la formula per la x corrente

      // ignoro eventuali valori non numerici (NaN) o infiniti
      if (std::isnan(y) || std::isinf(y))
        continue;

      // se la curva "sfora" oltre il limite di sicurezza, blocco la generazione
      if (y > limitY || y < -limitY)
        break;

      points.append(QPointF(x, y));
    }

    return points;
  }

  void CurveUtils::computeCumulativeDistance(const QList<QPointF>& pts,
                                             std::vector<double>& cumDist,
                                             const double metersPerPixel) {
    if (pts.count() < 2)
      return;

    DEBUG_ASSERT(cumDist.empty() == true, "il vettore delle distanze cumulative deve essere pulito prima di calcolarne di nuove");
    cumDist.reserve(pts.count() - 1); // da n punti ottengo n-1 segmenti
    double s = 0.0;

    for (int i = 1; i < pts.count(); i++) {
      s += getScaledPointsDistance(pts[i - 1], pts[i], metersPerPixel);
      cumDist.push_back(s);
    }
  }

  const int CurveUtils::getSegmentIndex(const double s, const std::vector<double>& cumDist) {
    DEBUG_ASSERT(cumDist.size() > 0, "Deve essere presente almeno un segmento"); // 1 segmento => 2 punti
    auto it = std::upper_bound(cumDist.begin(), cumDist.end(), s);               // elemento per cui il valore è < s (minore STRETTO)
    int i = std::distance(cumDist.begin(), it);                                  // posizione del segmento nel vettore

    if (i >= cumDist.size())
      i = cumDist.size() - 1;

    DEBUG_ASSERT(i >= 0 && i < cumDist.size(), "L'index del segmento ottenuto è fuori dal range dei valori validi", i, cumDist.size(), cumDist, s);
    return i;
  }

  const double CurveUtils::clampDistance(const double s, const std::vector<double>& cumDist) {
    DEBUG_ASSERT(cumDist.size() != 0, "Deve esistere almeno un segmento");
    return std::clamp(s, 0.0, cumDist.back());
  }

  const double CurveUtils::getSineAt(const double s, const std::vector<double>& cumDist, const QList<QPointF>& pts) {
    DEBUG_ASSERT(cumDist.size() > 0, "Deve essere presente almeno un segmento");

    int index = getSegmentIndex(s, cumDist);
    double dx = pts[index + 1].x() - pts[index].x();
    double dy = pts[index + 1].y() - pts[index].y();
    double ds = std::hypot(dx, dy);

    DEBUG_ASSERT(ds > 0, "Segmento di lunghezza nulla");
    return dy / ds;
  }

  const double CurveUtils::computeTheoreticalTime(const QList<QPointF>& pts,
                                                  const double metersPerPixel,
                                                  const double& gravity) {
    double totalTime = 0.0;

    if (pts.count() < 2)
      return 0.0;

    double yStart = applyScale(pts[0].y(), metersPerPixel);
    double v1 = 0.0; // la velocità iniziale al primo punto è sempre 0.0

    for (int i = 0; i < pts.count() - 1; i++) {
      double y2 = applyScale(pts[i + 1].y(), metersPerPixel);

      // pitagora
      double d = getScaledPointsDistance(pts[i], pts[i + 1], metersPerPixel);

      // VELOCITA': sqrt(2 * g * Δy), dalla conservazione dell'energia
      // il max, serve per proteggere i calcoli in caso di valori negativi
      double v2 = std::sqrt(std::max(0.0, 2.0 * gravity * (y2 - yStart)));

      // v1: velocità in ingresso (ereditata dal ciclo precedente)
      // v2: velocità in uscita (calcolata ora)

      // tempo = distanza / velocità media => d / ((v1 + v2) / 2) => 2d / (v1 + v2)
      totalTime += (2.0 * d) / (v1 + v2);

      // La velocità in uscita di questo segmento sarà la velocità in ingresso del prossimo
      v1 = v2;
    }

    spdlog::debug("{} Calcolato il tempo teorico (scala 1:{}): {} s", logTag, metersPerPixel, totalTime);

    return totalTime;
  }

  const double CurveUtils::computeBestTheoreticalTime(const QPointF& target,
                                                      const QPointF& startPoint,
                                                      const int numPoints,
                                                      const double metersPerPixel,
                                                      const double& gravity) {
    // genero la cicloide ottimale a partire dal reale punto iniziale del tracciato disegnato
    QList<QPointF> bestCurve = generateCycloidPoints(target, startPoint, numPoints);

    // se la cicloide non può essere generata (es. target più in alto dell'inizio), il tempo teorico ottimale è infinito
    if (bestCurve.isEmpty())
      return std::numeric_limits<double>::infinity();

    return computeTheoreticalTime(bestCurve, metersPerPixel, gravity);
  }

} // namespace BDraw
