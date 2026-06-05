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
  - [libassert](https://github.com/jeremy-rifkin/libassert) (per la gestione avanzata delle asserzioni).

Il software è stato testato e validato su **macOS** e **Linux (Ubuntu)**.

## Compilazione e Sviluppo

Per semplificare l'installazione delle librerie esterne su qualsiasi sistema operativo (Windows, macOS, Linux), si consiglia l'uso del package manager **vcpkg**.

> **Consiglio sull'installazione di Qt**: Se non hai la necessità di utilizzare l'ambiente grafico **Qt Creator** per sviluppare, è fortemente consigliato installare Qt tramite i **package manager** (come `apt` su Linux o `Homebrew` su macOS) o strumenti leggeri come `aqtinstall`. Questo evita di dover scaricare l'intero installer ufficiale (Qt Online Installer), che richiede la creazione di un account ed occupa diversi gigabyte sul disco.

### Installazione delle dipendenze di sistema (Linux/Ubuntu)

Prima di iniziare a configurare il progetto su Ubuntu, è necessario installare sul sistema i compilatori, gli strumenti per vcpkg e le librerie di sviluppo di Qt6:

```bash
sudo apt update
sudo apt install build-essential gfortran cmake ninja-build pkg-config \
                 zip unzip tar curl wget autoconf autoconf-archive automake libtool \
                 qt6-base-dev qt6-tools-dev qt6-l10n-tools
```

### Installazione leggera di Qt6 (macOS e Windows)

Per evitare di installare l'intero ambiente di sviluppo pesante di Qt (come l'installer online ufficiale), è possibile installare una versione minimalista di Qt 6.10.0 utilizzando gestori di pacchetti o lo strumento a riga di comando `aqtinstall`.

#### 1. macOS (via Homebrew)
Homebrew consente di installare Qt6 e gli strumenti di build richiesti per compilare alcune dipendenze di `vcpkg` (come `libbacktrace` usato da Boost/spdlog):
```bash
brew install qt autoconf autoconf-archive automake libtool
```
*(Nota: il percorso in cui viene installato Qt6 tramite Homebrew è solitamente `/opt/homebrew/opt/qt`)*.


#### 2. Windows e macOS (via `aqtinstall`)
`aqtinstall` è uno strumento Python che scarica solo i binari precompilati necessari dai server ufficiali Qt, senza richiedere registrazioni o il download di strumenti aggiuntivi.

1. Installa lo strumento tramite `pip`:
   ```bash
   pip install aqtinstall
   ```
2. Installa la versione **6.10.0** desktop per la tua piattaforma:
   - **Su macOS**:
     ```bash
     aqt install-qt mac desktop 6.10.0 clang_64
     ```
   - **Su Windows (compilatore MSVC 2022)**:
     ```cmd
     aqt install-qt windows desktop 6.10.0 win64_msvc2022_64
     ```
   - **Su Windows (compilatore MinGW)**:
     ```cmd
     aqt install-qt windows desktop 6.10.0 win64_mingw
     ```

#### 3. macOS, Windows e Linux (via installatore ufficiale Qt / Qt Creator)
Se hai installato Qt tramite l'installer ufficiale (Qt Online Installer), la libreria si troverà solitamente in una sottocartella della tua cartella home (o in `C:\` su Windows):
* **Su macOS**: ad esempio in `~/Qt/6.10.3/macos/`
* **Su Windows**: ad esempio in `C:\Qt\6.10.3\msvc2022_64\` o `C:\Qt\6.10.3\mingw_64\`
* **Su Linux**: ad esempio in `~/Qt/6.10.3/gcc_64/`

In questo caso, non hai pacchetti di sistema globali e dovrai individuare manualmente questo percorso sul tuo disco per impostarlo come `CMAKE_PREFIX_PATH` durante la configurazione o nell'IDE.

---

### Gestione Dipendenze con VCPKG

Il progetto è configurato per installare automaticamente tutte le dipendenze necessarie (`spdlog`, `armadillo`, `boost` ed `openblas`) all'avvio della configurazione di CMake.

1. **Installazione di VCPKG**:
   Clona il repository ufficiale ed esegui lo script di bootstrap (per maggiori dettagli vedi [VCPKG_GUIDE.md](VCPKG_GUIDE.md)):

   - **macOS / Linux**:
     ```bash
     git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
     ~/vcpkg/bootstrap-vcpkg.sh -disableMetrics
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
   - **Visual Studio Code**: Installa l'estensione **CMake Tools** e aggiungi la configurazione al file `.vscode/settings.json` del workspace. 
     Se hai installato Qt tramite Homebrew o `aqtinstall`, devi specificare anche `CMAKE_PREFIX_PATH` in modo che CMake possa trovarlo:
     ```json
     {
         "cmake.configureSettings": {
             "CMAKE_TOOLCHAIN_FILE": "${env:VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake",
             // Rimuovi il commento e imposta il percorso corretto per la tua installazione:
             // "CMAKE_PREFIX_PATH": "/opt/homebrew/opt/qt" (Homebrew su macOS)
             // "CMAKE_PREFIX_PATH": "/Users/TUO_UTENTE/Qt/6.10.3/macos" (Installatore ufficiale su macOS)
             // "CMAKE_PREFIX_PATH": "C:/Qt/6.10.0/msvc2022_64" (aqtinstall su Windows con MSVC)
             // "CMAKE_PREFIX_PATH": "C:/Qt/6.10.0/mingw_64" (aqtinstall su Windows con MinGW)
         }
     }
     ```
   - **Qt Creator**: Apri il file `CMakeLists.txt`. Nelle impostazioni del progetto alla voce *CMake configuration*, aggiungi la variabile di tipo percorso chiamata `CMAKE_TOOLCHAIN_FILE` puntandola a `<percorso_a_vcpkg>/scripts/buildsystems/vcpkg.cmake`. Se necessario (su macOS/Windows), aggiungi anche `CMAKE_PREFIX_PATH` come tipo percorso puntandolo alla cartella di Qt6 installata.

