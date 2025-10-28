#ifndef UNICODE
#define UNIOCDE
#endif

#include "helloWorkGraph.hpp"
#include <atlcomcli.h>
#include <d3dapp/elemwindow.hpp>
#include <memory>
#include <iostream>
#include <common.hpp>
#include <filesystem>

/// <summary>
/// 1. Feature support check
/// 2. Compile shader
/// 3. load shader library
/// 4. allocate backing memory
/// launch the graph
/// </summary>

#define Verify tinyd3d::Verify

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 618; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; }

int wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PWSTR pCmdLine, int nCmdShow) {

	// This is really important
#ifdef _DEBUG
	{
		ComPtr<ID3D12Debug> dbgController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dbgController)))) {
			dbgController->EnableDebugLayer();
		}
	}
#endif

	tinyd3d::ApplicationInfoDesc appInfo{};
	appInfo.windowConfig.title = "Hello Work Graph";
	appInfo.windowConfig.windowInstance = hInstance;
	appInfo.windowConfig.nCmdShow = nCmdShow;

	// Enum adapter and create device
	ComPtr<IDXGIFactory4> factor;
	ComPtr<IDXGIAdapter1> adapter;
	Verify(CreateDXGIFactory2(0, IID_PPV_ARGS(&factor)));

	bool supported = false;
	for(auto idx = 0; factor->EnumAdapters1(idx, &adapter) != DXGI_ERROR_NOT_FOUND; ++idx) {
		Verify(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&appInfo.device)));

		DXGI_ADAPTER_DESC1 temp;
		adapter->GetDesc1(&temp);

		// check if the device support work graph
		D3D12_FEATURE_DATA_D3D12_OPTIONS21 options{};
		Verify(appInfo.device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &options, sizeof(options)));
		if (options.WorkGraphsTier != D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED) {
			supported = true;
			break;
		}
	}

	if (!supported) {
		throw std::runtime_error("Can find physical device that support work graph");
	}

	// queue info
	tinyd3d::QueueInfo cgQueue{};
	cgQueue.desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	cgQueue.desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	appInfo.device->CreateCommandQueue(&cgQueue.desc, IID_PPV_ARGS(&cgQueue.queue));
	appInfo.queues.push_back(cgQueue);

	auto app = std::make_unique<tinyd3d::Application>();
	auto elemHelloWorkGraph = std::make_shared<tinyd3d::HelloWrokGraph>();
	app->init(appInfo);
	app->addElement(elemHelloWorkGraph);
	app->run();

	return 0;
}