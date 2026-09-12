// cspell:ignore substepping
#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace BDraw {

  namespace Constants {
    inline constexpr double gravity = 9.81;                      // accelerazione di gravità (m/s^2)
    inline constexpr double threshold = 1e-6;                    // soglia minima per le operazioni
    inline constexpr double minMoveDistance = 1.0;               // distanza minima fra un campione e l'altro (del disegno libero)
    inline constexpr int margin = 30;                            // margine dai bordi della scena
    inline constexpr int ballRadius = 6;                         // raggio grafico dei gravi in pixel
    inline constexpr int deltaTimeMilliseconds = 16;             // millisecondi tra un frame e il successivo, 16 ms ~= 60 FPS
    inline constexpr double deltaTimeSeconds = 0.016;            // espresso in secondi
    inline constexpr double maxTimeElapsed = 0.05;               // soglia di sicurezza per evitare che la simulazione scatti (circa 3 frame persi)
    inline constexpr double fixedSubDT = 0.00416666666666666666; // ~4.2 ms (240 Hz)
  } // namespace Constants

} // namespace BDraw

#endif // CONSTANTS_H
