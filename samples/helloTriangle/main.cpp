#ifndef UNICODE
#define UNIOCDE
#endif

#include "helloTriangle.hpp"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{

    tinyd3d::ApplicationInfoDesc appConfig{};
    appConfig.windowConfig.title = "Hello Triangle"; // doesn't matter if there is space in between in window class registeration?
    appConfig.windowConfig.nCmdShow = nCmdShow;
    appConfig.windowConfig.windowInstance = hInstance;
    appConfig.windowConfig.windowSize = tinyd3d::uvec2(800, 600);

#ifdef _DEBUG
	{
		ComPtr<ID3D12Debug> dbgController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dbgController)))) {
			dbgController->EnableDebugLayer();
		}
	}

	// determine shader debug compile flags
	appConfig.shaderCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    appConfig.shaderCompileFlags = 0;
#endif

    // TODO: have a backend context and get all the device, adapter, queues etc. from it
    // TODO: abstract to the d3d12 backend, like appConfig.device = tinyd3d::CreateDevice(vendorName ... )
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	// enum the hardware adapter 
    ComPtr<IDXGIAdapter1> adapter;

    // TODO: appConfig.device = tinyd3d::CreateDevice(...);
    for (uint16_t i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        // create the device if the compaitable device is found
        HRESULT hr = D3D12CreateDevice(adapter.Get(), appConfig.featureLevel, IID_PPV_ARGS(&appConfig.device));

        if (SUCCEEDED(hr)) {
            DXGI_ADAPTER_DESC1 adapterDesc{};
            adapter->GetDesc1(&adapterDesc);
            //std::cout << "Selected adapter: " << *desc.Description; // TODO: put it into imgui log window

            // necessary?
            appConfig.adapter = adapter;
            break;
        }
    }

    // TODO: appConfig.queus = tinyd3d::getQueuesInfo(deivce, ...);
    // graphics queue
    tinyd3d::Queue cgQueue;
    cgQueue.desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cgQueue.desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    appConfig.device->CreateCommandQueue(&cgQueue.desc, IID_PPV_ARGS(&cgQueue.queue));
    appConfig.queues.push_back(cgQueue);

    // copy queue
    tinyd3d::Queue cpQueue;
    cpQueue.desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cpQueue.desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
    appConfig.device->CreateCommandQueue(&cpQueue.desc, IID_PPV_ARGS(&cpQueue.queue));
    appConfig.queues.push_back(cpQueue);

    auto app = std::make_unique<tinyd3d::Application>();
    auto helloTriangle = std::make_shared<ElemHelloTriangle>();

    // TODO: read config from the json file
    app->init(appConfig);
    app->addElement(helloTriangle);

    app->run();

    return 0;
}
