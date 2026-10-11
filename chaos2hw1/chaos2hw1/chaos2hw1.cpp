#include <iostream>
#include <dxgi1_6.h>
#pragma comment(lib, "dxgi.lib")

int main()
{
	IDXGIFactory4* dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory));

	if (SUCCEEDED(hr))
	{
		IDXGIAdapter1* adapter = nullptr;
		for (UINT adapterIndx = 0;  DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(adapterIndx, &adapter); adapterIndx++)
		{
			DXGI_ADAPTER_DESC1 desc;
			hr = adapter->GetDesc1(&desc);
			if (FAILED(hr))
			{
				continue;
			}
			std::wcout << desc.Description << "\n";
			std::cout << "Video memory: " << desc.DedicatedVideoMemory / 1024 / 1024 << " MB\n";

			adapter->Release();
		}
		dxgiFactory->Release();
	}
	return 0;
}