#include <directx/d3dx12.h>
#include "helloTriangle.hpp"
#include <filesystem>
#include <imgui.h>
#include <d3d12Backend/Shader.hpp>

// TODO: some repeatable logic should move to dx12 backend

#include "tiny_gltf.h"
using namespace tinygltf;

void loadModel(std::string filename) {
	Model model;
	TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);

	assert(ret);
}

void ElemHelloTriangle::onAttach(tinyd3d::Application* app)
{
	auto device = app->getDevice();
	auto swapchain = app->getSwapchain();
	m_cpyQueue = app->getContext().getCpyQueue();

	// or I get the device, app info here and pass to the functions
	LoadPipeline(device.Get(), swapchain.Get());
	LoadAssets(device.Get());
}

void ElemHelloTriangle::onDetach()
{
}

void ElemHelloTriangle::preRender()
{
	// necessary update for the next frame
	// for example: dynamic data, particle data which need to be 
	// updated every frame, can use upload heap

	// create a member cmd list record the upload resources cmd
	// and batch with the cmd here to execute

	/// which one is better: temp cmd or persistance cmd list and resue every frame???
	/// for dynamic data, you can use gpu visible upload heap, and map the region
	/// so that the cpu can keep write data to it
	/// and use cmdlist->copybufferregion to copy the data to gpu vram

}

void ElemHelloTriangle::onRender(ID3D12GraphicsCommandList* cmd)
{
	cmd->SetPipelineState(m_pso.getPipelineState().Get());
	cmd->SetGraphicsRootSignature(m_rootSig.Get());
	// these two is a must
	cmd->RSSetScissorRects(1, &m_scissorRect);
	cmd->RSSetViewports(1, &m_viewport);
	cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmd->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	cmd->DrawInstanced(3, 1, 0, 0);
}

void ElemHelloTriangle::onUIRender()
{
	static float f = 0.0f;
	ImGui::Begin("Hello Triangle");

	ImGui::Text("This is a test text!");
	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);

	ImGui::End();
}

void ElemHelloTriangle::onResize()
{
}

void ElemHelloTriangle::postRender(ID3D12GraphicsCommandList* cmd)
{
}

void ElemHelloTriangle::LoadPipeline(ID3D12Device* device, IDXGISwapChain3* swapchain)
{
	/// Create the shader
	std::filesystem::path src = __FILE__;
	auto triangleShader = src.parent_path() / "shaders/transform.hlsl";

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	auto& compiler = tinyd3d::DxcCompiler::getInstance();
	std::vector<tinyd3d::ShaderCompileInfo> compileInfos;
	// Dxc does not support shader modle 5.0 and below
	compileInfos.push_back({ triangleShader.c_str(), tinyd3d::ShaderType::VS, std::vector<LPCWSTR>{ L"-T", L"vs_6_0", L"-E", L"VSMain" } });
	compileInfos.push_back({ triangleShader.c_str(), tinyd3d::ShaderType::PS, std::vector<LPCWSTR>{ L"-T", L"ps_6_0", L"-E", L"PSMain" } });
	auto compiledShaders = compiler.compile(compileInfos);

	/// Create root signature
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
	rootSigDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ComPtr<ID3DBlob> rootSignature;
	ComPtr<ID3DBlob> error;
	tinyd3d::Verify(D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignature, &error));
	tinyd3d::Verify(device->CreateRootSignature(0, rootSignature->GetBufferPointer(), rootSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSig)));

	/// Create pipeline state object
	tinyd3d::GfxShaderSet set{};
	auto& vs = compiledShaders[tinyd3d::ShaderType::VS];
	auto& ps = compiledShaders[tinyd3d::ShaderType::PS];
	set.inputLayout = { inputElementDescs, _countof(inputElementDescs) };
	set.VS = { vs->GetBufferPointer(), vs->GetBufferSize() };
	set.PS = { ps->GetBufferPointer(), ps->GetBufferSize() };
	m_pso.init(set, m_rootSig);
	m_pso.build(device, L"Hello triangle pso");

	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(&m_cmdAlloc));

	// viewport and scissor rect set up
	DXGI_SWAP_CHAIN_DESC desc{};
	swapchain->GetDesc(&desc);
	m_viewport.Width = desc.BufferDesc.Width;
	m_viewport.Height = desc.BufferDesc.Height;
	m_scissorRect.right = desc.BufferDesc.Width;
	m_scissorRect.bottom = desc.BufferDesc.Height;
}

