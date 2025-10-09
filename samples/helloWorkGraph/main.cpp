#ifndef UNICODE
#define UNIOCDE
#endif

#include "helloWorkGraph.hpp"
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

int wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PWSTR pCmdLine, int nCmdShow) {
	tinyd3d::ApplicationInfoDesc appInfo{};
	appInfo.windowConfig.title = "Hello Work Graph";
	appInfo.windowConfig.windowInstance = hInstance;
	appInfo.windowConfig.nCmdShow = nCmdShow;

	D3D12_FEATURE_DATA_D3D12_OPTIONS21 Options{};
	// Enum adapter and create device
	ComPtr<IDXGIFactory4> factor;
	ComPtr<IDXGIAdapter1> adapter;
	Verify(CreateDXGIFactory1(IID_PPV_ARGS(&factor)));

	for(auto idx = 0; factor->EnumAdapters1(idx, &adapter) != DXGI_ERROR_NOT_FOUND; ++idx) {
		Verify(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&appInfo.device)));

		// check if the device support work graph
		D3D12_FEATURE_DATA_D3D12_OPTIONS21 options{};
		appInfo.device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS21, &options, sizeof(options));
		if (options.WorkGraphsTier != D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED)
			break;
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