# Publishing SLINGSHIFT

The game is [playable on itch.io](https://samykar.itch.io/slingshift).
Use this guide when updating the browser or desktop downloads.

## Update itch.io

1. Build the browser release with `SLINGSHIFT_TEST_HOOKS` disabled and package it
   with `tools/package_release.py`.
2. Upload the web ZIP to the existing project as the browser-playable file.
   Its root must contain `index.html`, `index.js`, `index.wasm`,
   `index.data` and `icon.png`, without an extra enclosing directory.
3. Check the embed at 1280 by 800, with fullscreen and mobile support enabled.
   Test focus and audio activation, a completed level, save/reload, fullscreen,
   and orientation changes in the hosted iframe.
4. Upload tested desktop packages as downloads with the correct platform labels.
   The browser game is not a native Android or iOS package.
5. Update the description, version, artwork and gameplay screenshots as needed.
   `docs/ITCH_DESCRIPTION.md`, `docs/cover.png` and `docs/screenshots/`
   contain the store materials.
6. Complete the [manual release checks](RELEASE_CHECKLIST.md) before making
   updated downloads public.

For upload details, see itch.io's [HTML5 guide](https://itch.io/docs/creators/html5).

## GitHub releases

Source and build instructions are in the
[repository](https://github.com/SamYKarabacak/SLINGSHIFT).

The desktop workflow builds and tests Windows and Linux on push. The web
workflow runs manually with a pinned Emscripten SDK. Check each run's results
in Actions; the workflows and [validation report](VALIDATION.md) cover
different checks.

For a new release, update the version and notes, run the relevant builds and
tests, then tag the source. Attach the tested desktop, browser and source ZIPs
with `SHA256SUMS.txt`. Include platform limitations in the release notes and
keep the project credit and required license notices in each distribution.

## Package builds

With the desired targets already built, run:

```sh
python3 tools/package_release.py --web build/web --windows build/windows --linux build/linux
```

For Visual Studio, use `--windows build/vs2022/Release`. Omit targets you are
not distributing.

The script checks browser entry files and the Windows PE signature, includes
license notices, creates a source ZIP, and writes SHA-256 sums. Test the
extracted packages as well; packaging checks do not exercise the application.
