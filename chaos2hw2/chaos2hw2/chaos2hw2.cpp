#include <iostream>
#include <dxgi1_6.h>
#include <d3d12.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

int main()
{
	IDXGIFactory4* dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	if (SUCCEEDED(hr))
	{
		IDXGIAdapter1* adapter = nullptr;
		ID3D12Device* device = nullptr;
		for (UINT adapterIndx = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(adapterIndx, &adapter); adapterIndx++)
		{
			DXGI_ADAPTER_DESC1 desc;
			hr = adapter->GetDesc1(&desc);
			if (FAILED(hr))
			{
				adapter->Release();
				continue;
			}
			std::wcout << desc.Description << "\n";
			std::cout << "Video memory: " << desc.DedicatedVideoMemory / 1024 / 1024 << " MB\n";

			if (device==nullptr && !(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
			{
				hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
				if (SUCCEEDED(hr))
				{
					std::wcout << L"Device created on: " << desc.Description << L"\n";
				}
				else
				{

				}
			}
			adapter->Release();
		}
		if (device==nullptr)
		{
			std::wcout << L"Failed to create D3D12 device\n";
			dxgiFactory->Release();
			return 1;
		}
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		ID3D12CommandQueue* commandQueue = nullptr;
		hr=device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
		if (FAILED(hr))
		{
			std::wcout << L"Failed to create command queue\n";
			device->Release();
			dxgiFactory->Release();
			return 1;
		}

		ID3D12CommandAllocator* commandAllocator = nullptr;
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
		if (FAILED(hr))
		{
			std::wcout << L"Failed to create command allocator\n";
			commandAllocator->Release();
			device->Release();
			dxgiFactory->Release();
			return 1;
		}
		ID3D12GraphicsCommandList* commandList = nullptr;
		hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
		if (FAILED(hr))
		{
			std::wcout << L"Failed to create command list\n";
			commandAllocator->Release();
			commandQueue->Release();
			device->Release();
			dxgiFactory->Release();
			return 1;
		}
		commandList->Release();
		commandAllocator->Release();
		commandQueue->Release();
		device->Release();
		dxgiFactory->Release();
	}
	return 0;
}