#include <directx/d3dx12.h>
#include "hellpResourcesBinding.hpp"
#include <utils/helper.hpp>
#include <stdexcept>
#include <filesystem>
#include <d3d12Backend/Context.hpp>
#include <d3d12Backend/Shader.hpp>

void ElemHelloResources::onAttach(tinyd3d::Application* app)
{
	m_device       = app->getDevice();
	m_gfxQueue     = app->getContext().getGfxQueue();
	m_mainCpyQueue = app->getContext().getCpyQueue();
	m_sedCpyQueue  = app->getContext().getCpyQueue(1);

	///
	/// Init
	///
	tinyd3d::Verify(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_cpyAlloc)));
	tinyd3d::Verify(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_cpyAlloc.Get(), nullptr, IID_PPV_ARGS(&m_cpyCmdList)));

	/// Descriptor heaps
	m_csuHeapIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	m_samplerHeapIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	// create descriptor heaps with fix default size
	D3D12_DESCRIPTOR_HEAP_DESC csuHeapDesc{};
	csuHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	csuHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	csuHeapDesc.NumDescriptors = m_defaultHeapSize;
	csuHeapDesc.NodeMask = 0;
	tinyd3d::Verify(m_device->CreateDescriptorHeap(&csuHeapDesc, IID_PPV_ARGS(&m_csuHeap)));

	m_csuBaseCpuHandle = m_csuHeap->GetCPUDescriptorHandleForHeapStart();

	// TODO: sampler heap

	D3D12_DESCRIPTOR_HEAP_DESC csuSVHeapDesc{};
	csuSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	csuSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	csuSVHeapDesc.NumDescriptors = m_defaultHeapSize;
	csuSVHeapDesc.NodeMask = 0;
	tinyd3d::Verify(m_device->CreateDescriptorHeap(&csuSVHeapDesc, IID_PPV_ARGS(&m_csuHeapShaderVisible)));

	m_csuSVBaseCpuHandle = m_csuHeapShaderVisible->GetCPUDescriptorHandleForHeapStart();
	m_csuSVBaseGpuHandle = m_csuHeapShaderVisible->GetGPUDescriptorHandleForHeapStart();
}

void ElemHelloResources::onDetach()
{
}

void ElemHelloResources::preRender()
{
}

void ElemHelloResources::onRender(ID3D12GraphicsCommandList* cmd)
{
}

void ElemHelloResources::onUIRender()
{
}

void ElemHelloResources::onResize()
{
}

void ElemHelloResources::postRender(ID3D12GraphicsCommandList* cmd)
{
}

