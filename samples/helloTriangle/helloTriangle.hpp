#pragma once

#include <d3dapp/application.hpp>

using namespace Microsoft::WRL;

class ElemHelloTriangle : public tinyd3d::IAppElement {
public:
    ElemHelloTriangle() = default;
    ~ElemHelloTriangle() = default;

public:
    void onAttach(tinyd3d::Application* app) override;
    void onDetach() override;
    void preRender() override;
    void onRender(ID3D12GraphicsCommandList* cmd) override;
    void onUIRender() override;
    void onResize() override;

private:
    void LoadPipeline(ID3D12Device* device, IDXGISwapChain3* swapchain);
    void LoadAssets(ID3D12Device* device);
    void waitForPrevFrame();

private:
    // Resources
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Pipeline obj
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;
    ComPtr<ID3D12CommandAllocator> m_cmdAlloc;
    ComPtr<ID3D12RootSignature> m_rootSig;
    ComPtr<ID3D12PipelineState> m_pso;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;

    // for data copy and mapping
    ComPtr<ID3D12CommandQueue> m_cpQueue;

    // Sync objs
    uint16_t m_frameIndex;
    ComPtr<ID3D12Fence> m_fence;
    uint64_t m_fenceValue{ 0 };
    HANDLE m_fenceEvent; // ???

    // Shaders
    ComPtr<ID3DBlob> m_vs;
    ComPtr<ID3DBlob> m_ps;

    uint16_t m_compileFlags{ 0 };
};