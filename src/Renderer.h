#pragma once
#include <string>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

class Renderer 
{
public:
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
    void copyTextureToReadbackBuffer();
    void executeCommandList();
    void waitForGPU();
    void saveImage(const std::string& fileName);

    void renderFrame();
    void cleanUp();
    void frameEnd();

    void enableDebugLayer();
private:
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    static const UINT width = 800;
    static const UINT height = 600;

    ComPtr<IDXGIFactory4> factory = nullptr;
    ComPtr<IDXGIAdapter1> adapter = nullptr;
    ComPtr<ID3D12Device> device = nullptr;

    ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
    ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

    D3D12_RESOURCE_DESC texDesc = {};
    ComPtr<ID3D12Resource> renderTarget=nullptr;
    ComPtr<ID3D12DescriptorHeap> rtvHeap=nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = {};

    ComPtr<ID3D12Resource> readbackBuffer = nullptr;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint = {};

    ComPtr<ID3D12Fence> fence = nullptr;
    HANDLE fenceEvent = nullptr;
    UINT64 fenceValue = 1;

    UINT64 frameIndex = 0;
};