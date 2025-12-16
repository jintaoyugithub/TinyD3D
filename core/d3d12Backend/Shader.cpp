#include "Shader.hpp"
#include <stdexcept>
#include "../utils/helper.hpp"

std::unordered_map<tinyd3d::ShaderType, D3D12_SHADER_BYTECODE> tinyd3d::DxcCompiler::compile(std::vector<ShaderCompileInfo>& compileInfos)
{

	ComPtr<IDxcResult> result;
	ComPtr<IDxcIncludeHandler> includeHanlder;
	tinyd3d::Verify(m_utils->CreateDefaultIncludeHandler(&includeHanlder));
	std::unordered_map<tinyd3d::ShaderType, D3D12_SHADER_BYTECODE> compiledShader;

	for (auto& compileInfo : compileInfos) {
		ComPtr<ID3DBlob> code;
		ComPtr<IDxcBlobEncoding> byteCode;
		tinyd3d::Verify(m_utils->LoadFile(compileInfo.filepath, nullptr, &byteCode));

		// latest compiler3 use dxbuffer* as the parameter where store the shader data
		DxcBuffer shaderBuffer{};
		shaderBuffer.Ptr = byteCode->GetBufferPointer();
		shaderBuffer.Size = byteCode->GetBufferSize();
		shaderBuffer.Encoding = DXC_CP_ACP;

		tinyd3d::Verify(m_compiler->Compile(
			&shaderBuffer,
			compileInfo.flags.data(),
			compileInfo.flags.size(),
			includeHanlder.Get(),
			IID_PPV_ARGS(&result)
		));

		// Check compile status
		HRESULT hr;
		result->GetStatus(&hr);
		if (SUCCEEDED(hr)) {
			tinyd3d::Verify(result->GetResult((IDxcBlob**)code.GetAddressOf()));
			compiledShader.insert({ compileInfo.type, { code->GetBufferPointer(), code->GetBufferSize() } });
			continue;
		}

		ComPtr<IDxcBlobEncoding> errors;
		result->GetErrorBuffer(&errors);
		auto text = errors->GetBufferPointer();
		if (text) {
			throw std::runtime_error(reinterpret_cast<const char*>(text));
		}
	}

	return compiledShader;
}

tinyd3d::DxcCompiler::DxcCompiler()
{
	// Check dependencies
	auto dxilMod = LoadLibrary("dxil.dll");
	if (dxilMod == 0) {
		throw std::runtime_error("Fail to load dxil dll");
	}

	auto dxcMod = LoadLibrary("dxcompiler.dll");
	if (dxcMod == 0) {
		throw std::runtime_error("Fail to load dxcompiler dll");
	}
	m_dxcInstance = (DxcCreateInstanceProc)GetProcAddress(dxcMod, "DxcCreateInstance");

	// Create dxc utils and compiler
	m_dxcInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_utils));
	m_dxcInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_compiler));
}


bool tinyd3d::DxcCompiler::hotReaload()
{
	return false;
}
