#pragma once
#include <string>
#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>

using Microsoft::WRL::ComPtr;

class Renderer 
{
public:
    void prepareForRendering(HWND hwnd);
    void renderFrameWithSwapChain();
    void stopRendering();

    UINT getWidth() const;
    UINT getHeight() const;
    UINT64 getFrameIdx() const;
private:
    void createDevice();
    void createCommandsManagers();
    void createSwapChain(HWND windowHandle);
    void createDescriptorHeapForSwapChain();
    void createRenderTargetViewFromSwapChain();
    void createFence();

    void frameBegin();
    void waitForGPU();
    void frameEnd();

    void enableDebugLayer();
private:
    template <typename T>
    using ComPtr = Microsoft::WRL::ComPtr<T>;

    static const UINT width = 800;
    static const UINT height = 600;
    static const UINT swapChainBuffersCount = 2;

    ComPtr<IDXGIFactory4> factory = nullptr;
    ComPtr<IDXGIAdapter1> adapter = nullptr;
    ComPtr<ID3D12Device> device = nullptr;

    ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
    ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
    ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;

    ComPtr<IDXGISwapChain3> swapChain;
    UINT backBufferIdx = 0;

    ComPtr<ID3D12DescriptorHeap> rtvHeap=nullptr;
    ComPtr<ID3D12Resource> renderTargets[swapChainBuffersCount];
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[swapChainBuffersCount] = {};
    UINT rtvDescriptorSize = 0;

    ComPtr<ID3D12Fence> fence = nullptr;
    Microsoft::WRL::Wrappers::Event fenceEvent;
    UINT64 fenceValue = 1;

    UINT64 frameIndex = 0;
    float rendColor[4] = {0.f,0.f,0.f,1.f};
};