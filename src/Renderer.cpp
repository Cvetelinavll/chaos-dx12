#include "Renderer.h"
#include <iostream>
#include <cassert>
#include "CompiledShaders/ConstColor.hlsl.h"
#include "CompiledShaders/ConstColorVS.hlsl.h"
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
	createVertexBuffer();
	createRootSignature();
	createPipelineState();
	createViewport();
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

void Renderer::createVertexBuffer()
{
	Vertex triangleVertices[] =
	{
		{  0.0f,  0.5f },
		{  0.5f, -0.5f },
		{ -0.5f, -0.5f }
	};

	D3D12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	D3D12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(triangleVertices));

	HRESULT hr = device->CreateCommittedResource(&heapProps,
												D3D12_HEAP_FLAG_NONE,
												&bufferDesc,
												D3D12_RESOURCE_STATE_GENERIC_READ,
												nullptr,
												IID_PPV_ARGS(&vertexBuffer));
	assert(SUCCEEDED(hr));

	void* mappedData = nullptr;
	hr = vertexBuffer->Map(0,nullptr, &mappedData);
	assert(SUCCEEDED(hr));

	memcpy(mappedData, triangleVertices, sizeof(triangleVertices));

	vertexBuffer->Unmap(0, nullptr);

	vertexBufferView = D3D12_VERTEX_BUFFER_VIEW();
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = sizeof(triangleVertices);
}

void Renderer::createRootSignature()
{
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc,
											 D3D_ROOT_SIGNATURE_VERSION_1,
											 &signature,
											 &error
	);
	assert(SUCCEEDED(hr));

	hr = device->CreateRootSignature(0,
									signature->GetBufferPointer(),
									signature->GetBufferSize(),
									IID_PPV_ARGS(&rootSignature)
	);
	assert(SUCCEEDED(hr));
}

void Renderer::createPipelineState()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.PS = {g_const_color, _countof(g_const_color)};
	psoDesc.VS = {g_const_color_vs, _countof(g_const_color_vs)};
	psoDesc.InputLayout = {inputLayout, _countof(inputLayout)};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = FALSE;
	psoDesc.DepthStencilState.StencilEnable = FALSE;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;

	const HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
	assert(SUCCEEDED(hr));
}

void Renderer::createViewport()
{
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = static_cast<LONG>(width);
	scissorRect.bottom = static_cast<LONG>(height);
}

void Renderer::frameBegin()
{
	float angle = static_cast<float>(frameIndex) * 0.01f;
	float cosA = cosf(angle);
	float sinA = sinf(angle);

	Vertex triangleVertices[] =
	{
		{  0.0f * cosA - 0.5f * sinA,   0.0f * sinA + 0.5f * cosA },
		{  0.5f * cosA - -0.5f * sinA,  0.5f * sinA + -0.5f * cosA },
		{ -0.5f * cosA - -0.5f * sinA, -0.5f * sinA + -0.5f * cosA }
	};

	void* pVertexData = nullptr;
	HRESULT hr = vertexBuffer->Map(0, nullptr, &pVertexData);
	assert(SUCCEEDED(hr));
	memcpy(pVertexData, triangleVertices, sizeof(triangleVertices));
	vertexBuffer->Unmap(0, nullptr);

	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));

	hr = commandList->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));

	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Transition.pResource = renderTargets[backBufferIdx].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &barrier);

	commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIdx], FALSE, nullptr);
	commandList->ClearRenderTargetView(rtvHandles[backBufferIdx], greenColor, 0, nullptr);
}

void Renderer::renderFrameWithSwapChain()
{
	frameBegin();

	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), nullptr);

	commandList->SetPipelineState(pipelineState.Get());
	commandList->SetGraphicsRootSignature(rootSignature.Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0,1,&vertexBufferView);

	commandList->RSSetViewports(1,&viewport);
	commandList->RSSetScissorRects(1,&scissorRect);

	commandList->DrawInstanced(3, 1, 0, 0);

	D3D12_RESOURCE_BARRIER barrier{};
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