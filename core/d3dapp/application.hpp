#pragma once

#include <directx/d3dx12.h>
#include <d3d12.h>
#include <memory>
#include <vector>
#include <assert.h>
// TODO: make it target include
#include <common.hpp>
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

//#include <d3dx12.h>

namespace tinyd3d {
using namespace Microsoft::WRL;

class Application;

//TODO: move to d3dbackend queue.hpp
struct QueueInfo {
    D3D12_COMMAND_QUEUE_DESC desc{};
    ComPtr<ID3D12CommandQueue> queue{ NULL };
};

struct Fence {
    ComPtr<ID3D12Fence> fence;
    inline static uint64_t fenceValue = 0;
};

struct IAppElement {
public:
    virtual void onAttach(Application* app) = 0;
    virtual void onDetach() = 0;
    virtual void preRender() = 0;
    virtual void onRender(ID3D12GraphicsCommandList* cmd) = 0;
    virtual void onUIRender() = 0;
    virtual void onResize() = 0;
    virtual void postRender(ID3D12GraphicsCommandList* cmd) = 0;

    virtual ~IAppElement() = default;
};

struct WindowConfig {
    const char* title;
    int nCmdShow{ 1 };
    WindowInstance windowInstance{ NULL };
    uvec2 windowSize{ 800, 600 };
    ivec2 upperLeftPos{ 0, 0 };
    bool fullScreen{ false };
};

struct ApplicationInfoDesc {
    uint16_t frameCount{ 2 }; //TODO: could be a bug

    WindowConfig windowConfig;

    // TODO: add d3d related variables like device, queue info etc.
    ComPtr<ID3D12Device14> device{ NULL };
    ComPtr<IDXGIAdapter1> adapter{ NULL };
    //ComPtr<IDXGISwapChain3> swapChain{ NULL };
    std::vector<QueueInfo> queues;

    // TODO: move the config to json
    // and construct with d3dcontext class
    D3D_FEATURE_LEVEL featureLevel{D3D_FEATURE_LEVEL_12_0};
    std::vector<D3D12_FEATURE_DATA_D3D12_OPTIONS> Options;

    uint16_t shaderCompileFlags{ 0 };
};

class Application {
public:
    Application() = default;
    virtual ~Application() {assert(m_elements.empty());};

    void init(ApplicationInfoDesc& info);
    void run();
    void close();

public:
    void addElement(const std::shared_ptr<IAppElement> elem);

    // Getters
    inline ApplicationInfoDesc getAppInfo() const { return m_appInfo; };
    inline ComPtr<ID3D12Device> getDevice() const { return m_device; };
    inline ComPtr<IDXGISwapChain3> getSwapchain() const { return m_swapchain; };
    inline QueueInfo getQueue(uint16_t idx) const { return m_queues[idx]; };
    inline ComPtr<ID3D12Resource> getRenderTargets(uint16_t idx) const { return m_renderTargets[idx]; };
    inline std::shared_ptr<Fence> getMainCopyFence() const { return m_copyFence; };

    //inline WindowInstance getWindowInstance() const { return m_appInfoDesc.windowConfig.windowInstance; };
    inline WindowHandler getMainWindow() const { return m_mainWindow; };
    inline uvec2 getWindowSize() const { return m_appInfo.windowConfig.windowSize; };

private:
    void drawFrame(ID3D12GraphicsCommandList* cmd);
    void render2Swapchain(ID3D12GraphicsCommandList* cmd);
    void presentFrame();

    /// <summary>
    /// wait for the gpu to finish
    /// or record the cmd for the next frame
    /// </summary>
    void endFrame(ID3D12GraphicsCommandList* cmd);

private:
    ApplicationInfoDesc m_appInfo;
    WindowHandler m_mainWindow{ NULL };

    ComPtr<IDXGISwapChain3> m_swapchain;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    std::vector<ComPtr<ID3D12Resource>> m_renderTargets;
    uint16_t m_rtvDescriptorSize;
    uint16_t m_curFrameIdx;

    ComPtr<ID3D12CommandAllocator> m_cmdAlloc;

    ComPtr<ID3D12Device> m_device;
    ComPtr<IDXGIAdapter1> m_adapter;
    std::vector<std::shared_ptr<IAppElement>> m_elements;
    std::vector<QueueInfo> m_queues;

    // sync objs
    std::shared_ptr<Fence> m_graphicsFence;
    std::shared_ptr<Fence> m_computeFence;
    std::shared_ptr<Fence> m_copyFence;
    HANDLE m_fenceEvent;

    // TODO: command line arguments
};
}