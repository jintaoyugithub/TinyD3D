#include "PipelineState.hpp"
#include "../utils/helper.hpp"

using Microsoft::WRL::ComPtr;

void tinyd3d::GfxPipelineState::init(const GfxShaderSet& gfxShaderSet, ComPtr<ID3D12RootSignature> rootSig)
{
	// init defaut desc
	m_defaultDesc.NodeMask = 1;
	m_defaultDesc.pRootSignature = rootSig.Get();
	m_defaultDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	m_defaultDesc.RasterizerState   = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	m_defaultDesc.RasterizerState.FrontCounterClockwise = true;
	m_defaultDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	m_defaultDesc.NumRenderTargets = 1;
	m_defaultDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // TODO: query from swapchain
	m_defaultDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	m_defaultDesc.SampleMask = UINT_MAX;
	m_defaultDesc.SampleDesc.Count = 1;

	m_defaultDesc.InputLayout = *gfxShaderSet.inputLayout;
	m_defaultDesc.VS = gfxShaderSet.VS;
	m_defaultDesc.PS = gfxShaderSet.PS;
	m_defaultDesc.DS = gfxShaderSet.DS;
	m_defaultDesc.HS = gfxShaderSet.HS;
	m_defaultDesc.GS = gfxShaderSet.GS;
}

void tinyd3d::GfxPipelineState::build(ComPtr<ID3D12Device> device, const wchar_t* name)
{
	Verify(device->CreateGraphicsPipelineState(&m_defaultDesc, IID_PPV_ARGS(&m_gfxPso)));
	m_gfxPso->SetName(name);
}
