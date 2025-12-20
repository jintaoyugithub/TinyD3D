#ifndef UNICODE
#define UNIOCDE
#endif

#include "helloWorkGraph.hpp"
#include <atlcomcli.h>
#include <d3dapp/elemwindow.hpp>
#include <memory>
#include <iostream>
#include <utils/pch.hpp>
#include <filesystem>

#include <d3d12Backend/Context.hpp>

/// <summary>
/// 1. Feature support check
/// 2. Compile shader
/// 3. load shader library
/// 4. allocate backing memory
/// launch the graph
/// </summary>

#define Verify tinyd3d::Verify

extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 618; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = u8".\\D3D12\\"; } // this might require copy the all .dlls from downloaded NuGet packages to the specified path

int wWinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PWSTR pCmdLine, int nCmdShow) {

	/// Application Context
	tinyd3d::ContextInfo ctxConfig;
	// Required features
	D3D12_FEATURE_DATA_D3D12_OPTIONS21 options21{};
	ctxConfig.features.emplace_back(
		D3D12_FEATURE_D3D12_OPTIONS21,
		&options21,
		sizeof(options21),
		[](const void* data) {
			auto* opt = static_cast<const D3D12_FEATURE_DATA_D3D12_OPTIONS21*>(data);
			return opt->WorkGraphsTier != D3D12_WORK_GRAPHS_TIER_NOT_SUPPORTED;
		}
	);
	D3D12_FEATURE_DATA_ROOT_SIGNATURE rootSigData{};
	ctxConfig.features.emplace_back(
		D3D12_FEATURE_ROOT_SIGNATURE,
		&rootSigData,
		sizeof(rootSigData),
		[](const void* data) {
			auto* opt = static_cast<const D3D12_FEATURE_DATA_ROOT_SIGNATURE*>(data);
			return opt->HighestVersion != D3D_ROOT_SIGNATURE_VERSION_1_1;
		}
	);
	// Add a gfx queue
	ctxConfig.queueDescs.emplace_back(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		D3D12_COMMAND_QUEUE_FLAG_NONE
	);
	// Add a copy queue
	ctxConfig.queueDescs.emplace_back(
		D3D12_COMMAND_LIST_TYPE_COPY,
		D3D12_COMMAND_QUEUE_FLAG_NONE
	);

	tinyd3d::D3DContext context;
	context.init(ctxConfig);

	tinyd3d::ApplicationInfoDesc appInfo{};
	appInfo.context = context;
	appInfo.windowConfig.title = "Hello Work Graph";
	appInfo.windowConfig.windowInstance = hInstance;
	appInfo.windowConfig.nCmdShow = nCmdShow;

	auto app = std::make_unique<tinyd3d::Application>();
	auto elemHelloWorkGraph = std::make_shared<tinyd3d::HelloWrokGraph>();
	app->init(appInfo);
	app->addElement(elemHelloWorkGraph);
	app->run();

	return 0;
}