void ElemHelloResources::createVertIdxBuffers()
{
	struct Vertex {
		tinyd3d::vec3 pos;
		tinyd3d::vec4 color;
	};

	D3D12_INPUT_ELEMENT_DESC vertInputDesc[]{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};

	m_gfxShaderSet.push_back({ vertInputDesc });

	/// Vertex data
	Vertex quad[] = {
		// position (clip space) and color
		{tinyd3d::vec3(-.5f, -.5f, .0f), tinyd3d::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
		{tinyd3d::vec3( .5f, -.5f, .0f), tinyd3d::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
		{tinyd3d::vec3( .5f,  .5f, .0f), tinyd3d::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
		{tinyd3d::vec3(-.5f,  .5f, .0f), tinyd3d::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
	};

	D3D12_RESOURCE_DESC         vertDesc { };
	vertDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertDesc.Width              = sizeof(quad);
	vertDesc.Height             = 1;
	vertDesc.Alignment          = 0;
	vertDesc.DepthOrArraySize   = 1;
	vertDesc.MipLevels          = 1;
	vertDesc.Format             = DXGI_FORMAT_UNKNOWN;
	vertDesc.SampleDesc.Count   = 1;  // non-msaa
	vertDesc.SampleDesc.Quality = 0; 
	vertDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // buffer is always row major
	vertDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

	/// Index data
	uint16_t quadIndices[] {
		0, 1, 2,
		2, 3, 0,
	};

	D3D12_RESOURCE_DESC          indexDesc { };
	indexDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
	indexDesc.Width              = sizeof(quadIndices);
	indexDesc.Height             = 1;
	indexDesc.Alignment          = 0;
	indexDesc.DepthOrArraySize   = 1;
	indexDesc.MipLevels          = 1;
	//indexDesc.Format = DXGI_FORMAT_R16_UINT;
	indexDesc.Format             = DXGI_FORMAT_UNKNOWN; // buffer must be unknown
	indexDesc.SampleDesc.Count   = 1;  // non-msaa
	indexDesc.SampleDesc.Quality = 0;
	indexDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR; // buffer is always row major
	indexDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

	if (m_bUsePlacedResource) {
		// TODO
	}
	else {
		ComPtr<ID3D12Resource> vertResCpuHanle;
		ComPtr<ID3D12Resource> indexResCpuuHanle;

		m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&vertDesc,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			nullptr,
			IID_PPV_ARGS(&vertResCpuHanle)
		);

		m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&indexDesc,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			nullptr,
			IID_PPV_ARGS(&indexResCpuuHanle)
		);

		if (m_bUseDefaultHeap) {
			m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&vertDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_vertBuffer)
			);

			m_device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&indexDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_indexBuffer)
			);

			D3D12_SUBRESOURCE_DATA vertData{};
			vertData.pData = quad;
			vertData.RowPitch = sizeof(quad); // 1D
			vertData.SlicePitch = sizeof(quad); // 2D, but the buffer here is only 1D

			UpdateSubresources(
				m_cpyCmdList.Get(),
				m_vertBuffer.Get(),
				vertResCpuHanle.Get(),
				0,
				0,
				1,
				&vertData
			);

			// Transit resource state after update
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_vertBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			);

			m_cpyCmdList->ResourceBarrier(1, &barrier);

			D3D12_SUBRESOURCE_DATA indexData{};
			indexData.pData = quadIndices;
			indexData.RowPitch = sizeof(quadIndices);
			indexData.SlicePitch = sizeof(quadIndices);

			UpdateSubresources(
				m_cpyCmdList.Get(),
				m_indexBuffer.Get(),
				indexResCpuuHanle.Get(),
				0,
				0,
				1,
				&vertData
			);

			barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				m_indexBuffer.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST,
				D3D12_RESOURCE_STATE_INDEX_BUFFER
			);

			m_cpyCmdList->ResourceBarrier(1, &barrier);

			// batch the upload together?
			// execute the cmd list here
		}
		else {
			// follow regular Map -> memcpy
			uint8_t* pVertDataBegin{ nullptr };
			CD3DX12_RANGE readRange(0, 0); // we only write, so no read range
			// committed resource create a implicit heap in the sys ram and return
			// a d3d resource handle
			// Map function map the begin address of this implicit heap to a pointer we specify
			// so that we can use that pointer to copy the data
			// in this case, GPU will read the data vai the PCIe
			tinyd3d::Verify(m_vertBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertDataBegin)));
			memcpy(pVertDataBegin, quad, sizeof(quad));
			m_vertBuffer->Unmap(0, nullptr);
		}

		m_vertBufferView.BufferLocation = m_vertBuffer->GetGPUVirtualAddress();
		m_vertBufferView.SizeInBytes = sizeof(quad);
		m_vertBufferView.StrideInBytes = sizeof(Vertex);

		m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
		m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		m_indexBufferView.SizeInBytes = sizeof(quadIndices);
	}
}

void ElemHelloResources::createConstantBuffers()
{
	struct LightConstants {
		tinyd3d::vec4 color;
	};

	LightConstants lights[] = {
		{tinyd3d::vec4(1.0, 1.0, 1.0, 1.0)}, // white
		{tinyd3d::vec4(1.0, 0.8, 0.5, 1.0)}, // light yellow
	};

	// align the constant buffer with 256 bytes
	uint16_t cbSize = (sizeof(lights) + 255) & ~255;
	
	m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(cbSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_lightsColorCB)
	);

	// keep this constant buffer in the upload heap
	uint8_t* pLightColorBegin{ nullptr };
	CD3DX12_RANGE readRange(0, 0);
	tinyd3d::Verify(m_lightsColorCB->Map(0, &readRange, reinterpret_cast<void**>(&pLightColorBegin)));
	memcpy(pLightColorBegin, lights, sizeof(lights));
	m_lightsColorCB->Unmap(0, nullptr);


	D3D12_CONSTANT_BUFFER_VIEW_DESC lightCBVDesc{};
	lightCBVDesc.BufferLocation = m_lightsColorCB->GetGPUVirtualAddress();
	lightCBVDesc.SizeInBytes = cbSize;

	D3D12_CPU_DESCRIPTOR_HANDLE curHeapCpuHandle{};
	curHeapCpuHandle.ptr = m_csuBaseCpuHandle.ptr + (m_csuCpuHandleOffset * m_csuHeapIncrementSize);
	m_device->CreateConstantBufferView(&lightCBVDesc, curHeapCpuHandle);
	m_csuCpuHandleOffset++;
}

