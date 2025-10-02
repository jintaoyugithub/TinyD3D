#include "application.hpp"
#include "elemwindow.hpp"

namespace tinyd3d {
void Application::init(ApplicationInfoDesc& info)
{
    // init all the members
    m_appInfo = info;
    m_adapter = info.adapter;
    m_device = info.device;
    m_queues = info.queues;
    m_renderTargets.resize(info.frameCount);

    // create allocator
    m_device->CreateCommandAllocator(m_queues[0].desc.Type, IID_PPV_ARGS(&m_cmdAlloc));

    // add a window element
    auto elemWindow = std::make_shared<ElemWindow>(info.windowConfig);
    //auto elemImgui = std::make_shared<ElemWindow>();

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
    factory->CreateSwapChain(
        m_queues[0].queue.Get(),
        &scDesc,
        &swapChain
    );

    // project the swapchain to the version we want
    swapChain.As(&m_swapchain);

    // move the render target and rtv handler here
    // as default render target for the swapchain present
    // and expose the render target to the elements which
    // need to write the data into them
    // create descriptor heap
    {
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
		rtvHeapDesc.NumDescriptors = info.frameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
        // descriptor size is related to the device
        m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // heap handler
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// create view for render target
		for (uint16_t idx = 0; idx < info.frameCount; ++idx) {
            m_swapchain->GetBuffer(idx, IID_PPV_ARGS(&m_renderTargets[idx]));
            m_device->CreateRenderTargetView(m_renderTargets[idx].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
    }
}

void Application::run()
{
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // render a frame
        const float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), 0, m_rtvDescriptorSize);

        ComPtr<ID3D12GraphicsCommandList> tempCmd;
        m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&tempCmd));

        tempCmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        tempCmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        tempCmd->Close();

        // test
        auto hr = m_swapchain->Present(1, 0);

        drawFrame(tempCmd.Get());
        // probably will need rtv to perform render 2 target
        render2swapchain(tempCmd.Get());
        presentFrame();

        // add sync here

        // endFrame();
    }

    close();
}

void Application::addElement(std::shared_ptr<IAppElement> elem)
{
    m_elements.push_back(elem);
    elem->onAttach(this);
}

void Application::drawFrame(ID3D12CommandList* cmd)
{
}

void Application::render2swapchain(ID3D12CommandList* cmd)
{
}

void Application::presentFrame()
{
}

void Application::close()
{
    // bug here
    // if don't clear, elements size will be insance
    m_elements.clear();
}
}