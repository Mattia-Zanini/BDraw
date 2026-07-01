#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simulationCanvas.h"

#include <libassert/assert.hpp>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QLineEdit>

#include <QVector>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setMinimumSize(initWindowWidth, initWindowHeigth); // Imposto la grandezza minima della finestra

    // creo il widget centrale e il layout orizzontale principale
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // creo il pannello sinistro
    QWidget *leftPanel = new QWidget(centralWidget);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);

    // imposto i limiti di larghezza per il pannello sinistro
    leftPanel->setFixedWidth(controlPanelWidth);

    // creo il pannello destro per la scena grafica
    SimulationCanvas *simulationCanvas = new SimulationCanvas(centralWidget); // rightPanel

    // pulsanti d'azione
    QPushButton *clearBtn = new QPushButton("Pulisci", leftPanel);
    QPushButton *repeatBtn = new QPushButton("Ripeti", leftPanel);

    // pulsanti per le curve di default
    QPushButton *lineBtn = new QPushButton("Retta", leftPanel);
    QPushButton *circleBtn = new QPushButton("Circonferenza", leftPanel);
    QPushButton *cycloidBtn = new QPushButton("Cicloide", leftPanel);

    // checkbox per mostrare/nascondere il punto target
    QCheckBox *showTargetCb = new QCheckBox("Mostra Target", leftPanel);
    showTargetCb->setChecked(false);
    showTargetCb->setStyleSheet("font-size: 14px; color: #ffffff");

    QCheckBox *showOptimalCb = new QCheckBox("Mostra Ottimo", leftPanel);
    showOptimalCb->setChecked(false);
    showOptimalCb->setStyleSheet("font-size: 14px; color: #ffffff");

    // stile default dei labels presenti nel pannello sinistro
    QString labelStyle = "font-size: 14px; color: #ffffff";

    // slider e label dello zoom
    QLabel *zoomLabel = new QLabel("Zoom: 100 px/m", leftPanel);
    zoomLabel->setStyleSheet(labelStyle);

    QSlider *zoomSlider = new QSlider(Qt::Horizontal, leftPanel);
    zoomSlider->setRange(10, 100);
    zoomSlider->setValue(100);

    // labels del pannello sinistro
    QLabel *pathLabel = new QLabel("Curve predefinite:", leftPanel);
    QLabel *timeLabel = new QLabel("Tempo stimato: ---", leftPanel);
    QLabel *actualTimeLabel = new QLabel("Tempo effettivo: ---", leftPanel);
    QLabel *bestTimeLabel = new QLabel("Tempo migliore: ---", leftPanel);
    QLabel *lengthLabel = new QLabel("Lunghezza: ---", leftPanel);

    QLabel *formulaLabel = new QLabel("Disegna da formula:", leftPanel);
    QLineEdit *formulaInput = new QLineEdit(leftPanel);
    formulaInput->setPlaceholderText("Es. 2x");
    formulaInput->setStyleSheet("padding: 5px; font-size: 14px;");

    pathLabel->setStyleSheet(labelStyle);
    timeLabel->setStyleSheet(labelStyle);
    actualTimeLabel->setStyleSheet(labelStyle);
    bestTimeLabel->setStyleSheet(labelStyle);
    lengthLabel->setStyleSheet(labelStyle);

    // imposto, in ordine, il pannello sinistro
    leftLayout->addWidget(pathLabel);
    leftLayout->addWidget(lineBtn);
    leftLayout->addWidget(circleBtn);
    leftLayout->addWidget(cycloidBtn);

    leftLayout->addSpacing(15);
    leftLayout->addWidget(zoomLabel);
    leftLayout->addWidget(zoomSlider);
    leftLayout->addWidget(lengthLabel);
    leftLayout->addSpacing(15);

    leftLayout->addWidget(clearBtn);
    leftLayout->addWidget(repeatBtn);
    leftLayout->addWidget(showTargetCb);
    leftLayout->addWidget(showOptimalCb);

    leftLayout->addSpacing(15);

    leftLayout->addWidget(timeLabel);
    leftLayout->addWidget(actualTimeLabel);
    leftLayout->addSpacing(10);
    leftLayout->addWidget(bestTimeLabel);

    leftLayout->addSpacing(15);
    formulaLabel->setStyleSheet(labelStyle);
    leftLayout->addWidget(formulaLabel);
    leftLayout->addWidget(formulaInput);

    connect(formulaInput, &QLineEdit::returnPressed,
            [this, simulationCanvas, formulaInput]()
            {
                if (!formulaInput->text().trimmed().isEmpty())
                    simulationCanvas->drawCurveFromFormula(formulaInput->text());
            });

    // aggiungo uno spazio vuoto elastico, questo spingerà  in modo compatto tutti i tuoi pulsanti verso la parte superiore del pannello
    leftLayout->addStretch();

    // aggiungo il pannello sinistro e il canvas a destra, nel layout orizzontale
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(simulationCanvas);

    // connetto il click dei pulsanti ai rispettivi metodi che devono eseguire
    connect(lineBtn, &QPushButton::clicked, simulationCanvas, &SimulationCanvas::drawLine);
    connect(circleBtn, &QPushButton::clicked, simulationCanvas, &SimulationCanvas::drawCircle);
    connect(cycloidBtn, &QPushButton::clicked, simulationCanvas, &SimulationCanvas::drawCycloid);
    connect(showTargetCb, &QCheckBox::toggled, simulationCanvas, &SimulationCanvas::drawRedDot);
    connect(showOptimalCb, &QCheckBox::toggled, simulationCanvas, &SimulationCanvas::setShowOptimal);

    connect(zoomSlider, &QSlider::valueChanged, this,
            [=](int value)
            {
                zoomLabel->setText(QString("Zoom: %1 px/m").arg(value));
                simulationCanvas->setMetersPerPixel(1.0 / value);

                if (simulationCanvas->hasCurve())
                {
                    double length = simulationCanvas->getCurveLength();
                    lengthLabel->setText(QString("Lunghezza: %1 m")
                                             .arg(length, 0, 'f', 2));

                    double time = simulationCanvas->computeTheoreticalTime();
                    timeLabel->setText(QString("Tempo stimato: %1 s")
                                           .arg(time, 0, 'f', 3));

                    double bestTimeVal = simulationCanvas->computeBestTheoreticalTime(simulationCanvas->getEndPoint());
                    bestTimeLabel->setText(QString("Tempo migliore: %1 s")
                                               .arg(bestTimeVal, 0, 'f', 3));
                }
            });

    connect(clearBtn, &QPushButton::clicked, this,
            [=]
            {
                simulationCanvas->clearScene();
                timeLabel->setText("Tempo stimato: ---");
                actualTimeLabel->setText("Tempo effettivo: ---");
                bestTimeLabel->setText("Tempo migliore: ---");
                lengthLabel->setText("Lunghezza: ---");
            });

    connect(simulationCanvas, &SimulationCanvas::drawingFinished, this,
            [=]
            {
                double time = simulationCanvas->computeTheoreticalTime();
                actualTimeLabel->setText("Tempo effettivo: ---");
                timeLabel->setText(QString("Tempo stimato: %1 s")
                                       .arg(time, 0, 'f', 3));

                double length = simulationCanvas->getCurveLength();
                lengthLabel->setText(QString("Lunghezza: %1 m")
                                         .arg(length, 0, 'f', 2));

                double bestTimeVal = simulationCanvas->computeBestTheoreticalTime(simulationCanvas->getEndPoint());
                bestTimeLabel->setText(QString("Tempo migliore: %1 s")
                                           .arg(bestTimeVal, 0, 'f', 3));

                simulationCanvas->startSimulation();
            });

    connect(repeatBtn, &QPushButton::clicked, this,
            [=]
            {
                actualTimeLabel->setText("Tempo effettivo: ---");
                simulationCanvas->startSimulation();
            });

    connect(simulationCanvas, &SimulationCanvas::simulationFinished, this,
            [=]
            {
                double simTime = simulationCanvas->getSimulationTime();
                actualTimeLabel->setText(QString("Tempo effettivo: %1 s")
                                             .arg(simTime, 0, 'f', 3));
            });

    // infine imposto il widget centrale sulla MainWindow
    setCentralWidget(centralWidget);
    setWindowTitle("");

    spdlog::debug("{} MainWindow inizializzato correttamente", stdTAG);
}

MainWindow::~MainWindow()
{
    delete ui;
    spdlog::debug("{} MainWindow distrutto correttamente", stdTAG);
}