void ElemHelloResources::createTextures(std::vector<const wchar_t*> filenames)
{
	auto resPath = std::filesystem::current_path() / "res";
	auto numTexs = filenames.size();

	for (const auto& file : filenames) {
		auto texFullPath = resPath / file;

		if (!std::filesystem::exists(texFullPath)) {
			throw std::runtime_error("Texture file does not exist!");
		}

		// Load texture via DirectXTex
		using namespace DirectX; 
		ScratchImage image;
		auto hr = DirectX::LoadFromWICFile(texFullPath.c_str(), WIC_FLAGS_NONE, nullptr, image);
		if (FAILED(hr)) {
			throw std::runtime_error("Fail to load image");
		}

		auto& metadata = image.GetMetadata();
		const auto* rawImage = image.GetImages();

		/// Upload the texutre and create a coressponding descriptor
		D3D12_RESOURCE_DESC texDesc{};
		switch (metadata.dimension)
		{
		case TEX_DIMENSION_TEXTURE1D:
			texDesc = CD3DX12_RESOURCE_DESC::Tex1D(
				metadata.format,
				static_cast<uint64_t>(metadata.width),
				static_cast<uint64_t>(metadata.arraySize));
			break;
		case TEX_DIMENSION_TEXTURE2D:
			texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
				metadata.format,
				static_cast<uint64_t>(metadata.width),
				static_cast<uint64_t>(metadata.height),
				static_cast<uint64_t>(metadata.arraySize));
			break;
		case TEX_DIMENSION_TEXTURE3D:
			texDesc = CD3DX12_RESOURCE_DESC::Tex3D(
				metadata.format,
				static_cast<uint64_t>(metadata.width),
				static_cast<uint64_t>(metadata.height),
				static_cast<uint64_t>(metadata.depth),
				static_cast<uint64_t>(metadata.arraySize));
			break;
		default:
			break;
		}

		// Texture must be upload to default heap
		ComPtr<ID3D12Resource> texCpuTempHandle;
		tinyd3d::Verify(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_SOURCE,
			nullptr,
			IID_PPV_ARGS(&texCpuTempHandle)
		));

		tinyd3d::Verify(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&texDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&m_checkboardTex)
		));

		// subresource data amount should align with texture count like mip map or what
		std::vector<D3D12_SUBRESOURCE_DATA> imageSubData(image.GetImageCount());

		// create desc for each sub resource
		for (auto idx = 0; idx < image.GetImageCount(); idx++) {
			imageSubData[idx].pData = rawImage[idx].pixels;
			imageSubData[idx].RowPitch = rawImage[idx].rowPitch;
			imageSubData[idx].SlicePitch = rawImage[idx].slicePitch;
		}

		UpdateSubresources(
			m_cpyCmdList.Get(), 
			m_checkboardTex.Get(), 
			texCpuTempHandle.Get(), 
			0, 
			0, 
			image.GetImageCount(), 
			imageSubData.data()
		);

		// Add a barrier
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			m_checkboardTex.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			//D3D12_RESOURCE_STATE_GENERIC_READ
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		);

		m_cpyCmdList->ResourceBarrier(1, &barrier);

		// Create a descriptor
		auto resDesc = texCpuTempHandle->GetDesc();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = resDesc.Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		// TODO: move this to the switch case so that we can have a little bit more flixibility
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

		// TODO: Better check if the size of the current descriptor heap is big enough 
		
		// Add and update descriptor heap
		D3D12_CPU_DESCRIPTOR_HANDLE curHandle{};
		curHandle.ptr = m_csuBaseCpuHandle.ptr + (m_csuCpuHandleOffset * m_csuHeapIncrementSize);
		m_device->CreateShaderResourceView(m_checkboardTex.Get(), &srvDesc, curHandle);
		m_csuCpuHandleOffset++;

		image.Release();
	}
}