### Build da riga di comando

È possibile configurare e compilare il progetto da terminale specificando il toolchain file di vcpkg e, se non usi l'installazione di sistema predefinita (ad es. su Ubuntu), specificando il percorso del framework tramite `CMAKE_PREFIX_PATH`.

Di default, nei comandi seguenti viene configurata la modalità **Debug** per abilitare tutti i log di tracciamento. Se si desidera compilare in modalità **Release** (ottimizzata e senza log di debug), è sufficiente sostituire `-DCMAKE_BUILD_TYPE=Debug` con `-DCMAKE_BUILD_TYPE=Release` durante la configurazione (o specificare `--config Release` durante la build su Windows/MSVC).

> [!NOTE]
> La prima configurazione del progetto impiega circa **10 ~ 15 minuti**. Questo accade perché `vcpkg` deve scaricare e compilare da sorgente tutte le dipendenze esterne (Boost, Armadillo, spdlog, ecc.). Le build e configurazioni successive saranno molto più rapide.


- **Su Linux (Ubuntu)** (usando Ninja per massimizzare le prestazioni di build):
  ```bash
  # 1. Configura il progetto in modalità Debug
  cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Debug

  # 2. Compila il progetto
  cmake --build build
  ```

- **Su macOS (con Qt installato via Homebrew)**:
  ```bash
  # 1. Configura specificando il percorso CMAKE_PREFIX_PATH di Homebrew in modalità Debug
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt" -DCMAKE_BUILD_TYPE=Debug

  # 2. Compila il progetto
  cmake --build build -j 8
  ```

- **Su macOS (con Qt installato via installer ufficiale / Qt Creator)**:
  ```bash
  # 1. Configura specificando la cartella di Qt nella home (es. versione 6.10.3)
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="$HOME/Qt/6.10.3/macos" -DCMAKE_BUILD_TYPE=Debug

  # 2. Compila il progetto
  cmake --build build -j 8
  ```

- **Su Windows e macOS (con Qt installato via `aqtinstall`)**:
  Se hai installato una versione locale di Qt tramite `aqtinstall`, devi indicare a CMake la cartella specifica contenente la versione e il compilatore prescelto (sostituisci `C:/Qt` o `~/Qt` con la tua reale cartella di installazione).

  *   **Windows (compilatore MSVC 2022)**:
      ```cmd
      :: Configura specificando il toolchain e la cartella del compilatore MSVC di Qt 6.10.0
      cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.0/msvc2022_64"
      
      :: Compila specificando la configurazione desiderata (Debug o Release)
      cmake --build build --config Debug
      ```
  *   **Windows (compilatore MinGW)**:
      ```cmd
      :: Configura specificando il toolchain e la cartella di MinGW di Qt 6.10.0 in modalità Debug
      cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.0/mingw_64" -DCMAKE_BUILD_TYPE=Debug
      
      :: Compila
      cmake --build build
      ```
  *   **macOS (compilatore Clang)**:
      ```bash
      # Configura specificando il toolchain e la cartella clang_64 di Qt 6.10.0 in modalità Debug
      cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="$HOME/Qt/6.10.0/clang_64" -DCMAKE_BUILD_TYPE=Debug
      
      # Compila
      cmake --build build -j 8
      ```

### Esecuzione

Una volta completata la compilazione, puoi avviare l'applicazione:

- **Su Linux (Ubuntu)**:
  ```bash
  ./build/BDraw
  ```

- **Su macOS**:
  ```bash
  ./build/BDraw.app/Contents/MacOS/BDraw
  ```

## Documentazione e Approfondimenti

Per una trattazione dettagliata dei requisiti funzionali, dell'analisi matematica del problema, della storia della brachistocrona e delle scelte architettoniche del software, si rimanda alla documentazione completa disponibile nel file:

**[Progetto BDraw.pdf](LaTeX%20Tesi/Progetto_BDraw.pdf)**

---
*Autore: Mattia Zanini*  
*Università degli Studi di Padova - Ingegneria Informatica*
