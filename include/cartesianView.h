#ifndef CARTESIANVIEW_H
#define CARTESIANVIEW_H

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include <QGraphicsView>
#include <QGraphicsScene>

class CartesianView : public QGraphicsView
{
public:
    explicit CartesianView(QWidget *parent = nullptr);
    ~CartesianView() override;

private:
};

#endif // CARTESIANVIEW_H