void ElemHelloTriangle::LoadAssets(ID3D12Device* device)
{
	// create vertex buffer
	{
		// vertices data
		tinyd3d::Vertex triangle[] = {
			// position (clip space) and color
			{tinyd3d::vec3(-.5f, -.5f, .0f), tinyd3d::vec4(1.0f, 0.0f, 0.0f, 1.0f)},
			{tinyd3d::vec3(.5f, -.5f, .0f), tinyd3d::vec4(0.0f, 1.0f, 0.0f, 1.0f)},
			{tinyd3d::vec3(.0f, .5f, .0f), tinyd3d::vec4(0.0f, 0.0f, 1.0f, 1.0f)},
		};

		// so I need two heap res, one default, one upload
		// write data to upload, and then copy to the default
		// which will be used in the gpu

		/// this is because gpu can access to deafult heap very fast
		/// this method is better for large data like static massive vertices

		CD3DX12_HEAP_PROPERTIES heapDefault(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_HEAP_PROPERTIES heapUpload(D3D12_HEAP_TYPE_UPLOAD);

		// create two temp resource to store the data
		//ComPtr<ID3D12Resource> gpuVertexRes;

		auto bufferSize = sizeof(triangle);

		// heap used for gpu
		device->CreateCommittedResource(
			&heapDefault,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&gpuVertexRes)
		);

		// heap used for cpu
		device->CreateCommittedResource(
			&heapUpload,
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)
		);


		// copy data to vertex buffer since we don't need to read 
		// this triangles data from cpu by using Map() and memcpy()
		D3D12_SUBRESOURCE_DATA subResData{};
		// fill upload heap with vertex data
		subResData.pData = triangle;
		// what are these two?
		subResData.RowPitch = bufferSize; // the size in onw row of your data
		subResData.SlicePitch = subResData.RowPitch; // the size in buytes of one slice, for 3d texture

		/// the existance of m_vertexBuffer is to let cpu to write the data to this buffer
		/// then copy the data from m_vertexBuffer to actual gpu memory
		/// we need two different heap that's because default can only access by GPU
		/// upload can be access by CPU

		//// create cmd list
		//ComPtr<ID3D12GraphicsCommandList> tempCmd;
		////auto hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_cmdAlloc.Get(), m_pso.Get(), IID_PPV_ARGS(&tempCmd));
		//auto hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&tempCmd));
		//
		//// copy the data to the gpu
		//UpdateSubresources(
		//	tempCmd.Get(), // use a temp cmd list?
		//	gpuVertexRes.Get(),
		//	m_vertexBuffer.Get(),
		//	0,
		//	0,
		//	1,
		//	&subResData
		//);

		//tempCmd->Close();

		//// upload the data
		//ID3D12CommandList* list = { tempCmd.Get() };
		//m_cpQueue->ExecuteCommandLists(1, &list);

		// wait for the cmd list to finish execute

		/// updatesubresouces vs. map() and memcpy()?
		/// map() + memcpy() doesn't require intermediate staging (the cpuVertexRes)
		/// can directly copy the data to the upload heap
		/// useful for small or dynamic data
		uint8_t* pVertBegin{nullptr};
		CD3DX12_RANGE readRange(0, 0);
		m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertBegin));
		memcpy(pVertBegin, triangle, sizeof(triangle));
		m_vertexBuffer->Unmap(0, nullptr);

		// init vertex buffer view
		//m_vertexBufferView.BufferLocation = gpuVertexRes->GetGPUVirtualAddress();
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(tinyd3d::Vertex);
		m_vertexBufferView.SizeInBytes = bufferSize;

		auto fenceVal = m_cpyQueue.signal();
		m_cpyQueue.wait(fenceVal);

		///
		/// TEST
		///

		// CPU staging heap
		//ComPtr<ID3D12DescriptorHeap> meshHeap;
		//D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
		//heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		//heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		//heapDesc.NumDescriptors = 100;
		//device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&meshHeap));

		//// Should be a loop, for all mesh in the meshes
		//int countFromAccessors = 3;
		//int strideFromBufferView = 12;
		//int meshId = 2;
		//std::vector<unsigned char> tempData;
		//auto incrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		//ComPtr<ID3D12Resource> meshRes;

		//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		//srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		//srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		//srvDesc.Buffer.NumElements = countFromAccessors;
		//srvDesc.Buffer.StructureByteStride = strideFromBufferView;

		//// Use default heap and upload heap to upload the res...

		//CD3DX12_CPU_DESCRIPTOR_HANDLE handle(meshHeap->GetCPUDescriptorHandleForHeapStart());
		//handle.Offset(meshId, incrementSize);
		//device->CreateShaderResourceView(meshRes.Get(), &srvDesc, handle);
	}
}
