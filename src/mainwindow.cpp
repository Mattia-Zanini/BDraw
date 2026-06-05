#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "simulationCanvas.h"

#include <libassert/assert.hpp>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

#include <QVector>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setMinimumSize(640, 480); // Imposto la grandezza minima della finestra

    // creo il widget centrale e il layout orizzontale principale
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // creo il pannello sinistro
    QWidget *leftPanel = new QWidget(centralWidget);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);

    // imposto i limiti di larghezza per il pannello sinistro
    leftPanel->setFixedWidth(180);

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

    // stile default dei labels presenti nel pannello sinistro
    QString labelStyle = "font-size: 14px; color: #ffffff";

    // labels del pannello sinistro
    QLabel *pathLabel = new QLabel("Curve predefinite:", leftPanel);
    QLabel *timeLabel = new QLabel("Tempo stimato: ---", leftPanel);
    QLabel *actualTimeLabel = new QLabel("Tempo effettivo: ---", leftPanel);

    pathLabel->setStyleSheet(labelStyle);
    timeLabel->setStyleSheet(labelStyle);
    actualTimeLabel->setStyleSheet(labelStyle);

    // imposto, in ordine, il pannello sinistro
    leftLayout->addWidget(pathLabel);
    leftLayout->addWidget(lineBtn);
    leftLayout->addWidget(circleBtn);
    leftLayout->addWidget(cycloidBtn);

    leftLayout->addSpacing(30);

    leftLayout->addWidget(clearBtn);
    leftLayout->addWidget(repeatBtn);
    leftLayout->addWidget(showTargetCb);

    leftLayout->addSpacing(15);

    leftLayout->addWidget(timeLabel);
    leftLayout->addWidget(actualTimeLabel);

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

    connect(clearBtn, &QPushButton::clicked, this,
            [=]
            {
                simulationCanvas->clearScene();
                timeLabel->setText("Tempo stimato: ---");
                actualTimeLabel->setText("Tempo effettivo: ---");
            });

    connect(simulationCanvas, &SimulationCanvas::drawingFinished, this,
            [=]
            {
                double time = simulationCanvas->computeTheoreticalTime();
                actualTimeLabel->setText("Tempo effettivo: ---");
                timeLabel->setText(QString("Tempo stimato: %1 s")
                                       .arg(time, 0, 'f', 3));

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