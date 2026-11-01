# DX12Renderer

DirectX 12 renderer developed during the Chaos DirectX 12 Ray Tracing Course 2026.

## Features

- Selects the first hardware GPU adapter that supports Direct3D 12 (feature level 11_0), skipping software adapters
- Enables the D3D12 debug layer in Debug builds
- Creates a direct command queue, command allocator and graphics command list
- Creates an 800x600 render target texture with a render target view
- Renders frames by clearing the render target to a color that changes with the frame index (red, green, blue)
- Copies each frame to a readback buffer, waits for the GPU with a fence and saves it as a PPM image

## Requirements

- Windows 10 or later, x64
- GPU with Direct3D 12 support (feature level 11_0 or higher)
- Visual Studio 2022 with the "Desktop development with C++" workload
- Python 3, available as `python` in PATH (used by the build events)
- Qt 6.11.3 for MSVC 2022 64-bit
- Optional, for the D3D12 debug layer: the Windows optional feature "Graphics Tools"

## Setup

Qt is not stored in the repository. Install Qt 6.11.3 (MSVC 2022 64-bit) with the Qt Online Installer, then copy these folders from `C:\Qt\6.11.3\msvc2022_64` into `third_party\Qt6`:

```
bin
include
lib
plugins
qml
```

## Building

1. Open `DX12Renderer.sln`
2. Select `Debug | x64` or `Release | x64`
3. Build > Build Solution

- C++ standard: C++17
- Output: `bin\Debug` or `bin\Release`
- Intermediate files: `obj\x64\Debug` or `obj\x64\Release`
- The pre-build event runs Qt's moc through `scripts\gen_prebuild_cmd.py`
- The post-build event copies the Qt DLLs and the `platforms` plugin next to the executable through `scripts\postbuild.py`

## Running

From Visual Studio: Debug > Start Debugging (F5). D3D12 debug layer messages appear in the Output window.

The program renders three frames and saves them as `frame_0.ppm`, `frame_1.ppm` and `frame_2.ppm` in the working directory: the repository root when started from Visual Studio, otherwise the folder it is started from. PPM images can be opened with GIMP or IrfanView.

The program takes no command line arguments.

Prebuilt executables are available under [Releases](https://github.com/Cvetelinavll/chaos-dx12/releases). The release builds need the Microsoft Visual C++ Redistributable (x64).

## Project structure

| Path | Purpose |
|---|---|
| `src/DX12Renderer.cpp` | Entry point, runs the renderer |
| `src/Renderer.h`, `src/Renderer.cpp` | DirectX 12 setup, frame rendering and image output |
| `scripts/` | Build event scripts (moc, copying Qt DLLs) |
| `third_party/Qt6/` | Qt 6, copied locally, not in git |
| `bin/`, `obj/` | Build output, not in git |

## Lecture branches

Each lecture is developed in its own branch, based on the previous one. `main` always contains the latest finished lecture.

| Branch | Release | Content |
|---|---|---|
| `lecture-01` | | Enumerate GPU adapters with DXGI |
| `lecture-02` | v0.2 | Device, command queue, allocator and command list; Renderer class |
| `lecture-03` | v0.3 | Render target, readback buffer, fence, multiple frames; review fixes; repository restructure; Qt linked |
