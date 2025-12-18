#ifndef UNICODE
#define UNICODE
#endif

#include "hellpResourcesBinding.hpp"
#include "Windows.h"

int wWinMain(HINSTANCE hInstance, HINSTANCE hInstancePrev, PWSTR pCmdLine, int nCmdShow) {
	///
	/// D3D context
	///
	tinyd3d::ContextInfo ctxConfig{};
	// Add required feature
	D3D12_FEATURE_DATA_ROOT_SIGNATURE rootSigFd{};
	rootSigFd.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
	ctxConfig.features.push_back({
		D3D12_FEATURE_ROOT_SIGNATURE,
		&rootSigFd,
		sizeof(rootSigFd),
		[](const void* data) {
			auto* opt = reinterpret_cast<const D3D12_FEATURE_DATA_ROOT_SIGNATURE*>(data);
			return opt->HighestVersion >= D3D_ROOT_SIGNATURE_VERSION_1_1;
		}
	});
	ctxConfig.addQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
	// Two copy queue one primary and one secondary
	ctxConfig.addQueue(D3D12_COMMAND_LIST_TYPE_COPY);
	ctxConfig.addQueue(D3D12_COMMAND_LIST_TYPE_COPY);

	tinyd3d::D3DContext context;
	context.init(ctxConfig);

	///
	/// Application configuration
	/// 
	tinyd3d::ApplicationInfoDesc appDesc{};
	appDesc.context = context;
	appDesc.windowConfig.title = "Hello Resources Bindings";
	appDesc.windowConfig.windowInstance = hInstance;
	appDesc.windowConfig.nCmdShow = nCmdShow;

	///
	/// Create application and add elements
	/// 
	auto app = std::make_unique<tinyd3d::Application>();
	auto elemResBinding = std::make_shared<ElemHelloResources>();
	app->init(appDesc);
	app->addElement(elemResBinding);
    app->run();

    return 0;
}