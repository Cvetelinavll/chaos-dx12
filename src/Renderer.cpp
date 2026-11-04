#include "Renderer.h"
#include <iostream>
#include <cassert>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

void Renderer::prepareForRendering()
{
	enableDebugLayer();
	createDevice();
	createCommandsManagers();
	createGPUTexture();
	createRenderTargetView();
	createReadbackBuffer();
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
}

void Renderer::createGPUTexture()
{
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	D3D12_HEAP_PROPERTIES heapProps={};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

	const HRESULT hr = device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(&renderTarget));
	assert(SUCCEEDED(hr));
}

void Renderer::createRenderTargetView()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	const HRESULT hr = device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap));
	assert(SUCCEEDED(hr));

	rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();
	device->CreateRenderTargetView(renderTarget.Get(), nullptr, rtvHandle);
}

void Renderer::generateConstColorTexture(const float color[4])
{
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
}

void Renderer::createReadbackBuffer()
{
	UINT64 bufferSize = 0;
	device->GetCopyableFootprints(&texDesc,0,1,0, &footprint, nullptr, nullptr, &bufferSize);

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = bufferSize;
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_READBACK;

	const HRESULT hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&readbackBuffer));
	assert(SUCCEEDED(hr));
}

void Renderer::createFence()
{
	const HRESULT hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	assert(fenceEvent);
}

void Renderer::copyTextureToReadbackBuffer()
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTarget.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	commandList->ResourceBarrier(1,& barrier);

	D3D12_TEXTURE_COPY_LOCATION source = {};
	source.pResource = renderTarget.Get();
	source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	source.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION destination = {};
	destination.pResource = readbackBuffer.Get();
	destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	destination.PlacedFootprint = footprint;

	commandList->CopyTextureRegion(&destination, 0,0,0,&source, nullptr);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1,&barrier);
}

void Renderer::executeCommandList()
{
	HRESULT hr = commandList->Close();
	assert(SUCCEEDED(hr));

	ID3D12CommandList* lists[] = {commandList.Get()};
	commandQueue->ExecuteCommandLists(_countof(lists), lists);
	
	hr = commandQueue->Signal(fence.Get(), fenceValue);
	assert(SUCCEEDED(hr));
}

void Renderer::waitForGPU()
{
	if (fence->GetCompletedValue() < fenceValue)
	{
		HRESULT hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
		assert(SUCCEEDED(hr));

		WaitForSingleObject(fenceEvent, INFINITE);
	}
	++fenceValue;
}

void Renderer::frameBegin()
{
	float frameCoef = static_cast<float>(frameIndex % 1000) / 1000.0f;

	rendColor[0] = frameCoef;
	rendColor[1] = 1.f - frameCoef;
	rendColor[2] = 0.f;
	rendColor[3] = 1.f;
}

void Renderer::renderFrame()
{
	frameBegin();
	generateConstColorTexture(rendColor);
	copyTextureToReadbackBuffer();
	executeCommandList();
	waitForGPU();
	frameEnd();
}

void Renderer::copyFrame(uint8_t* destination, size_t destinationRowPitch)
{
	void* mapped = nullptr;
	const HRESULT hr = readbackBuffer->Map(0, nullptr, &mapped);
	assert(SUCCEEDED(hr));

	const uint8_t* source = static_cast<const uint8_t*>(mapped) + footprint.Offset;
	const size_t rowSize = static_cast<size_t>(width) * 4;
	for (UINT row = 0; row < height; ++row)
	{
		memcpy(destination + row * destinationRowPitch, source + row * footprint.Footprint.RowPitch, rowSize);
	}

	readbackBuffer->Unmap(0, nullptr);
}

void Renderer::frameEnd()
{
	HRESULT hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));

	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));

	++frameIndex;
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
	const HRESULT hr = commandQueue->Signal(fence.Get(), fenceValue);
	assert(SUCCEEDED(hr));
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