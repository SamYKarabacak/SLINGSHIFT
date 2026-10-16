#!/usr/bin/env python3
"""Package existing builds. No compilation, uploads or publishing happen here."""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]
VERSION = "1.0.0"


def add(zip_file: zipfile.ZipFile, file: Path, name: str) -> None:
    if not file.is_file():
        raise FileNotFoundError(file)
    zip_file.write(file, name)


def notices(zip_file: zipfile.ZipFile) -> None:
    for name in ("LICENSE", "THIRD_PARTY_NOTICES.md"):
        add(zip_file, ROOT / name, name)
    add(zip_file, ROOT / "assets/FONT-LICENSE.txt", "licenses/FONT-LICENSE.txt")
    add(zip_file, ROOT / "vendor/raylib/LICENSE", "licenses/raylib-LICENSE.txt")
    add(zip_file, ROOT / "vendor/raylib/src/external/glfw/LICENSE.md", "licenses/GLFW-LICENSE.txt")
    # Retain dependency headers verbatim, including their original full license blocks.
    external = ROOT / "vendor/raylib/src/external"
    for file in sorted(external.glob("*.h")):
        add(zip_file, file, "licenses/raylib-dependency-sources/" + file.name)
    for file in sorted((ROOT / "licenses").glob("*")):
        if file.is_file():
            add(zip_file, file, "licenses/" + file.name)


def make_zip(file: Path) -> zipfile.ZipFile:
    return zipfile.ZipFile(file, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--web", type=Path, help="directory containing index.html/js/wasm/data")
    parser.add_argument("--windows", type=Path, help="directory containing slingshift.exe and assets")
    parser.add_argument("--linux", type=Path, help="directory containing slingshift and assets")
    parser.add_argument("--output", type=Path, default=ROOT / "dist")
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    packages: list[Path] = []

    source = args.output / f"SLINGSHIFT-{VERSION}-source.zip"
    allowed = {"src", "tests", "tools", "assets", "docs", "web", "cmake", "vendor", "licenses", ".github"}
    with make_zip(source) as zip_file:
        for file in sorted(ROOT.rglob("*")):
            if not file.is_file():
                continue
            relative = file.relative_to(ROOT)
            if len(relative.parts) == 1 or relative.parts[0] in allowed:
                if "__pycache__" not in relative.parts:
                    add(zip_file, file, "SLINGSHIFT/" + relative.as_posix())
    packages.append(source)

    if args.web:
        folder = args.web.resolve()
        names = ("index.html", "index.js", "index.wasm", "index.data", "icon.png")
        for name in names:
            if not (folder / name).is_file():
                raise FileNotFoundError(folder / name)
        if (folder / "index.wasm").read_bytes()[:4] != b"\0asm":
            raise ValueError("Invalid WebAssembly signature")
        js = (folder / "index.js").read_text(encoding="utf-8")
        if "SlingQA" in js or "sling_qa_state" in js:
            raise ValueError("QA hooks are enabled. Rebuild a public configuration.")
        target = args.output / f"SLINGSHIFT-{VERSION}-web.zip"
        with make_zip(target) as zip_file:
            for name in names:
                add(zip_file, folder / name, name)
            notices(zip_file)
            zip_file.writestr("PLAY.txt", "Upload this ZIP as an HTML game on itch.io, or serve this extracted folder over HTTP. index.html is the entry point.\nSee the source project's docs/VALIDATION.md for testing coverage.\n")
        with zipfile.ZipFile(target) as zip_file:
            assert "index.html" in zip_file.namelist() and zip_file.testzip() is None
        packages.append(target)

    for platform, requested, executable, signature in (
        ("windows-x64", args.windows, "slingshift.exe", b"MZ"),
        ("linux-x64", args.linux, "slingshift", b"\x7fELF"),
    ):
        if requested is None:
            continue
        folder = requested.resolve()
        binary = folder / executable
        if binary.read_bytes()[:len(signature)] != signature:
            raise ValueError(f"Invalid {platform} executable signature")
        if not (folder / "assets/Interface.ttf").is_file():
            raise FileNotFoundError(folder / "assets/Interface.ttf")
        target = args.output / f"SLINGSHIFT-{VERSION}-{platform}.zip"
        with make_zip(target) as zip_file:
            add(zip_file, binary, executable)
            for file in sorted((folder / "assets").rglob("*")):
                if file.is_file() and file.suffix in {".ttf", ".png", ".txt"}:
                    add(zip_file, file, file.relative_to(folder).as_posix())
            notices(zip_file)
            add(zip_file, ROOT / "docs/VALIDATION.md", "VALIDATION.md")
            zip_file.writestr("PLAY.txt", f"SLINGSHIFT {VERSION}\nExtract the entire ZIP, then run {executable}. Keep assets alongside it.\nDrag backward from the spacecraft and release to launch. Drag empty space for fine aim.\nR retries. Escape pauses. See VALIDATION.md for the platform's testing status.\n")
        packages.append(target)

    hashes = []
    for package in packages:
        with zipfile.ZipFile(package) as zip_file:
            if zip_file.testzip() is not None:
                raise ValueError(f"Corrupt ZIP: {package}")
        digest = hashlib.sha256(package.read_bytes()).hexdigest()
        hashes.append(f"{digest}  {package.name}")
        print(f"{package.name}: {package.stat().st_size:,} bytes")
    (args.output / "SHA256SUMS.txt").write_text("\n".join(hashes) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
