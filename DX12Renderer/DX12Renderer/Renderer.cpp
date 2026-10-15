#include "Renderer.h"


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

Renderer::~Renderer()
{
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	adapter->Release();
	factory->Release();
}

bool Renderer::render()
{
	if (!prepareForRendering())
	{
		return false;
	}
	return true;
}

bool Renderer::prepareForRendering()
{
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	if (FAILED(hr))
	{
		std::wcout << L"Failed to create DXGI factory!\n";
		return false;
	}

	if (!createDevice())
	{
		return false;
	}

	if (!createCommandsManagers())
	{
		return false;
	}
	return true;
}

bool Renderer::createDevice()
{
	HRESULT hr = E_FAIL;
	IDXGIAdapter1* current = nullptr;

	for (UINT adapterIndx = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndx, &current); adapterIndx++)
	{
		DXGI_ADAPTER_DESC1 desc;
		hr = current->GetDesc1(&desc);
		if (FAILED(hr))
		{
			current->Release();
			continue;
		}
		std::wcout << desc.Description << "\n";
		std::cout << "Video memory: " << desc.DedicatedVideoMemory / 1024 / 1024 << " MB\n";

		if (device == nullptr && !(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
		{
			hr = D3D12CreateDevice(current, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
			if (SUCCEEDED(hr))
			{
				std::wcout << L"Device created on: " << desc.Description << L"\n";
				adapter = current;
				continue;
			}
			else
			{
				std::wcout << L"Error\n";
				return false;
			}
		}
		current->Release();
	}
	
	return true;
}

bool Renderer::createCommandsManagers()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue));
	if (FAILED(hr))
	{
		std::wcout << L"Failed to create command queue!\n";
		return false;
	}
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	if (FAILED(hr))
	{
		std::wcout << L"Failed to create command allocator!\n";
		return false;
	}

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	if (FAILED(hr))
	{
		std::wcout << L"Failed to create command list!\n";
		return false;
	}
	std::wcout << L"Command queue, allocator and list created!\n";
	return true;
}
