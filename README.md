# DX12Renderer

DirectX 12 renderer developed during the Chaos DirectX 12 Ray Tracing Course 2026.

## Features

- Enumerates all GPU adapters and prints the adapter name (vendor and model) and dedicated video memory
- Creates an `ID3D12Device` on the first hardware adapter supporting feature level 11_0
- Creates a direct command queue, command allocator and graphics command list
- Reports creation failures and exits with code 1

## Requirements

- Windows 10 or later, x64
- GPU with Direct3D 12 support (feature level 11_0 or higher)

## Building

- Visual Studio 2022 with the "Desktop development with C++" workload
- Windows SDK (installed with the workload)
- C++ standard: C++14 (MSVC default)

1. Open `DX12Renderer/DX12Renderer.sln`
2. Select the `Release | x64` configuration
3. Build > Build Solution

The executable is written to `DX12Renderer/x64/Release/DX12Renderer.exe`.

## Running

Run from a terminal so the output stays visible:

```
.\DX12Renderer.exe
```

The program takes no command line arguments. Exit code 0 means all DirectX objects were created successfully, 1 means a creation step failed.

Prebuilt executables are available under [Releases](https://github.com/Cvetelinavll/chaos-dx12/releases).

## Example output

```
Intel(R) Iris(R) Xe Graphics
Video memory: 128 MB
Device created on: Intel(R) Iris(R) Xe Graphics
Microsoft Basic Render Driver
Video memory: 0 MB
Command queue, allocator and list created!
```

## Project structure

| File | Purpose |
|---|---|
| `DX12Renderer.cpp` | Entry point, creates the renderer |
| `Renderer.h` / `Renderer.cpp` | DirectX 12 setup: factory, adapter, device and command objects |

## Lecture branches

Each lecture is developed in its own branch, based on the previous one. `main` always contains the latest finished lecture.

| Branch | Release | Content |
|---|---|---|
| `lecture-01` | | Enumerate GPU adapters with DXGI |
| `lecture-02` | v0.2 | Device, command queue, allocator and command list; Renderer class |
