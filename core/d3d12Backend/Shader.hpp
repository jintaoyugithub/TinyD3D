#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <unordered_map>

namespace tinyd3d {
using Microsoft::WRL::ComPtr;

enum ShaderType {
	VS,
	PS,
	DS,
	HS,
	GS,
};

struct ShaderCompileInfo {
	const wchar_t* filepath;
	ShaderType type;
	std::vector<LPCWSTR> flags;
};

struct IShaderCompiler {
	//virtual std::unordered_map<ShaderType, D3D12_SHADER_BYTECODE> compile(std::vector<ShaderCompileInfo>& compileInfos) = 0;
	virtual std::unordered_map<tinyd3d::ShaderType, Microsoft::WRL::ComPtr<ID3DBlob>> compile(std::vector<ShaderCompileInfo>& compileInfos) = 0;
	virtual bool hotReaload() = 0;
};

class DxcCompiler : public IShaderCompiler {
public:
	~DxcCompiler() = default;

	DxcCompiler(const DxcCompiler& compiler) = delete;
	DxcCompiler operator=(const DxcCompiler& compiler) = delete;

	bool hotReaload() override;
	virtual std::unordered_map<tinyd3d::ShaderType, Microsoft::WRL::ComPtr<ID3DBlob>> compile(std::vector<ShaderCompileInfo>& compileInfos) override;

	inline static DxcCompiler& getInstance() { static DxcCompiler compiler; return compiler; };

private:
	DxcCompiler();

private:
	// Singleton
	static DxcCompiler m_dxcCompiler;

	DxcCreateInstanceProc m_dxcInstance;
	ComPtr<IDxcUtils> m_utils;
	ComPtr<IDxcResult> m_result;
	ComPtr<IDxcCompiler3> m_compiler;
	
};
}
