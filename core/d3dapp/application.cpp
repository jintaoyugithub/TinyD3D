#include "elemwindow.hpp"

namespace tinyd3d {
void Application::init(ApplicationInfoDesc& info)
{
    // init all the members
    m_appInfo = info;
    m_adapter = info.adapter;
    m_device = info.device;
    m_queues = info.queues;

    // add a window element
    auto elemWindow = std::make_shared<ElemWindow>(info.windowConfig);
    auto elemImgui = std::make_shared<ElemWindow>();

    addElement(elemWindow);
    //addElement(elemImgui);

    m_mainWindow = elemWindow->getMainWindow();

    // create swapchain
    ComPtr<IDXGISwapChain> swapChain;
    DXGI_SWAP_CHAIN_DESC scDesc{};
    scDesc.BufferCount = m_appInfo.frameCount;
    scDesc.BufferDesc.Width = m_appInfo.windowConfig.windowSize.x;
    scDesc.BufferDesc.Height = m_appInfo.windowConfig.windowSize.y;
    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // ???
    // should be created after the window is created
    scDesc.OutputWindow = m_mainWindow; 
    scDesc.SampleDesc.Count = 1; // ???
    scDesc.Windowed = !m_appInfo.windowConfig.fullScreen;

    // TODO: move this logic to dx12 backend
    // like m_swapchain = tinyd3d::CreateSwapchain(...);
    ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));
    auto hr = factory->CreateSwapChain(
        //cgQueue.queue.Get(),
        m_queues[0].queue.Get(),
        &scDesc,
        &swapChain
    );

    // project the swapchain to the version we want
    swapChain.As(&m_swapchain);
}

void Application::run()
{
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    close();
}

void Application::addElement(std::shared_ptr<IAppElement> elem)
{
    m_elements.push_back(elem);
    elem->onAttach(this);
}

void Application::close()
{
    // bug here
    // if don't clear, elements size will be insance
    m_elements.clear();
}
}