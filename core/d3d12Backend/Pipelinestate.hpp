#pragma once

#include <directx/d3dx12.h>
#include <d3d12.h>
#include <wrl.h>
#include <cassert>

/*
* 1. Root Signature via cmd list or pipeline state object?
*    > They have to match!
*/

/*
* TODO:
* 1. PSO Cache, check Microsoft mini engine pipelinestate.cpp
*/

namespace tinyd3d {
class Shader;
using Microsoft::WRL::ComPtr;

struct GfxShaderSet {
	D3D12_INPUT_LAYOUT_DESC inputLayout;
	D3D12_SHADER_BYTECODE VS;
	D3D12_SHADER_BYTECODE PS;
	D3D12_SHADER_BYTECODE DS;
	D3D12_SHADER_BYTECODE HS;
	D3D12_SHADER_BYTECODE GS;
};

struct CompShaderSet {

};

class GfxPipelineState {
public:
	GfxPipelineState() = default;
	~GfxPipelineState() = default;

public:
	/// Create the pipeline state object with default desc and specified shaders
	void init(const GfxShaderSet& gfxShaderSet, ComPtr<ID3D12RootSignature> rootSig);
	/// Create the pipeline state object with custom desc
	inline void init(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc) { m_defaultDesc = desc; };

	void build(ComPtr<ID3D12Device> device, const wchar_t* name = L"Unnamed gfx pso");

	// Setters, modified the existed default desc for quick creation
	inline void setRootSignature(const ComPtr<ID3D12RootSignature>& rootSig) { 
		m_rootSig = rootSig; 
		m_defaultDesc.pRootSignature = rootSig.Get();
	};
	inline void setRTVFormat(DXGI_FORMAT format, uint32_t index = 0) { m_defaultDesc.RTVFormats[index] = format; };

	// Getters
	inline ComPtr<ID3D12RootSignature> getRootSignature() const{ 
		assert(m_rootSig.Get() != nullptr);
		return m_rootSig; 
	}; 
	inline ComPtr<ID3D12PipelineState> getPipelineState() const { 
		assert(m_gfxPso.Get() != nullptr);
		return m_gfxPso; 
	};

private:
	const wchar_t* m_name;
	ComPtr<ID3D12PipelineState> m_gfxPso;
	ComPtr<ID3D12RootSignature> m_rootSig;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_defaultDesc{};
};

class CompPipelineState {

};

class StateObject {
public:
	void addSubObject();
	void bindShader(const Shader& shader);

private:
	ComPtr<ID3D12StateObject> m_so;
};
}