# Allpix2likeMALTA2Behavior

[![Framework](https://img.shields.io/badge/Framework-Allpix%20Squared-blue.svg)](https://project-allpix-squared.web.cern.ch/)
[![Data Format](https://img.shields.io/badge/Data%20Format-ROOT%20TTree%20%28MALTA2%20Native%29-orange.svg)](#-malta2-digital-encoding-logic)

🌐 **Language Switch:** [English](#english-version) | [日本語](#日本語バージョン)

---

## English Version

### 🚀 System Architecture
This repository contains a highly optimized custom output module (`Malta2TreeWriterModule`) for **Allpix Squared**. 
It intercepts simulated `PixelHitMessage` data during runtime and dynamically translates the continuous spatial and timing physics observables into the exact native digital hardware encoding format used by the **MALTA2 Monolithic Active Pixel Sensor (MAPS)**.

### 📂 Repository Structure
```text
.
├── .gitignore                   # Excludes heavy sim datasets (*.root) and build artifacts
├── Malta2TreeWriterModule.hpp   # Module header definitions
└── Malta2TreeWriterModule.cpp   # Implementation file (Digital encoding & ROOT Tree filling)
```

### ⚡ MALTA2 Digital Encoding Logic

The core of this module maps the global simulated pixel indices $(x, y)$ and time-of-arrival into MALTA2 specific chip registers and logic layouts inside the run() loop:
* dcolumn & group: Computes double-column addresses ($x / 2$) and pixel group segments ($y / 16$).
* pixel Bitmask: Reconstructs the 16-bit hit layout using lower parity offsets ((1 << pix_index)).
* bcid, winid, phase: Digitizes the continuous simulation hit timestamp (hit.getLocalTime()) into timing bins of 25.0 ns (Bunch Crossing ID), 3.125 ns (Window ID), and 0.39 ns (Fine Phase) respectively.

### 💾 File Serialization

* Multi-Detector Tracking: Automatically detects all registered telescope planes via the GeometryManager. It spawns isolated, parallel ROOT output trees
* (run_######_[plane_index].root) dynamically mapped to each independent detector name.

*Metadata Persistence: Stores full configuration dumps from ConfigManager inside a dedicated config/ directory within the ROOT file structure during finalize(), ensuring absolute reproducibility of simulation parameters.
