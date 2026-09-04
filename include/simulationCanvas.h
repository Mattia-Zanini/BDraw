// cspell:ignore SIMULATIONCANVAS qreal

#ifndef SIMULATIONCANVAS_H
#define SIMULATIONCANVAS_H

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>
#include <armadillo>
#include <exprtk.hpp>

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/beta.hpp>
#include <boost/math/tools/toms748_solve.hpp>
#include <boost/math/interpolators/pchip.hpp>

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

class SimulationCanvas : public QGraphicsView {
  Q_OBJECT

  // Metodi
public:
  explicit SimulationCanvas(QWidget* parent = nullptr);
  ~SimulationCanvas() override;

  void redrawCurve(const QList<QPointF>& newPoints);                                  // ridisegna la curva
  void clearScene();                                                                  // pulisce la scena e resetta i dati
  void drawLine();                                                                    // disegna una retta
  void drawCycloid();                                                                 // disegna una cicloide
  void drawCircle();                                                                  // disegna un arco di circonferenza
  const double computeTheoreticalTime(const QList<QPointF>& customPoints = {}) const; // calcola il tempo teorico di una curva
  void startSimulation();                                                             // avvia la simulazione e l'animazione
  const double getSimulationTime() const;                                             // ritorna il tempo reale impiegato dalla pallina principale
  const double getOptimalSimulationTime() const;                                      // ritorna il tempo reale impiegato dalla pallina ottima
  void setMetersPerPixel(double val);                                                 // imposta la scala di conversione pixel/metri
  bool hasCurve() const;                                                              // controlla se è presente almeno una curva disegnata
  const double getCurveLength() const;                                                // ritorna la lunghezza totale della curva in metri
  const QPointF getEndPoint() const;                                                  // ritorna l'ultimo punto della curva corrente
  const double computeBestTheoreticalTime(const QPointF& target) const;               // calcola il tempo teorico della cicloide passante per il target
  void drawCurveFromFormula(const QString& formulaStr);

signals:
  void drawingFinished();    // segnala è terminato il disegno
  void simulationFinished(); // segnala il termine della simulazione

public slots:
  void drawRedDot(bool show);     // mostra o nasconde il punto target sulla scena
  void setShowOptimal(bool show); // imposta la visibilità della curva ottimale

private:
  // Metodi per gestire gli eventi del mouse
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void drawBackground(QPainter* painter, const QRectF& rect) override;
  void resizeEvent(QResizeEvent* event) override;

  int getScaledSampleCount(int basePoints = 3000) const;                                                        // ritorna il numero di punti da campionare di una cruva, il valore scala linearmente con la dimensione della finestra
  const std::string pointToString(const QPointF& p) const;                                                      // converte un punto nel formato stringa "(x, y)" (DEBUG)
  const std::string pointsToString(const QList<QPointF>& pList) const;                                          // scrive come lista, su ogni riga, il punto (x, y) in stringa (DEBUG)
  void postProcessingCurve();                                                                                   // tolgo i punti che non rispettano la crescita monotona in X.
  const double applyScale(const double pixels) const;                                                           // Converte un valore da pixel a metri basandosi sulla scala impostata
  const double getScaledPointsDistance(const QPointF& p1, const QPointF& p2) const;                             // calcola la distanza euclidea fra 2 punti
  const double getSineAt(const double s, const std::vector<double>& cumDist, const QList<QPointF>& pts) const;  // ritorna il seno dell'inclinazione del segmento corrente in cui si trova la pallina
  void computeCumulativeDistance(const QList<QPointF>& pts, std::vector<double>& cumDist);                      // calcola le distanze cumulative dei segmenti della curva
  QList<QPointF> generateCycloidPoints(const QPointF& target, const QPointF& startPoint = QPointF(0, 0)) const; // genera i punti di una cicloide che termina sul target
  void updatePhysics();                                                                                         // esegue l'integrazione numerica dello stato della pallina
  void stepSymplecticEuler(const double inputValue, double& sineValue, double& sineOptimal);
  void updateBallPosition(const double s, QGraphicsEllipseItem* ball, const QPainterPath& path, const QList<QPointF>& pts, const std::vector<double>& cumDist); // aggiorna la posizione grafica della pallina sulla curva
  const double clampDistance(const double s, const std::vector<double>& cumDist) const;                                                                         // ritorna il valore della distanza in modo che rispetti il dominio [0, L]
  const int getSegmentIndex(const double s, const std::vector<double>& cumDist) const;                                                                          // ritorna l'indice del segmento rispetto alla distanza cumulativa
  void updateOptimalCurve();                                                                                                                                    // calcola e disegna la curva ottima se abilitata
  QList<QPointF> upsampleDrawnCurve(const int newNumPoints);

