#pragma once

#include <d3dapp/application.hpp>
#include <d3d12Backend/Queue.hpp>
#include <d3d12Backend/PipelineState.hpp>

using Microsoft::WRL::ComPtr;

class ElemHelloTriangle : public tinyd3d::IAppElement {
public:
    ElemHelloTriangle() = default;
    virtual ~ElemHelloTriangle() = default;

public:
    void onAttach(tinyd3d::Application* app) override;
    void onDetach() override;
    void preRender() override;
    void onRender(ID3D12GraphicsCommandList* cmd) override;
    void onUIRender() override;
    void onResize() override;
    void postRender(ID3D12GraphicsCommandList* cmd) override;

private:
    void LoadPipeline(ID3D12Device* device, IDXGISwapChain3* swapchain);
    void LoadAssets(ID3D12Device* device);

private:
    // Resources
    ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> gpuVertexRes;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Pipeline obj
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;
    tinyd3d::GfxPipelineState m_pso;
    ComPtr<ID3D12RootSignature> m_rootSig;

    // for data copy and mapping
    ComPtr<ID3D12CommandAllocator> m_cmdAlloc;
    tinyd3d::Queue m_cpyQueue;

    // Shaders
    ComPtr<ID3DBlob> m_vs;
    ComPtr<ID3DBlob> m_ps;
};