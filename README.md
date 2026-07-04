# PICO-RAY OS

**An Extended, Open-Source Fantasy Operating System & Hybrid Retro Game Engine built in C.**

PICO-RAY OS is a lightweight, multi-platform, all-in-one virtual retro workstation designed for game development, computer science education, and absolute creative freedom. Inspired by the minimalism of PICO-8 and the flexibility of modern game loops, it bridges the gap between romantic 16-bit limitations and powerful architectural scaling.

With **zero heavy external dependencies**, PICO-RAY OS boots straight into a unified desktop environment featuring built-in development suites, running seamlessly across **Linux, macOS, Windows, FreeBSD, and Haiku OS**.

---

## Key Architectural Features

* **Extended 32-Color Neon/C64 Palette:** True 5-bit color space matrix depth unlocking vibrant retro cyber aesthetics and nostalgic home-computer styles.
* **Dynamic Stride Sheet Alignment:** A revolutionary 2-parameter cartridge parser (`PR_LoadRayCartridge`) that dynamically adjusts row stride math based on the loaded file geometry. No slanted shearing or alignment anomalies, supporting any custom asset widths (64px, 128px, 256px+).
* ** Encapsulated Low-Level Kernel:** 100% object encapsulation via secure system getters (`PR_GetKernelState()`, `PR_GetAudioSystem()`), eliminating volatile global mutations.
* **Structured Dual-Channel Audio Engine:** Powered by Raylib audio mixing, separating continuous dynamic background streams (`music_play`) from rapid, one-shot polyphonic sound effects (`sfx_play`) with an integrated 10% step master volume controller and instant **MUTE** support.
* **Native UTF-8 Multibyte Typography:** A full structural character dictionary system natively supporting Middle-European accents, P8SCII icons, and custom sizes (3x5 small text up to 10x8 fonts).
* **Zero-Footprint Unified Cartridges:** Games, tilemaps, meta-properties, and scripts are combined entirely into single, highly portable `.ray` cartridge files.

---

## Integrated Native OS Suites

PICO-RAY OS is entirely self-sufficient, providing built-in micro-apps out of the box:

1. **Sprite Editor / Pixel Studio:** A pixel-art canvas featuring a bulletproof, iterative queue-based **Flood Fill (BKT) Tool** with strict 8x8 sprite boundary locking.
2. **Font Viewer & Studio:** An interactive typography workspace allowing Left-Click selection hooks, universal `ESCAPE` un-docking, and a Right-Click bitwise hardware eraser. It generates pixel-perfect preview grid updates and prints ready-to-use C-array rows directly to your terminal.
3. **Lua Runner & Live Reloader:** A pristine embedded Lua virtual machine environment providing immediate execution channels, comprehensive input sandboxing, and compilation exceptions debugging.

---

## System Development Roadmap

- [x] **Core Graphics & Direct VRAM Rasterizer** (Pixel-perfect clipping, transparency maps)
- [x] **Flat Linear Sequential Input Gates** (Input passthrough protection shields)
- [x] **Dynamic Context Font Editor & UTF-8 Decoder**
- [x] **Master Streaming & One-Shot Audio Mixer Infrastructure**
- [ ] **Procedural Chiptune Wave Synthesizer** (8-bit waveforms generation engine)
- [ ] **Dual-Pane Minimal Midnight Commander File Manager** (`app_commander.c`)
- [ ] **Shell / Console Terminal Subsystem** (Real disk operations: `ls`, `cd`, `mkdir`, `pwd`)
- [ ] **3D Low-Poly Mesh Object Viewer** (Ultra-lightweight PicoCAD-style matrix projection renderer)
- [ ] **Commodore 64 native tinySID Emulator Integration**

---

## Educational & Indie Philosophy

PICO-RAY OS is fundamentally designed to transform children and enthusiasts from **passive consumers** of digital content into **autonomous creators**. 

Modern game engines (Unity, Godot) are bloated with massive configuration interfaces and deep learning curves that smother immediate creative impulses. PICO-RAY OS returns to a pure, transparent **"Code-First"** environment. By exposing the virtual VRAM, memory registers, and raw byte structures inside the full-screen telemetry debugger (`F4`), children can visually grasp how a computer executes input, logic, and rendering pipelines. 

It is completely open-source, non-restrictive, and dedicated to the absolute preservation and free sharing of computer science knowledge.

---

## Acknowledgements & Inspirations

PICO-RAY OS is a love letter to the retro computing scene and stands on the shoulders of giants. We would like to explicitly thank and acknowledge the original creators of the platforms that inspired our virtual architecture:

- **PICO-8 (Lexaloffle Games / Joseph White):**
  For pioneering the fantasy console paradigm, the iconic P8SCII font aesthetics, and the master 16-color base palette configuration that shaped our default compatibility layers.
- **Pyxel (Kitao Takashi):**
  For demonstrating how a clean, modern pixel-art game engine can be beautifully packaged and structured for multi-platform developer workflows.
- **Haiku OS:**
  For inspiring our clean modular C desktop philosophies, our strict encapsulation rules, and our choice of the permissive MIT License.

---

## Building from Source

PICO-RAY OS relies exclusively on **pure C**, **Lua**, and **Raylib** with zero complex framework overhead.

```bash
# Clone the repository
git clone https://github.com
cd pico-ray-os

# Execute the central shell compiler pipeline script
chmod +x build.sh
./build.sh
```

---

## License

This project is open-source and released under the **MIT License**. Feel free to read, study, modify, hack, and distribute it to any corner of the universe!