void ElemHelloResources::createUAVBuffers()
{
	// Test particle structure
	struct GPUParticle {
		// actual data is randomly generated by the gpu
		tinyd3d::vec4 pos; 
		uint16_t state;
	};

	constexpr uint64_t texWidth = 500;
	constexpr uint64_t texHeight = 500;
	constexpr uint32_t particleCount = 10000;
	constexpr uint32_t bufferSize = particleCount * sizeof(GPUParticle);

	// Create d3d resource
	tinyd3d::Verify(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
		D3D12_RESOURCE_STATE_COMMON, // might not be common
		nullptr,
		IID_PPV_ARGS(&m_randomRWBuffer)
	));

	tinyd3d::Verify(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_R16_FLOAT,
			texWidth,
			1
		),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_randomRWTex)
	));

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavTexDesc{};
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavBufferDesc{};

	uavBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavBufferDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavBufferDesc.Buffer.FirstElement = 0;
	uavBufferDesc.Buffer.NumElements = particleCount;
	uavBufferDesc.Buffer.StructureByteStride = sizeof(GPUParticle);
	uavBufferDesc.Buffer.CounterOffsetInBytes = 0;
	uavBufferDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	// Add and update descriptor heap
	D3D12_CPU_DESCRIPTOR_HANDLE curHandle{};
	curHandle.ptr = m_csuBaseCpuHandle.ptr + (m_csuCpuHandleOffset * m_csuHeapIncrementSize);
	m_device->CreateUnorderedAccessView(m_randomRWBuffer.Get(), nullptr, &uavBufferDesc, curHandle);
	m_csuCpuHandleOffset++;

	curHandle.ptr = m_csuBaseCpuHandle.ptr + (m_csuCpuHandleOffset * m_csuHeapIncrementSize);
	m_device->CreateUnorderedAccessView(m_randomRWTex.Get(), nullptr, &uavTexDesc, curHandle);
	m_csuCpuHandleOffset++;
}

void ElemHelloResources::createGfxPso()
{
	using tinyd3d::ShaderType;

	/// Compile the shaders
	std::filesystem::path shaderPath = std::filesystem::current_path() / "shaders"; // or should I use __FILE__
	auto quadShader = shaderPath / "quad.hlsl";

	auto& compiler = tinyd3d::DxcCompiler::getInstance();
	std::vector<tinyd3d::ShaderCompileInfo> compileInfos;
	compileInfos.push_back({ quadShader.c_str(), tinyd3d::ShaderType::VS, std::vector<LPCWSTR>{ L"-T", L"-vs_6_8", L"-E", L"VSMain" } });
	compileInfos.push_back({ quadShader.c_str(), tinyd3d::ShaderType::PS, std::vector<LPCWSTR>{ L"-T", L"-ps_6_8", L"-E", L"PSMain" } });
	auto codes = compiler.compile(compileInfos);
	// Add the compiled shader to the shader set
	m_gfxShaderSet[0].VS = { codes[ShaderType::VS]->GetBufferPointer(), codes[ShaderType::VS]->GetBufferSize() };
	m_gfxShaderSet[0].PS = { codes[ShaderType::PS]->GetBufferPointer(), codes[ShaderType::PS]->GetBufferSize() };

	// TODO: root signature
	CD3DX12_ROOT_PARAMETER1 rootParameters[5]{};
	//rootParameters[0].InitAsConstants();
	//rootParameters[1].InitAsConstantBufferView();
	//rootParameters[3].InitAsShaderResourceView();
	//rootParameters[4].InitAsUnorderedAccessView();
	//rootParameters[2].InitAsDescriptorTable();
	CD3DX12_STATIC_SAMPLER_DESC staticSampler(0);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc{};
	//rootSigDesc.Init_1_1();

	ComPtr<ID3D12RootSignature> rootSig;
	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	//tinyd3d::Verify(D3DX12SerializeVersionedRootSignature());
	//tinyd3d::Verify(m_device->CreateRootSignature());

	// Bug? rootSig might be destory before pso creation?
	m_gfxPso.init(m_gfxShaderSet[0], rootSig);
	m_gfxPso.build(m_device, L"Hello resource binding gfx pso");
}

void ElemHelloResources::createCompPso()
{
}
