# Validation report

Prepared 16 October 2026. These are observed build and test results, not a claim that every platform has been tested.

## Platform status

| Target | Build | Runtime verification |
| --- | --- | --- |
| Browser / WebAssembly | Built with Emscripten 3.1.6 and raylib 5.5 | Passed automated Chromium 153.0.8010.0 playthrough; desktop and emulated touch input |
| Windows x64 | Cross-compiled with MinGW GCC 13; Windows GUI PE executable, icon and version resources | **Not runtime-verified on Windows.** Wine could not start because the environment prohibits its server socket |
| Linux x64 | Built on Ubuntu 24.04.3 with GCC 13.3 and CMake 3.28.3 | Ran under Xvfb with software OpenGL; verified actual game-loop arrivals on signals 1, 9 and 12 |
| Visual Studio | Complete CMake configuration/preset supplied | Visual Studio itself was unavailable; that generator was not executed |
| Physical Android / iOS browsers | Responsive web target supplied | **Not physically tested.** Chromium touch emulation is not a device test |
| Native Android / iOS | No native package | Outside this release's scope |

At the time the original build validation was prepared, there was no hosted deployment. The owner subsequently published the game at [itch.io](https://samykar.itch.io/slingshift) and reported successfully launching and playing it. The original automated browser test results below remain separate from that owner-reported manual check.

## Automated checks

**112 C++ checks passed:** 103 physics/game-rule checks and 9 save-format checks, in two CTest executables. Coverage includes:

- Gravity direction, falloff, superposition and finite values at singular-looking positions.
- Zero-force motion, circular-orbit closure, energy drift and angular momentum.
- Swept collision, tunneling, tangencies, initial overlap, gate/hazard ordering and chart exits.
- Invalid state/timestep containment, timeout, terminal-state handling and correction limits.
- Deterministic replay and exact prediction/actual equality within the tested build.
- Three-star solutions for all twelve levels, plus small aim-perturbation checks.
- Versioned save round trips, progression, invalid/truncated data and preservation of a good profile on decode failure.

AddressSanitizer and UndefinedBehaviorSanitizer also passed these tests. LeakSanitizer could not inspect processes in this restricted environment; it was disabled with `ASAN_OPTIONS=detect_leaks=0`. No leak-check pass is claimed.

**84 browser checks passed.** The suite drove actual mouse, keyboard and touch events against the compiled C++ game:

- All twelve signals completed sequentially through the real fixed-step game loop, earning 36 stars.
- Slingshot release, fine refinement, keyboard launch, pause/frozen time, resume, failure and instant retry.
- Progress persisted after reload; volume changes persisted in the save.
- A correction drag slowed time, release consumed the burn, and a second burn was refused.
- A fresh portrait touch profile completed the first signal; touch refinement completed the final signal.
- Cancelled touches did not launch a ship; changing orientation preserved a usable game state.
- No uncaught JavaScript errors were observed.

The suite uses an opt-in QA build to read state and known route fixtures. The campaign itself is played through normal input and normal progression. Public builds omit those hooks; packaging rejects a web build containing them. See `browser-validation.json` for the exact assertions and measured campaign flight times.

`level-verification.txt` contains the native physics test's route output. `tools/browser_qa.cjs` reproduces browser validation. Screenshots in `docs/screenshots/` are captured from the running game; the cover is separately rendered from its procedural art.

## Distribution archive checks

The produced archives were extracted into fresh directories. The extracted source configured, rebuilt and passed both CTest executables. The extracted Linux download completed the final signal through the actual game loop. The extracted public browser download passed eight additional checks: QA hooks were absent, desktop and touch launches each earned three stars, versioned progress was written, a synthesized-audio AudioContext was running, landscape resizing worked, and no missing HTTP assets or uncaught JavaScript errors were observed. `package-validation.json` records these checks and the tested WebAssembly hash.

## Remaining limits

Windows binaries are genuine compiled executables, but native Windows startup, driver behavior, audio and filesystem behavior remain unverified. The Windows-compiled test executables also could not run under blocked Wine. The executable's import table was inspected: only Windows system DLLs are required; MinGW C++ runtime DLLs are statically linked.

Firefox, Safari, actual mobile devices and the itch.io embed were not available for validation. Browser audio initialized during automated interaction without a runtime error, but headless testing does not establish a comfortable sound mix. Human usability and enjoyment testing remains advisable before announcing a public release.

There are no known missing core game systems. The remaining release checks concern platform coverage, hosted distribution and fresh-player feedback; use `RELEASE_CHECKLIST.md` before public announcement.
