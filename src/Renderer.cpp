#include "Renderer.h"
#include <iostream>
#include <cassert>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

void Renderer::prepareForRendering(HWND hwnd)
{
	enableDebugLayer();
	createDevice();
	createCommandsManagers();
	createSwapChain(hwnd);
	createDescriptorHeapForSwapChain();
	createRenderTargetViewFromSwapChain();
	createFence();
}

void Renderer::createDevice()
{
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	assert(SUCCEEDED(hr));

	for (UINT adapterIndx = 0; SUCCEEDED(factory->EnumAdapters1(adapterIndx, &adapter)); adapterIndx++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
		if(SUCCEEDED(hr))
		{
			std::wcout << L"Device created on: " << desc.Description << L"\n";
			break;
		}
		std::wcout << L"Device creation failed on: " << desc.Description<< L", HRESULT 0x" << std::hex << hr << std::dec << L"\n";
	}
	assert(device != nullptr);
}

void Renderer::createCommandsManagers()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
	assert(SUCCEEDED(hr));

	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	assert(SUCCEEDED(hr));

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(hr));

	hr = commandList->Close();
	assert(SUCCEEDED(hr));
}

void Renderer::createSwapChain(HWND windowHandle)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferCount = swapChainBuffersCount;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT hr = factory->CreateSwapChainForHwnd(commandQueue.Get(), windowHandle, &swapChainDesc, nullptr, nullptr, &swapChain1);

	assert(SUCCEEDED(hr));

	hr = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain));
	assert(SUCCEEDED(hr));
}

void Renderer::createDescriptorHeapForSwapChain()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = swapChainBuffersCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	assert(SUCCEEDED(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap))));

	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void Renderer::createRenderTargetViewFromSwapChain()
{
	for (UINT scBuffIndx = 0; scBuffIndx < 2; scBuffIndx++)
	{
		const HRESULT hr = swapChain->GetBuffer(scBuffIndx, IID_PPV_ARGS(&renderTargets[scBuffIndx]));
		assert(SUCCEEDED(hr));

		rtvHandles[scBuffIndx] = rtvHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandles[scBuffIndx].ptr += scBuffIndx * rtvDescriptorSize;
		device->CreateRenderTargetView(renderTargets[scBuffIndx].Get(), nullptr, rtvHandles[scBuffIndx]);
	}
}

void Renderer::createFence()
{
	const HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	fenceEvent.Attach(CreateEvent(nullptr, FALSE, FALSE,nullptr));
	assert(fenceEvent.IsValid());
}

void Renderer::frameBegin()
{
	float frameCoef = static_cast<float>(frameIndex % 1000) / 1000.0f;

	rendColor[0] = frameCoef;
	rendColor[1] = 1.f - frameCoef;
	rendColor[2] = 0.f;
	rendColor[3] = 1.f;
}

void Renderer::renderFrameWithSwapChain()
{
	frameBegin();

	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), nullptr);

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Transition.pResource = renderTargets[backBufferIdx].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &barrier);

	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIdx], FALSE, nullptr);
	commandList->ClearRenderTargetView(rtvHandles[backBufferIdx], rendColor, 0, nullptr);

	barrier.Transition.pResource = renderTargets[backBufferIdx].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &barrier);

	commandList->Close();

	ID3D12CommandList* ppCommandLists[] = {commandList.Get()};
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	swapChain->Present(0,0);


	waitForGPU();
	frameEnd();
}

void Renderer::waitForGPU()
{
	const HRESULT hr = commandQueue->Signal(fence.Get(), fenceValue);
	assert(SUCCEEDED(hr));

	if (fence->GetCompletedValue() < fenceValue)
	{
		HRESULT hr = fence->SetEventOnCompletion(fenceValue, fenceEvent.Get());
		assert(SUCCEEDED(hr));

		WaitForSingleObject(fenceEvent.Get(), INFINITE);
	}
	++fenceValue;
}

void Renderer::frameEnd()
{
	++frameIndex;
	backBufferIdx = swapChain->GetCurrentBackBufferIndex();
}

void Renderer::enableDebugLayer()
{
#ifdef _DEBUG
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();
	}
	else
	{
		std::wcout << L"D3D12 debug layer is not available\n";
	}
#endif
}

void Renderer::stopRendering()
{
	waitForGPU();
}

UINT Renderer::getWidth() const 
{ 
	return width; 
}

UINT Renderer::getHeight() const 
{ 
	return height; 
}

UINT64 Renderer::getFrameIdx() const 
{ 
	return frameIndex; 
}