# Release checklist

## Automated / local validation

- [x] C++20 physics and game compile with vendored raylib.
- [x] Every signal has a reproducible, achievable three-star route.
- [x] Physics, deterministic prediction, collision, scoring and save-format tests pass.
- [x] AddressSanitizer and UndefinedBehaviorSanitizer checks pass with LeakSanitizer disabled for this restricted environment.
- [x] Linux desktop runs with software-rendered OpenGL.
- [x] WebAssembly and Windows x64 binaries are actually compiled.
- [x] Cover artwork, icons, original procedural graphics and synthesized audio are included.
- [x] Source, build configurations, licensing and publishing materials are included.

The precise browser and platform checks are recorded in `VALIDATION.md`; this checklist does not imply unperformed tests.

## Owner checks before announcing the release

- [ ] Run the Windows ZIP on a clean Windows 10/11 x64 machine, with both integrated and dedicated graphics if available. Confirm audio, icons, resizing, fullscreen, save/reload and the complete campaign.
- [ ] Try the browser ZIP inside the actual itch.io preview iframe; test initial focus, audio activation, fullscreen, save/reload and orientation changes.
- [ ] Test a physical Android device and an iPhone/iPad in current Chrome/Safari. Mobile emulation is useful but is not a substitute for device testing.
- [ ] Check Firefox and Safari desktop. This environment's automated browser coverage is Chromium.
- [ ] Ask a few fresh players to attempt the tutorial without instructions. Watch for unclear drag direction, accidental burns or frustrating precision demands. Adjust based on observed play.
- [ ] Listen to sound effects and ambient volume on speakers and headphones. Automated audio loading does not establish a comfortable mix.
- [ ] Verify text and trajectories remain readable on a dim mobile screen. Confirm reduced-motion and glow preferences suit you.
- [ ] Review the MIT license and third-party notices, store description, screenshots and build status for accuracy.
- [ ] Keep public builds free of `SLINGSHIFT_TEST_HOOKS` and personal save files.
- [ ] Upload release ZIPs and checksums; download and extract them again to check the hosted files.
- [ ] Publish only after reviewing the actual page and downloads.