  // Attributi
  const std::string classTag = this->metaObject()->className(); // nome della classe
  const std::string logTag = "[" + classTag + "]";
  const double gravity = 9.81;
  const double threshold = 1e-6;      // soglia minima per le operazioni
  const double minMoveDistance = 1.0; // distanza minima fra un campione e l'altro (del disegno libero)
  const int margin = 30;              // margine dai bordi della scena
  const int ballRadius = 6;
  const int deltaTimeMilliseconds = 16;             // millisecondi tra un frame e il successivo, 16 ms ~= 60 FPS
  const double deltaTimeSeconds = 0.016;            // espresso in secondi
  const double maxTimeElapsed = 0.05;               // soglia di sicurezza per evitare che la simulazione scatti (circa 3 frame persi)
  const double fixedSubDT = 0.00416666666666666666; // 4.2 ms (240 Hz)

  QGraphicsScene* scene;                         // scena grafica principale che contiene tutti gli elementi visivi
  QPainterPath curve;                            // percorso grafico (path) della curva disegnata dall'utente
  QPainterPath optimalPath;                      // percorso grafico della curva ottima (cicloide)
  QPen pen;                                      // penna usata per disegnare la curva principale
  QGraphicsPathItem* curveItem;                  // puntatore all'elemento grafico della curva principale nella scena
  QGraphicsEllipseItem* ballItem;                // puntatore all'elemento grafico della pallina principale
  QGraphicsEllipseItem* ballOptimal;             // puntatore all'elemento grafico della pallina ottima
  QList<QPointF> points;                         // lista dei punti che compongono la curva disegnata
  bool isUserDrawing;                            // flag che indica se l'utente sta attualmente disegnando a mano libera
  double metersPerPixel;                         // fattore di conversione da pixel a metri
  arma::vec2 state;                              // stato del sistema
  arma::vec2 stateOptimal;                       // stato del sistema ottimo
  QTimer* simulationClock;                       // è il timer che scatta ogni tot millisecondi per far progredire la simulazione
  QElapsedTimer elapsedTime;                     // misura il tempo reale trascorso tra due frame successivi
  double totSimulationSeconds;                   // durata totale della simulazione, espressa in secondi
  double mainSimulationSeconds;                  // tempo impiegato dalla pallina principale
  double optimalSimulationSeconds;               // tempo impiegato dalla pallina ottima
  std::vector<double> cumulativeDistance;        // contiene le distanze cumulative della curva
  std::vector<double> cumulativeDistanceOptimal; // contiene le distanze cumulative della curva ottima
  QList<QPointF> optimalCurvePoints;             // lista dei punti che compongono la curva ottima
  QGraphicsPathItem* optimalCurveItem;           // puntatore all'elemento grafico della curva ottima nella scena
  QPen bestPen;                                  // penna usata per disegnare la curva ottima
  bool showOptimal;                              // flag per mostrare o nascondere la curva ottima e la sua pallina
  bool isCycloid;                                // flag che indica se la curva attualmente disegnata è una cicloide generata
  int initWidth;                                 // larghezza iniziale del canvas, usata per il ridimensionamento dei punti
  bool showTarget;                               // flag per mostrare o nascondere il punto di arrivo (pallino rosso)
  bool mainBallFinished;                         // flag che indica se la pallina principale ha raggiunto la destinazione
  bool optimalBallFinished;                      // flag che indica se la pallina ottima ha raggiunto la destinazione
  double physicsAccumulator;
};

#endif // SIMULATIONCANVAS_H