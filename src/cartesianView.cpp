#include "cartesianView.h"

// Inizializza il widget e configura la scena grafica e la fisica.
CartesianView::CartesianView(QWidget *parent) : QGraphicsView(parent)
{
  spdlog::debug("CartesianView inizializzato correttamente");
}

CartesianView::~CartesianView()
{
}