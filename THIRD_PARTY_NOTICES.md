# Third-party notices

SLINGSHIFT's game code, procedural artwork and sound synthesis are supplied under the MIT license in `LICENSE`.

| Component | License | Included notice |
| --- | --- | --- |
| raylib 5.5, commit `c1ab645ca298a2801097931d1079b10ff7eb9df8` | zlib/libpng | `vendor/raylib/LICENSE` |
| GLFW, bundled by raylib | zlib/libpng | `vendor/raylib/src/external/glfw/LICENSE.md` |
| raylib's bundled dependencies, including miniaudio and stb | Individual licenses retained in their source headers | `vendor/raylib/src/external/` |
| DejaVu Sans and DejaVu Sans Mono | Bitstream Vera license and DejaVu public-domain changes | `assets/FONT-LICENSE.txt` |

`Interface.ttf` is unmodified DejaVu Sans; `Telemetry.ttf` is unmodified DejaVu Sans Mono. Their internal font names remain unchanged. No third-party planet images, music recordings, texture packs, paid assets, web fonts or remotely loaded assets are used.

The supplied source contains the vendored dependency notices. Release ZIPs include the raylib, GLFW, and font notices. The browser's Emscripten runtime and the Windows compiler runtime carry their upstream permissive/runtime-exception licenses; relevant notices are included in `licenses/` in the distribution packages.

The promotional cover image `docs/store-cover.png` is separate from the in-game procedural artwork and is not included in the MIT license grant for the game code and in-game assets.
