#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include <QMainWindow>

// indica come Qt incapsula le dichiarazioni nel suo namespace
// (può essere vuoto o namespace Qt { ... } a seconda delle impostazioni di build).
QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    // attiva il sistema meta-object (MOC) di Qt
    // (signal/slot, proprietà, introspezione runtime) per la classe.
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
