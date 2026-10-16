# Publishing SLINGSHIFT

SLINGSHIFT is [playable on itch.io](https://samykar.itch.io/slingshift). This guide describes publishing and maintaining its browser, desktop and source releases.

## itch.io

1. Create a new project on your own itch.io account. Set its title to **SLINGSHIFT** and its kind to **HTML** so the browser game can run on the page.
2. Upload `SLINGSHIFT-1.0.0-web.zip` and mark it as the file to play in the browser. The ZIP must contain `index.html`, `index.js`, `index.wasm`, `index.data` and `icon.png` directly at its root; do not wrap them in another directory.
3. Set an initial embed size of **1280 by 800**, enable fullscreen, and enable mobile support. The canvas fits the browser viewport and supports both orientations. Start with click-to-play so audio activation and keyboard focus are clear.
4. Upload the Windows ZIP as an additional downloadable file and mark that download as Windows. Include the Linux build only if you want to support the tested Linux baseline. Do not label a build as Android or iOS native.
5. Use `docs/cover.png` (1260 by 1000) or `docs/cover-small.png` (630 by 500) for the cover. Add the actual screenshots in `docs/screenshots/`. Paste or adapt `docs/ITCH_DESCRIPTION.md` as the description. Suggested tags: puzzle, space, physics, short, relaxing, 2d, singleplayer.
6. Save the page as a draft or restricted preview first. Load the embedded game, play through a gate, reload, check the saved score, test fullscreen, and test a phone. Check that files load inside the itch.io iframe; localhost tests do not establish hosting compatibility.
7. Complete the remaining manual checks in `RELEASE_CHECKLIST.md`. Once satisfied, change the page's visibility to public.

The web ZIP is packaged to itch.io's documented HTML5 layout. The game is now [playable on itch.io](https://samykar.itch.io/slingshift); hosting was completed by the owner after these source files were originally packaged. See the current [HTML5 upload guide](https://itch.io/docs/creators/html5) and [creator setup guide](https://itch.io/docs/creators/getting-started) for the service's current interface.

## GitHub

Use a **new, empty repository**. The source archive is a standalone project and contains no fabricated Git history. Leave existing repositories untouched.

1. Extract the source ZIP and open a terminal in its `SLINGSHIFT` directory.
2. Read `LICENSE`, `THIRD_PARTY_NOTICES.md`, and the validation report. Add your own truthful project description if desired.
3. Create a new repository called `SLINGSHIFT` on GitHub. Initialize the local repository and make its first real commit:

```sh
git init
git add .
git commit -m "Release SLINGSHIFT 1.0 source"
git branch -M main
git remote add origin https://github.com/SamYKarabacak/SLINGSHIFT.git
git push -u origin main
```

4. Set the repository description to “A browser-playable orbital physics puzzle game built with C++20, raylib, and WebAssembly.” The README uses relative screenshot links and should render directly.
5. The included GitHub Actions workflow builds and tests Windows and Linux when you push. A separate manually triggered web workflow builds with a pinned Emscripten SDK. These workflows are supplied configurations, not evidence that CI has already run.
6. After your own Windows testing and CI pass, create a `v1.0.0` release. Attach the tested Windows ZIP, browser ZIP, source ZIP and `SHA256SUMS.txt`. Copy the relevant platform validation notes into the release description. Do not imply native Windows testing until it has occurred.
7. Set the repository's About website URL to `https://samykar.itch.io/slingshift`; the README already links to the published page.
8. Retain the creator credit, **Sam Y. Karabacak**, in the project and release materials, and include all required license notices with distributions.

Official instructions: [adding local code to GitHub](https://docs.github.com/en/migrations/importing-source-code/using-the-command-line-to-import-source-code/adding-locally-hosted-code-to-github) and [managing releases](https://docs.github.com/en/repositories/releasing-projects-on-github/managing-releases-in-a-repository).

## Repackage a new version

Build the intended targets with test hooks disabled, run the appropriate tests, update the version and validation report, then run:

```sh
python3 tools/package_release.py --web build/web --windows build/windows --linux build/linux
```

For a Visual Studio build, use `--windows build/vs2022/Release`. The script only packages requested, present builds. It checks the browser entry files and the Windows PE signature, includes notices, creates a source ZIP, and writes SHA-256 sums. Packaging checks do not replace runtime testing.
