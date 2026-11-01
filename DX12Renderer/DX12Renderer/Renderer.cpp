#include "Renderer.h"


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

Renderer::~Renderer()
{
	CloseHandle(fenceEvent);
	fence->Release();
	readbackBuffer->Release();
	rtvHeap->Release();
	renderTarget->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	adapter->Release();
	factory->Release();
}

void Renderer::render()
{
	prepareForRendering();
}

void Renderer::prepareForRendering()
{
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

	for (UINT adapterIndx = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndx, &adapter); adapterIndx++)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device))))
		{
			std::wcout << L"Device created on: " << desc.Description << L"\n";
			break;
		}
		adapter->Release();
		adapter = nullptr;
	}
	assert(adapter);
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

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	assert(SUCCEEDED(hr));

	hr = commandList->Close();
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
	device->CreateRenderTargetView(renderTarget, nullptr, rtvHandle);
}

void Renderer::generateConstColorTexture(const float color[4])
{
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
	commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
}

void Renderer::generateImage(const float color[4], const std::string& fileName)
{
	resetCommandList();
	generateConstColorTexture(color);
	copyTextureToReadbackBuffer();
	executeCommandList();
	waitForGPU();
	saveImage(fileName);
	std::cout << "Saved " << fileName << "\n";
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

void Renderer::resetCommandList()
{
	HRESULT hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));

	hr = commandList->Reset(commandAllocator, nullptr);
	assert(SUCCEEDED(hr));
}

void Renderer::copyTextureToReadbackBuffer()
{
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = renderTarget;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	commandList->ResourceBarrier(1,& barrier);

	D3D12_TEXTURE_COPY_LOCATION source = {};
	source.pResource = renderTarget;
	source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	source.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION destination = {};
	destination.pResource = readbackBuffer;
	destination.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	destination.PlacedFootprint = footprint;

	commandList->CopyTextureRegion(&destination, 0,0,0,&source, nullptr);

	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1,&barrier);
}

void Renderer::executeCommandList()
{
	const HRESULT hr = commandList->Close();
	assert(SUCCEEDED(hr));

	ID3D12CommandList* lists[] = {commandList};
	commandQueue->ExecuteCommandLists(_countof(lists), lists);
}

void Renderer::waitForGPU()
{
	++fenceValue;
	HRESULT hr = commandQueue->Signal(fence, fenceValue);
	assert(SUCCEEDED(hr));

	if (fence->GetCompletedValue() < fenceValue)
	{
		hr = fence->SetEventOnCompletion(fenceValue, fenceEvent);
		assert(SUCCEEDED(hr));
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

void Renderer::saveImage(const std::string& fileName)
{
	void* renderTData = nullptr;
	const HRESULT hr = readbackBuffer->Map(0, nullptr, &renderTData);
	assert(SUCCEEDED(hr));

	std::ofstream file(fileName, std::ios::binary);
	assert(file);

	file << "P3\n" << width << " " << height << "\n255\n";

	const unsigned char* bytes = static_cast<const unsigned char*>(renderTData) + footprint.Offset;
	for (UINT row = 0; row < height; row++)
	{
		const unsigned char* rowStart = bytes + row * footprint.Footprint.RowPitch;
		for (UINT column = 0; column < width; column++)
		{
			file.write(reinterpret_cast<const char*>(rowStart+column*4),3);
		}
	}

	D3D12_RANGE nothingWritten = {0,0};
	readbackBuffer->Unmap(0, &nothingWritten);
}
