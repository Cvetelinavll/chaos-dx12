#pragma once
#include <iostream>
#include <fstream>
#include <cassert>
#include <dxgi1_4.h>
#include <d3d12.h>

class Renderer 
{
public:
    Renderer() = default;
    Renderer(const Renderer& other) = delete;
    Renderer& operator=(const Renderer& other) = delete;
	~Renderer();

	void render();
private:
    void prepareForRendering();
    void createDevice();
    void createCommandsManagers();
    void createGPUTexture();
    void createRenderTargetView();
    void createReadbackBuffer();
    void createFence();

    void generateConstColorTexture(const float color[4]);
    void generateImage(const float color[4], const std::string& fileName);
    void resetCommandList();
    void copyTextureToReadbackBuffer();
    void executeCommandList();
    void waitForGPU();
    void saveImage(const std::string& fileName);

private:
    static const UINT width = 800;
    static const UINT height = 600;

    IDXGIFactory4* factory = nullptr;
    IDXGIAdapter1* adapter = nullptr;
    ID3D12Device* device = nullptr;

    ID3D12CommandQueue* commandQueue = nullptr;
    ID3D12CommandAllocator* commandAllocator = nullptr;
    ID3D12GraphicsCommandList* commandList = nullptr;

    D3D12_RESOURCE_DESC texDesc = {};
    ID3D12Resource* renderTarget=nullptr;
    ID3D12DescriptorHeap* rtvHeap=nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};

    ID3D12Resource* readbackBuffer = nullptr;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};

    ID3D12Fence* fence = nullptr;
    HANDLE fenceEvent = nullptr;
    UINT64 fenceValue = 0;
};