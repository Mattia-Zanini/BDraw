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
- **Framework UI**: [Qt 6.10+](https://www.qt.io/)
- **Librerie esterne**:
  - [Armadillo](https://arma.sourceforge.net/) (per il calcolo numerico).
  - [spdlog](https://github.com/gabime/spdlog) (per il logging di sistema).
  - [Boost](https://www.boost.org/) (in particolare il modulo `boost-math` per funzioni speciali e costanti matematiche).
  - [libassert](https://github.com/jeremy-rifkin/libassert) (per la gestione avanzata delle asserzioni).
  - [exprtk](https://github.com/ArashPartow/exprtk) (già inclusa nel progetto come libreria header-only, non è necessario scaricarla).

Il software è stato testato e validato su **macOS** e **Linux (Ubuntu)**.

## Formule Consigliate da Testare

È possibile inserire formule matematiche personalizzate per disegnare la pista della simulazione. L'asse Y cresce verso il basso (nel verso della gravità). Ecco alcune curve interessanti da testare:

- `sin(x / 30) * 50 + x / 2` (Le Montagne Russe)
- `-abs(x - 300) + 300` (La rampa da Skate)
- `sqrt(x) * 15` (Lo scivolo ad acqua)
- `x / 2 + sin(x / 15) * 15` (I Gradini morbidi)
- `exp(x / 110) - 1` (Il burrone)
- `-((x-200)/60)^4 + 300`

## Guida all'Installazione e Compilazione

Per semplificare l'installazione delle librerie esterne su qualsiasi sistema operativo (Windows, macOS, Linux), il progetto utilizza il package manager **vcpkg** (in Manifest Mode) integrato direttamente nel build system CMake.

---

### FASE 1: Installazione delle Dipendenze di Sistema e di Qt6

Prima di configurare il progetto, è necessario installare sul sistema i compilatori, gli strumenti di build e il framework Qt6.

> [!TIP]
> **Consiglio sull'installazione di Qt**: Se non hai la necessità di utilizzare l'ambiente grafico **Qt Creator** per sviluppare, è fortemente consigliato installare Qt tramite i **package manager** (come `apt` su Linux o `Homebrew` su macOS) o strumenti leggeri come `aqtinstall`. Questo evita di dover scaricare l'intero installer ufficiale (Qt Online Installer), che richiede la creazione di un account ed occupa diversi gigabyte sul disco.

#### A. macOS (via Homebrew - Raccomandato)
Homebrew consente di installare Qt6, il generatore Ninja e gli strumenti richiesti per compilare le dipendenze di `vcpkg` (inclusi `pkg-config`, `libtool`, ecc.):
```bash
brew install qt autoconf autoconf-archive automake libtool pkg-config ninja
```
*(Nota: il percorso in cui viene installato Qt6 tramite Homebrew è solitamente `/opt/homebrew/opt/qt`)*.

#### B. Linux (Ubuntu/Debian)
Installa i compilatori, i build tool di base e i pacchetti di sviluppo per Qt6:
```bash
sudo apt update
sudo apt install build-essential gfortran cmake ninja-build pkg-config \
                 zip unzip tar curl wget autoconf autoconf-archive automake libtool \
                 qt6-base-dev qt6-tools-dev qt6-l10n-tools
```

#### C. Installazione alternativa di Qt6 tramite `aqtinstall` (macOS e Windows)
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

#### D. Installazione classica tramite Installer Ufficiale Qt (macOS, Windows, Linux)
Se hai installato Qt tramite l'Online Installer ufficiale, la libreria si troverà solitamente in una sottocartella della tua cartella home:
* **Su macOS**: ad esempio in `~/Qt/6.10.3/macos/`
* **Su Windows**: ad esempio in `C:\Qt\6.10.3\msvc2022_64\` o `C:\Qt\6.10.3\mingw_64\`
* **Su Linux**: ad esempio in `~/Qt/6.10.3/gcc_64/`

Dovrai impostare questo percorso come `CMAKE_PREFIX_PATH` durante la configurazione o nell'IDE.

---

### FASE 2: Installazione e Configurazione di VCPKG

Il progetto è configurato per installare automaticamente tutte le dipendenze esterne (`spdlog`, `armadillo`, `boost` ed `openblas`) all'avvio della configurazione di CMake.

#### 1. Clonazione e Bootstrap di VCPKG
Scegli una cartella nel tuo sistema (es. la tua cartella home) e installa vcpkg:

* **macOS / Linux**:
  ```bash
  git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
  ~/vcpkg/bootstrap-vcpkg.sh -disableMetrics
  # Aggiungi VCPKG_ROOT alle variabili d'ambiente (es. nel tuo ~/.zshrc o ~/.bashrc)
  export VCPKG_ROOT=$HOME/vcpkg
  ```
* **Windows (PowerShell)**:
  ```powershell
  git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
  C:\vcpkg\bootstrap-vcpkg.bat
  # Configura la variabile d'ambiente dell'utente
  [System.Environment]::SetEnvironmentVariable('VCPKG_ROOT', 'C:\vcpkg', 'User')
  ```

Per maggiori dettagli su vcpkg, consulta anche [VCPKG_GUIDE.md](VCPKG_GUIDE.md).

#### 2. Integrazione con gli IDE

##### Visual Studio Code (CMake Tools)
Installa l'estensione **CMake Tools** e aggiungi la configurazione al file `.vscode/settings.json` del workspace. 
Specifica `CMAKE_TOOLCHAIN_FILE` e `CMAKE_PREFIX_PATH` (per indicare a CMake dove trovare Qt6 installato da Homebrew o aqtinstall):
```json
{
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "/Users/TUO_UTENTE/vcpkg/scripts/buildsystems/vcpkg.cmake", // Sostituisci col tuo percorso reale o usa "${env:VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        "CMAKE_PREFIX_PATH": "/opt/homebrew/opt/qt" // Modifica in base al percorso della tua installazione di Qt (es. per Homebrew su Mac)
    }
}
```

##### Qt Creator
Apri il file `CMakeLists.txt`. Nelle impostazioni del progetto alla voce *CMake configuration*, aggiungi la variabile di tipo percorso chiamata `CMAKE_TOOLCHAIN_FILE` puntandola a `<percorso_a_vcpkg>/scripts/buildsystems/vcpkg.cmake`. Aggiungi anche `CMAKE_PREFIX_PATH` di tipo percorso puntandolo alla cartella di Qt6 installata.

---

### FASE 3: Configurazione e Compilazione

> [!NOTE]
> La prima configurazione del progetto impiega circa **10 ~ 15 minuti**. Questo accade perché `vcpkg` deve scaricare e compilare da sorgente tutte le dipendenze esterne (Boost, Armadillo, spdlog, ecc.). Le build e configurazioni successive saranno quasi istantanee.
> Di default, nei comandi seguenti viene configurata la modalità **Debug** per abilitare tutti i log. Se si desidera compilare in modalità **Release** (ottimizzata e senza log di debug), è sufficiente sostituire `-DCMAKE_BUILD_TYPE=Debug` con `-DCMAKE_BUILD_TYPE=Release` durante la configurazione (o specificare `--config Release` durante la build su Windows/MSVC).

#### 1. Da Riga di Comando

* **Su macOS (con Qt installato via Homebrew - Raccomandato)**:
  ```bash
  # Configura specificando il percorso CMAKE_PREFIX_PATH di Homebrew usando il generatore Ninja
  cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt" -DCMAKE_BUILD_TYPE=Debug

  # Compila il progetto
  cmake --build build -j 8
  ```

* **Su Linux (Ubuntu)**:
  ```bash
  # Configura usando Ninja
  cmake -B build -S . -G Ninja -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Debug

  # Compila il progetto
  cmake --build build
  ```

* **Su macOS (con Qt installato via installer ufficiale / Qt Creator)**:
  ```bash
  # 1. Configura specificando la cartella di Qt nella home (es. versione 6.10.3)
  cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="$HOME/Qt/6.10.3/macos" -DCMAKE_BUILD_TYPE=Debug

  # 2. Compila il progetto
  cmake --build build -j 8
  ```

* **Su Windows e macOS (con Qt installato via `aqtinstall`)**:
  Se hai installato una versione locale di Qt tramite `aqtinstall`, devi indicare a CMake la cartella specifica contenente la versione e il compilatore prescelto (sostituisci `C:/Qt` o `~/Qt` con la tua reale cartella di installazione).

  * **Windows (compilatore MSVC 2022)**:
    ```cmd
    :: Configura specificando il toolchain e la cartella del compilatore MSVC di Qt 6.10.0
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.0/msvc2022_64"
    
    :: Compila specificando la configurazione desiderata (Debug o Release)
    cmake --build build --config Debug
    ```
  * **Windows (compilatore MinGW)**:
    ```cmd
    :: Configura specificando il toolchain e la cartella di MinGW di Qt 6.10.0 in modalità Debug
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="C:/Qt/6.10.0/mingw_64" -DCMAKE_BUILD_TYPE=Debug
    
    :: Compila
    cmake --build build
    ```
  * **macOS (compilatore Clang)**:
    ```bash
    # Configura specificando il toolchain e la cartella clang_64 di Qt 6.10.0 in modalità Debug
    cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" -DCMAKE_PREFIX_PATH="$HOME/Qt/6.10.0/clang_64" -DCMAKE_BUILD_TYPE=Debug
    
    # Compila
    cmake --build build -j 8
    ```

#### 2. Tramite VS Code (estensione CMake Tools)
Se hai completato la **FASE 2**, ti basta:
1. Cliccare sulla voce **"CMake: Configure"** nella Command Palette o salvare il file `settings.json`.
2. Premere **F7** o cliccare sul pulsante **"Build"** nella barra di stato in basso per compilare.

---

### FASE 4: Esecuzione

Una volta completata la compilazione, puoi avviare l'applicazione:

* **Su macOS**:
  ```bash
  open build/BDraw.app
  # oppure da riga di comando:
  ./build/BDraw.app/Contents/MacOS/BDraw
  ```

* **Su Linux (Ubuntu)**:
  ```bash
  ./build/BDraw
  ```

---

### Presentazione della Tesi
**[Zanini_Mattia.pdf](Zanini_Mattia.pdf)**

---
*Autore: Mattia Zanini*  
*Università degli Studi di Padova - Ingegneria Informatica*
