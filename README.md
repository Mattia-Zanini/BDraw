# BDraw - Progetto di Tesi Triennale

Questo repository contiene il codice sorgente di **BDraw**, un software interattivo sviluppato come progetto di **tesi triennale** presso l'**Università degli Studi di Padova**, per il corso di laurea in **Ingegneria Informatica**.

Il progetto esplora il **problema della brachistocrona**: la ricerca della curva che minimizza il tempo di percorrenza tra due punti sotto l'effetto della sola gravità.

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
  - [Boost](https://www.boost.org/) (in particolare il modulo `boost-math` per funzioni speciali e costanti matematiche).

Al momento, il software è stato testato e validato solo su **macOS**.

## Compilazione e Sviluppo

Per semplificare l'installazione delle librerie esterne su qualsiasi sistema operativo (Windows, macOS, Linux), si consiglia l'uso del package manager **vcpkg**.

### Gestione Dipendenze con VCPKG

Il progetto è configurato per installare automaticamente tutte le dipendenze necessarie (`spdlog`, `armadillo`, `boost` ed `openblas`) all'avvio della configurazione di CMake.

1. **Installazione di VCPKG**:
   Clona il repository ufficiale ed esegui lo script di bootstrap (per maggiori dettagli vedi [VCPKG_GUIDE.md](VCPKG_GUIDE.md)):

   - **macOS / Linux**:
     ```bash
     git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
     ~/vcpkg/bootstrap-vcpkg.sh
     # Aggiungi VCPKG_ROOT alle variabili d'ambiente (es. nel tuo ~/.zshrc o ~/.bashrc)
     export VCPKG_ROOT=$HOME/vcpkg
     ```
   - **Windows (PowerShell)**:
     ```powershell
     git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
     C:\vcpkg\bootstrap-vcpkg.bat
     # Configura la variabile d'ambiente dell'utente
     [System.Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'C:\vcpkg', 'User')
     ```

2. **Integrazione con l'IDE**:
   - **Visual Studio Code**: Installa l'estensione **CMake Tools** e aggiungi la seguente proprietà al file `.vscode/settings.json` del workspace:
     ```json
     {
         "cmake.configureSettings": {
             "CMAKE_TOOLCHAIN_FILE": "${env:VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
         }
     }
     ```
   - **Qt Creator**: Apri il file `CMakeLists.txt`. Nelle impostazioni del progetto alla voce *CMake configuration*, aggiungi una variabile di tipo percorso chiamata `CMAKE_TOOLCHAIN_FILE` puntandola a `<percorso_a_vcpkg>/scripts/buildsystems/vcpkg.cmake`.

### Build da riga di comando

È possibile configurare e compilare il progetto da terminale specificando il toolchain file di vcpkg:

```bash
# 1. Configura il progetto generando la cartella di build
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release

# 2. Compila il progetto
cmake --build build -j 8
```

### Esecuzione

Una volta completata la compilazione, puoi avviare l'applicazione (solo su macOS):

```bash
./BDraw.app/Contents/MacOS/BDraw
```

## Documentazione e Approfondimenti

Per una trattazione dettagliata dei requisiti funzionali, dell'analisi matematica del problema, della storia della brachistocrona e delle scelte architettoniche del software, si rimanda alla documentazione completa disponibile nel file:

**[Progetto BDraw.pdf](LaTeX%20Tesi/Progetto_BDraw.pdf)**

---
*Autore: Mattia Zanini*  
*Università degli Studi di Padova - Ingegneria Informatica*
