#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <libassert/assert.hpp>
#include <spdlog/spdlog.h>

#include <QMainWindow>

// Racchiude le forward declaration nel namespace standard di Qt
QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief Gestisce la finestra principale dell'applicazione BDraw.
 *
 * Questa classe eredita da QMainWindow e funge da contenitore principale
 * per l'interfaccia utente. Si occupa di fare da ponte tra la grafica e la
 * logica del programma: gestisce il disegno della curva da parte dell'utente, 
 * riceve gli input visivi e orchestra il post-processing dei dati.
 *
 * @author Mattia Zanini
 * @date 2026
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Costruttore della classe MainWindow.
     * 
     * @param parent Puntatore al widget padre. Di default è impostato a nullptr,
     * indicando che questa è una finestra indipendente di primo livello.
     *
     * Si occupa di costruire fisicamente a schermo i componenti visivi 
     * dell'interfaccia, inizializzare i layout e collegare le azioni dell'utente 
     * (es. il click di un bottone) alle funzioni che devono gestirle.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Distruttore della classe MainWindow.
     *
     * Si occupa di liberare in modo sicuro la memoria utilizzata dall'interfaccia
     * grafica al momento della chiusura della finestra.
     */
    ~MainWindow() override;

private:
    /**
     * @brief Puntatore all'interfaccia utente visiva.
     *
     * Questo puntatore fornisce un comodo accesso da codice a tutti i singoli 
     * elementi grafici (pulsanti, menu, etichette) che sono stati posizionati 
     * e disegnati utilizzando il tool Qt Designer.
     */
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
