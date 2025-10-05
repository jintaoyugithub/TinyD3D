#include <directx/d3dx12.h>
#include "helloTriangle.hpp"
#include <filesystem>

// TODO: some repeatable logic should move to dx12 backend

void ElemHelloTriangle::onAttach(tinyd3d::Application* app)
{
	auto device = app->getDevice();
	auto swapchain = app->getSwapchain();
	m_cpQueue = app->getQueue(1).queue;
	m_copyFence = app->getMainCopyFence().get();
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, "Hello triangle copy fence event");

	// or I get the device, app info here and pass to the functions
	LoadPipeline(device, swapchain);
	LoadAssets(device);
}

void ElemHelloTriangle::onDetach()
{
	CloseHandle(m_fenceEvent);
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
	cmd->SetPipelineState(m_pso.Get());
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
}

void ElemHelloTriangle::onResize()
{
}

void ElemHelloTriangle::postRender(ID3D12GraphicsCommandList* cmd)
{
}

void ElemHelloTriangle::LoadPipeline(ID3D12Device* device, IDXGISwapChain3* swapchain)
{
	// should be here? or in the application?
	// create cmd allocator
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
	{
		// create root signature
		// we don't need to specify anything here, so an empty root sig is enough
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc{};
		rootSigDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		ComPtr<ID3DBlob> rootSignature;
		ComPtr<ID3DBlob> error;

		/// what's this for? 
		/// this func transform root signature struct in cpu side
		/// to some binary data which gpu can understand
		/// because root signature is part of the shader, need to be compiled
		D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSignature, &error);

		// invalid parameter
		//auto hr = device->CreateRootSignature(0, rootSignature.Get(), rootSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSig));
		auto hr = device->CreateRootSignature(0, rootSignature->GetBufferPointer(), rootSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSig));
	}

	// create pipeline state, including shader compiling
	{
		// TODO: write getpath func in helper, add add_custom_command in cmake
		// copy the shader files to the work dir during the build time
		// then compile the shader there
		std::filesystem::path src = __FILE__;
		auto shaderPath = src.parent_path() / "shaders/transform.hlsl";
		auto hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", m_compileFlags, 0, &m_vs, nullptr);
		hr = D3DCompileFromFile(shaderPath.c_str(), nullptr, nullptr, "PSMain", "ps_5_0", m_compileFlags, 0, &m_ps, nullptr);

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};


		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.pRootSignature = m_rootSig.Get();
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs)};
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.NumRenderTargets = 1; // why 1?
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		/// what are these?
		/// multi sample anti aliasing
		psoDesc.SampleMask = UINT_MAX; // control which sample will be written to the RT
		psoDesc.SampleDesc.Count = 1; // msaa level
		// shaders
		psoDesc.VS = { m_vs->GetBufferPointer(), m_vs->GetBufferSize() };
		psoDesc.PS = { m_ps->GetBufferPointer(), m_ps->GetBufferSize() };

		hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso));
	}

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
		ComPtr<ID3D12Resource> gpuVertexRes;

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

		// create cmd list
		ComPtr<ID3D12GraphicsCommandList> tempCmd;
		//auto hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_cmdAlloc.Get(), m_pso.Get(), IID_PPV_ARGS(&tempCmd));
		auto hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, m_cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&tempCmd));
		
		// copy the data to the gpu
		UpdateSubresources(
			tempCmd.Get(), // use a temp cmd list?
			gpuVertexRes.Get(),
			m_vertexBuffer.Get(),
			0,
			0,
			1,
			&subResData
		);

		tempCmd->Close();

		// upload the data
		ID3D12CommandList* list = { tempCmd.Get() };
		m_cpQueue->ExecuteCommandLists(1, &list);

		// wait for the cmd list to finish execute

		/// updatesubresouces vs. map() and memcpy()?
		/// map() + memcpy() doesn't require intermediate staging (the cpuVertexRes)
		/// can directly copy the data to the upload heap
		/// useful for small or dynamic data

		// init vertex buffer view
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(tinyd3d::Vertex);
		m_vertexBufferView.SizeInBytes = bufferSize;

		auto fence = ++m_copyFence->fenceValue;
		m_cpQueue->Signal(m_copyFence->fence.Get(), fence);

		if (m_copyFence->fence->GetCompletedValue() < fence) {
			m_copyFence->fence->SetEventOnCompletion(fence, m_fenceEvent);
		}
	}
}
