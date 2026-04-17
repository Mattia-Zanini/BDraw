# Guida alla Gestione delle Dipendenze con VCPKG

Questa guida spiega come configurare il progetto **BDraw** per gestire le librerie esterne (`spdlog`, `armadillo`, `openblas`) in modo automatico su **Windows, macOS e Linux**.

## Perché usare VCPKG?
VCPKG è un package manager per C++ che risolve il problema delle librerie "mancanti" o difficili da compilare (come LAPACK/BLAS per Armadillo su Windows). Permette di scaricare e compilare tutto il necessario con un singolo comando.

---

## 1. Installazione di VCPKG (Una sola volta)

Scegli una cartella sul tuo PC (es. la tua home o `C:\`) e clona il repository ufficiale:

### Su macOS / Linux
```bash
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=$HOME/vcpkg # Aggiungi questo al tuo .zshrc o .bashrc
```

### Su Windows (PowerShell)
```powershell
git clone https://github.com/microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat
```

---

## 2. Configurazione del Progetto (`vcpkg.json`)

Nella root del progetto BDraw, crea un file chiamato `vcpkg.json`. Questo file elenca le dipendenze:

```json
{
  "name": "bdraw",
  "version-string": "0.1.0",
  "dependencies": [
    "spdlog",
    "armadillo",
    "openblas"
  ]
}
```
*Nota: `openblas` fornisce automaticamente LAPACK e BLAS in modo ottimizzato.*

---

## 3. Modifiche al `CMakeLists.txt`

Per far sì che CMake trovi le librerie scaricate da VCPKG, modifica il tuo `CMakeLists.txt` come segue:

```cmake
# Prima di add_executable
find_package(spdlog CONFIG REQUIRED)
find_package(Armadillo CONFIG REQUIRED)

# ... dopo qt_add_executable ...

target_link_libraries(BDraw PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
    spdlog::spdlog
    Armadillo::Armadillo
)
```

---

## 4. Come Compilare

Il segreto è passare a CMake il "Toolchain File" di VCPKG. Questo dice a CMake: *"Ehi, prima di cercare nel sistema, guarda cosa ha scaricato VCPKG"*.

### macOS / Linux
```bash
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build -j 8
```

### Windows (VS Code / PowerShell)
Sostituisci il percorso con quello dove hai installato VCPKG:
```powershell
cmake -B build -S . `
  -DCMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" `
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build
```

---

## 5. Integrazione con VS Code (Consigliato)

Per evitare di scrivere ogni volta il percorso del toolchain, aggiungi questa riga al tuo `.vscode/settings.json`:

```json
{
    "cmake.configureSettings": {
        "CMAKE_TOOLCHAIN_FILE": "/Users/TUO_UTENTE/vcpkg/scripts/buildsystems/vcpkg.cmake"
    }
}
```
*(Su Windows usa i backslash raddoppiati: `C:\\vcpkg\\...`)*

---

## Vantaggi Finali
1. **Zero Stress su Windows**: Non devi più cercare DLL o file `.lib` sparsi per il disco.
2. **Armadillo Funzionante**: `openblas` viene compilato e linkato automaticamente.
3. **Portabilità**: Chiunque scarichi il tuo progetto deve solo installare VCPKG e lanciare il comando di build.
