#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cartesianView.h"

#include <libassert/assert.hpp>

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QVector>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setMinimumSize(640, 480); // Imposto la grandezza minima della finestra

    QHBoxLayout h = QHBoxLayout(this);

    spdlog::debug("MainWindow inizializzato correttamente");
}

MainWindow::~MainWindow()
{
    delete ui;
    spdlog::debug("MainWindow distrutto correttamente");
}