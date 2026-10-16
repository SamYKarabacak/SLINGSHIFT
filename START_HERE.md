# SLINGSHIFT 1.0

The source project contains the complete C++20 game, twelve levels, tests, procedural art/audio, build configurations and publishing materials.

- **Browser:** [play SLINGSHIFT on itch.io](https://samykar.itch.io/slingshift). To host a build yourself, use `SLINGSHIFT-1.0.0-web.zip`; its `index.html` is at the ZIP root. For local play, extract the ZIP, run `python3 -m http.server 8000` in that folder, and open `http://localhost:8000`.
- **Windows:** extract `SLINGSHIFT-1.0.0-windows-x64.zip` completely and run `slingshift.exe`. Keep `assets` beside it. The executable was cross-compiled, but native Windows execution is not yet verified.
- **Linux:** extract `SLINGSHIFT-1.0.0-linux-x64.zip`; mark `slingshift` executable if your extractor did not retain permissions, then run it. Ubuntu 24.04 is the tested build baseline.
- **Source:** extract `SLINGSHIFT-1.0.0-source.zip` and open its `SLINGSHIFT` directory in Visual Studio or another CMake-capable editor. Begin with `README.md`.

Pull backward from the ship and release to launch. Drag empty space to refine your aim before pressing Launch. In flight, pull anywhere for one emergency burn. R retries. Escape pauses.

The game passed 112 C++ checks and 84 automated browser checks, including a full campaign playthrough. See `docs/VALIDATION.md` for the exact coverage and `docs/PUBLISHING.md` for itch.io and GitHub instructions. `docs/cover.png` and `docs/screenshots/` provide store artwork.

Created by **Sam Y. Karabacak**. See `CREDITS.md` for the project credit and `THIRD_PARTY_NOTICES.md` for license notices.
