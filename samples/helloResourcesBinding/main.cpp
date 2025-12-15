#ifndef UNICODE
#define UNICODE
#endif

#include "hellpResourcesBinding.hpp"
#include "Windows.h"

int WINAPI main(HINSTANCE hInstance, HINSTANCE hInstancePrev, PWSTR pCmdLine, int nCmdShow) {
	auto app = std::make_unique<tinyd3d::Application>();
#ifdef _DEBUG
	app->dbgEnable();
#endif // _DEBUG

	///
	/// Application configuration
	/// 
	tinyd3d::ApplicationInfoDesc appDesc{};
	appDesc.windowConfig.title = "Hello Resources Bindings";
	appDesc.windowConfig.windowInstance = hInstance;
	appDesc.windowConfig.nCmdShow = nCmdShow;
	appDesc.featureLevel = D3D_FEATURE_LEVEL_11_0;


	///
	/// Check required features support
	/// 


	///
	/// Add elements
	/// 
	auto elemResBinding = std::make_shared<ElemHelloResources>();
	app->init(appDesc);
	app->addElement(elemResBinding);
}