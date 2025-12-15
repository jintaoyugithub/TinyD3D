#include <directx/d3dx12.h>
#include "application.hpp"
#include "elemwindow.hpp"
#include "elemimgui.hpp"
#include <stdexcept>

namespace tinyd3d {
void Application::init(ApplicationInfoDesc& info)
{
    // init all the members
    m_appInfo = info;
    m_adapter = info.adapter;
    m_device = info.device;
    m_queues = info.queues;
    m_renderTargets.resize(info.frameCount);
    m_graphicsFence = std::make_shared<Fence>();
    m_computeFence = std::make_shared<Fence>();
    m_copyFence = std::make_shared<Fence>();

    // create allocator
    m_device->CreateCommandAllocator(m_queues[0].desc.Type, IID_PPV_ARGS(&m_cmdAlloc));

    // add a window element
    auto elemWindow = std::make_shared<ElemWindow>(info.windowConfig); // this will cause at the end of this func, elemWindow won't exist anymore
    auto elemImgui = std::make_shared<ElemImgui>();

    addElement(elemWindow);

    m_mainWindow = elemWindow->getMainWindow();

    // create swapchain
    {
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
    }

    // need swapchian to init
    addElement(elemImgui);

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
            auto hr = m_swapchain->GetBuffer(idx, IID_PPV_ARGS(&m_renderTargets[idx]));
            m_device->CreateRenderTargetView(m_renderTargets[idx].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
    }

    // create sync objs
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_graphicsFence->fence));
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_computeFence->fence));
    m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_copyFence->fence));
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, "render targets fence event");
    if (!m_fenceEvent) {
        throw std::runtime_error("Fail to create the fenceEvent");
    }
}

void Application::run()
{
    MSG msg = {};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        ImGui_ImplDX12_NewFrame();
	    ImGui_ImplWin32_NewFrame();
	    ImGui::NewFrame();

        // render a frame
        const float clearColor[] = { 0.2f, 0.2f, 0.6f, 1.0f };
        // TODO: put to end frame
        m_curFrameIdx = m_swapchain->GetCurrentBackBufferIndex();

        // TODO: auto cmd = device->createTempCmd();
        // cmd->begin()
        // ...
        // cmd->stop()

        // reset the allocator
        Verify(m_cmdAlloc->Reset());

        ComPtr<ID3D12GraphicsCommandList> tempCmd;
        auto hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&tempCmd));

        // we need to automatically transition front/back buffer state for draw and present
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_curFrameIdx].Get(),
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
        tempCmd->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_curFrameIdx, m_rtvDescriptorSize);
        tempCmd->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
        tempCmd->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        drawFrame(tempCmd.Get());
        // probably will need rtv to perform render 2 target
        // necessary?
        render2Swapchain(tempCmd.Get());

        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_curFrameIdx].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        );
        tempCmd->ResourceBarrier(1, &barrier);

        tempCmd->Close();

        ID3D12CommandList* cmds = { tempCmd.Get() };
        m_queues[0].queue->ExecuteCommandLists(1, &cmds);

        // test
        //presentFrame();
        m_swapchain->Present(1, 0);

        endFrame(tempCmd.Get());
    }

    close();
}

void Application::addElement(std::shared_ptr<IAppElement> elem)
{
    m_elements.push_back(elem);
    elem->onAttach(this);
}

void Application::drawFrame(ID3D12GraphicsCommandList* cmd)
{
    for (auto& elem : m_elements) {
        elem->onUIRender();
    }

    ImGui::Render();

    // should be render in backward
    for (auto itr = m_elements.rbegin(); itr != m_elements.rend(); itr++) {
        (*itr)->onRender(cmd);
    }
}

void Application::render2Swapchain(ID3D12GraphicsCommandList* cmd)
{
}

void Application::presentFrame()
{
}

void Application::renderUI(ID3D12GraphicsCommandList* cmd)
{
    // render ui in backward
}

void Application::endFrame(ID3D12GraphicsCommandList* cmd)
{
    auto curFence = ++m_graphicsFence->fenceValue;
    m_queues[0].queue->Signal(m_graphicsFence->fence.Get(), curFence);

    if (m_graphicsFence->fence->GetCompletedValue() < curFence) {
        m_graphicsFence->fence->SetEventOnCompletion(curFence, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    // TODO: record cmd for the next frame
}

void Application::close()
{
    for (auto& elem : m_elements) {
        elem->onDetach();
    }

    m_elements.clear();

    // TODO: flush the cmd and wait the gpu to finish
    // otherwise will cause swapchain deconstructor fail
    // since you're still using the gpu
}

// TOOD: migrate as a helper function
void Application::dbgEnable()
{
    ComPtr<ID3D12Debug> dbgController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dbgController)))) dbgController->EnableDebugLayer();
}
}