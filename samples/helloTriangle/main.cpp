#ifndef UNICODE
#define UNIOCDE
#endif

#include "helloTriangle.hpp"
#include <d3dapp/elemwindow.hpp>
#include <Windows.h>
#include <memory>
#include <iostream>

void test(tinyd3d::Application* app) {
    ComPtr<ID3D12CommandAllocator> alloc;
    app->getDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&alloc));
	ComPtr<ID3D12Device> devices;
	alloc->GetDevice(IID_PPV_ARGS(&devices));
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    tinyd3d::ApplicationInfoDesc appConfig{};
    appConfig.windowConfig.title = "Hello Triangle"; // doesn't matter if there is space in between in window class registeration?
    appConfig.windowConfig.nCmdShow = nCmdShow;
    appConfig.windowConfig.windowInstance = hInstance;
    appConfig.windowConfig.windowSize = tinyd3d::uvec2(800, 600);

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

    // TODO: appConfig.queus = tinyd3d::getQueuesInfo(...);
    tinyd3d::QueueInfo cgQueue;
    cgQueue.desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    cgQueue.desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    appConfig.device->CreateCommandQueue(&cgQueue.desc, IID_PPV_ARGS(&cgQueue.queue));
    appConfig.queues.push_back(cgQueue);

    //auto app = std::make_unique<tinyd3d::Application>();
    auto app = tinyd3d::Application();
    auto helloTriangle = std::make_shared<ElemHelloTriangle>();

    app.init(appConfig);
    app.addElement(helloTriangle);

    {
	// create descriptor heaps
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = 2; //bug
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    ComPtr<ID3D12DescriptorHeap> testHeap;
	auto hr = app.getDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&testHeap));
	auto size = appConfig.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    testHeap->GetCPUDescriptorHandleForHeapStart();

    test(&app);
    }

    app.run();

    return 0;
}
