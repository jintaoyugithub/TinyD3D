#pragma once

#include "../utils/pch.hpp"
#include <memory>
#include <vector>
#include <assert.h>
// TODO: make it target include
#include <wrl.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <d3d12Backend/Queue.hpp>
#include <d3d12Backend/Context.hpp>

//#include <d3dx12.h>

namespace tinyd3d {
using namespace Microsoft::WRL;

class Application;

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
    D3DContext context;
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
    inline D3DContext getContext() const { return m_appInfo.context; };
    inline ComPtr<ID3D12Device> getDevice() const { return m_device; };
    inline ComPtr<IDXGISwapChain3> getSwapchain() const { return m_swapchain; };
    inline ComPtr<ID3D12Resource> getRenderTargets(uint16_t idx) const { return m_renderTargets[idx]; };

    //inline WindowInstance getWindowInstance() const { return m_appInfoDesc.windowConfig.windowInstance; };
    inline WindowHandler getMainWindow() const { return m_mainWindow; };
    inline uvec2 getWindowSize() const { return m_appInfo.windowConfig.windowSize; };

private:
    void drawFrame(ID3D12GraphicsCommandList* cmd);
    void render2Swapchain(ID3D12GraphicsCommandList* cmd);
    void presentFrame();
    void renderUI(ID3D12GraphicsCommandList* cmd);

    /// <summary>
    /// wait for the gpu to finish or record the cmd for the next frame
    /// </summary>
    void endFrame();

private:
    ApplicationInfoDesc m_appInfo;
    WindowHandler m_mainWindow{ NULL };

    ComPtr<IDXGISwapChain3> m_swapchain;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    std::vector<ComPtr<ID3D12Resource>> m_renderTargets;
    uint16_t m_rtvDescriptorSize;
    uint16_t m_curFrameIdx;

    ComPtr<ID3D12Device> m_device;
    Queue m_gfxQueue;
    std::vector<std::shared_ptr<IAppElement>> m_elements;

    // TODO: better abstract with cmd list
    ComPtr<ID3D12CommandAllocator> m_cmdAlloc;
};
}