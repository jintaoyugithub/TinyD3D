#pragma once

#include <d3dapp/application.hpp>
#include <dxcapi.h>
#include <initguid.h>

using namespace Microsoft::WRL;

namespace tinyd3d {

/// <summary>
/// This element create a UAV buffer with init value 0
/// Invoke the three nodes in work graph to modified the values
/// and finally read the value back to the CPU
/// </summary>
class HelloWrokGraph : public IAppElement {
public:
    HelloWrokGraph() = default;
    virtual ~HelloWrokGraph() = default;

public:
    void onAttach(tinyd3d::Application* app) override;
    void onDetach() override;
    void preRender() override;
    void onRender(ID3D12GraphicsCommandList* cmd) override;
    void onUIRender() override;
    void onResize() override;
    void postRender(ID3D12GraphicsCommandList* cmd) override;

private:
    void loadDependencies();
    void compileShader();
    void allocBackingMem();
    void constructPipeline();
    void dispatchGraph(ID3D12GraphicsCommandList* cmd);
    void dataReadback();

private:
    ComPtr<ID3D12Device14> m_device;
    // leave the application only responsible for graphics
    ComPtr<ID3D12CommandQueue> m_comQueue;
    ComPtr<ID3D12CommandAllocator> m_comAlloc;

    // work graph
    ComPtr<IDxcCompiler3> m_dxcompiler;
    ComPtr<IDxcUtils> m_dxcUtils;
    ComPtr<ID3D12StateObject> m_workgraphSO;
    ComPtr<ID3D12RootSignature> m_globalRS;
    ComPtr<ID3D12WorkGraphProperties> m_wgProp;
    ComPtr<ID3D12Resource> m_backingBuffer;

    DxcCreateInstanceProc m_dxcInstance;
};
}