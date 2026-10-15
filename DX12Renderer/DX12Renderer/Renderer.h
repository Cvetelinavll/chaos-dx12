#pragma once
#include <iostream>
#include <dxgi1_4.h>
#include <d3d12.h>

class Renderer 
{
public:
	~Renderer();

	bool render();
private:
    bool prepareForRendering();
    bool createDevice();
    bool createCommandsManagers();

    IDXGIFactory4* factory = nullptr;
    IDXGIAdapter1* adapter = nullptr;
    ID3D12Device* device = nullptr;

    ID3D12CommandQueue* commandQueue = nullptr;
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12GraphicsCommandList* commandList = nullptr;
};