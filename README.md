# BDraw - Progetto di Tesi Triennale

Questo repository contiene il codice sorgente di **BDraw**, un software interattivo sviluppato come progetto di **tesi triennale** presso l'**Università degli Studi di Padova**, per il corso di laurea in **Ingegneria Informatica**.

Il progetto esplora il celebre **problema della brachistocrona**: la ricerca della curva che minimizza il tempo di percorrenza tra due punti sotto l'effetto della sola gravità.

## Descrizione del Progetto

BDraw permette di confrontare l'intuizione umana con la soluzione matematica (l'arco di cicloide). Attraverso un'interfaccia grafica, l'utente può:
- Disegnare curve a mano libera o tramite equazioni.
- Avviare una simulazione fisica in tempo reale di una pallina che percorre il tracciato.
- Visualizzare il calcolo teorico esatto del tempo di percorrenza sulla spezzata disegnata.
- Confrontare le prestazioni della propria curva con la soluzione ottimale calcolata dal sistema.

## Requisiti di Sistema

Il progetto è sviluppato in **C++** e richiede i seguenti componenti per la compilazione:

- **Linguaggio**: C++17 o superiore.
- **Build System**: CMake 3.16+.
- **Framework UI**: [Qt 6.10+](https://www.qt.io/) (consigliata l'ultima versione stabile).
- **Librerie esterne**:
  - [Armadillo](https://arma.sourceforge.net/) (per il calcolo numerico).
  - [spdlog](https://github.com/gabime/spdlog) (per il logging di sistema).

Al momento, il software è stato testato e validato esclusivamente su **macOS**.

## Compilazione e Sviluppo

Per una corretta configurazione del progetto e delle sue dipendenze, si consiglia di utilizzare uno dei seguenti ambienti:

1. **Qt Creator**: È l'approccio più diretto. È sufficiente aprire il file `CMakeLists.txt` e configurare il progetto utilizzando un Kit basato su Qt 6.*
2. **Visual Studio Code**: Utilizzando l'estensione **CMake Tools**. Assicurarsi che il percorso di installazione di Qt sia correttamente configurato nelle impostazioni di CMake o nelle variabili d'ambiente.

## Documentazione e Approfondimenti

Per una trattazione dettagliata dei requisiti funzionali, dell'analisi matematica del problema, della storia della brachistocrona e delle scelte architettoniche del software, si rimanda alla documentazione completa (report di progetto) disponibile nel file:

📄 **[Progetto BDraw.md](Progetto%20BDraw.md)**

---
*Autore: Mattia*  
*Università degli Studi di Padova - Ingegneria Informatica*
