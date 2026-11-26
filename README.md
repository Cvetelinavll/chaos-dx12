# DX12Renderer

DirectX 12 renderer developed during the Chaos DirectX 12 Ray Tracing Course 2026.

## Features

- Selects the first hardware GPU adapter that supports Direct3D 12 (feature level 11_0), skipping software adapters
- Enables the D3D12 debug layer in Debug builds
- Qt main window with a viewport widget
- Renders directly into the viewport through a DXGI swap chain with 2 buffers (FLIP_DISCARD)
- Tracks the current back buffer and presents each frame after the command list executes
- Draws a triangle with the GPU rendering pipeline: vertex buffer in an upload heap, root signature, graphics pipeline state, viewport and scissor rectangle
- HLSL vertex and pixel shaders compiled at build time by dxc into C++ headers
- Rotates the triangle around the center by updating the vertex buffer every frame
- Rendering loop driven by a QTimer, running whenever the Qt event loop is idle
- FPS counter in the status bar, updated every second by a separate QTimer
- Waits for the GPU to finish before the window closes

## Requirements

- Windows 10 or later, x64
- GPU with Direct3D 12 support (feature level 11_0 or higher)
- Visual Studio 2022 with the "Desktop development with C++" workload
- Python 3, available as `python` in PATH (used by the build events)
- Qt 6.11.3 for MSVC 2022 64-bit

## Setup

Qt is not stored in the repository. Install Qt 6.11.3 (MSVC 2022 64-bit) with the Qt Online Installer, then copy these folders from `C:\Qt\6.11.3\msvc2022_64` into `third_party\Qt6`:

```
bin
include
lib
plugins
qml
```
The D3D12 headers and the `d3dx12` helpers from [DirectX-Headers](https://github.com/microsoft/DirectX-Headers) are included in `third_party\d3dx12`, so they need no setup.

## Building

1. Open `DX12Renderer.sln`
2. Select `Release | x64` or `Debug | x64`
3. Build > Build Solution

- C++ standard: C++17
- Output: `bin\Release` or `bin\Debug`
- Intermediate files: `obj\x64\Release` or `obj\x64\Debug`
- The pre-build event runs Qt's moc for the classes listed in `scripts\gen_prebuild_cmd.py` and writes the results to `src\qtgen`
- The HLSL shaders in `src\shaders` are compiled with Shader Model 6.3 into `obj\x64\<Configuration>\CompiledShaders\<Name>.hlsl.h`, which `Renderer.cpp` includes
- The post-build event copies the Qt DLLs and the `platforms` and `styles` plugins next to the executable through `scripts\postbuild.py`

When adding a new class with `Q_OBJECT`, add its header name to `moc_files` in `scripts\gen_prebuild_cmd.py`, build once, then add the generated `src\qtgen\moc_<Name>.cpp` to the project.

## Running

From Visual Studio: Debug > Start Without Debugging (Ctrl+F5). Use Release for a representative FPS. In Debug, start with F5 to see D3D12 debug layer messages in the Output window.

The window shows the rendered frames live, with the FPS at the bottom right. Close the window to quit.

The program takes no command line arguments.

Prebuilt executables are available under [Releases](https://github.com/Cvetelinavll/chaos-dx12/releases). The release builds need the Microsoft Visual C++ Redistributable (x64).

## Project structure

| Path | Purpose |
|---|---|
| `src/DX12Renderer.cpp` | Entry point: creates the Qt application and `MyApp` |
| `src/MyApp.h`, `src/MyApp.cpp` | Links GUI and renderer: rendering loop, FPS calculation, startup and shutdown |
| `src/MainWindow.h`, `src/MainWindow.cpp` | Main window: viewport and status bar with the FPS |
| `src/ViewportWidget.h` | Viewport widget; provides its native window handle (HWND) for the swap chain |
| `src/Renderer.h`, `src/Renderer.cpp` | DirectX 12 renderer: device and command setup, swap chain, frame rendering and presenting, GPU synchronization |
| `scripts/` | Build event scripts (moc, copying Qt DLLs) |
| `third_party/d3dx12/` | D3D12 headers and `d3dx12` helpers from DirectX-Headers (MIT license) |
| `third_party/Qt6/` | Qt 6, copied locally, not in git |
| `bin/`, `obj/`, `src/qtgen/` | Build output and generated files, not in git |

## Lecture branches

Each lecture is developed in its own branch, based on the previous one. `main` always contains the latest finished lecture.

| Branch | Release | Content |
|---|---|---|
| `lecture-01` | | Enumerate GPU adapters with DXGI |
| `lecture-02` | v0.2 | Device, command queue, allocator and command list; Renderer class |
| `lecture-03` | v0.3 | Render target, readback buffer, fence, multiple frames; review fixes; repository restructure; Qt linked |
| `lecture-04` | v0.4 | Qt main window, viewport, QTimer rendering loop, FPS counter |
| `lecture-05` | v0.5 | Swap chain with 2 buffers presenting into the viewport, RTV descriptor heap, back buffer tracking |
| `lecture-07` | v0.7 | Triangle with the GPU pipeline: vertex buffer, HLSL shaders, root signature, pipeline state, viewport; moving, checker pattern and rotating triangle |