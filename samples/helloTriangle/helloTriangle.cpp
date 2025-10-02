#include <directx/d3dx12.h>
#include "helloTriangle.hpp"

// TODO: some repeatable logic should move to dx12 backend

void ElemHelloTriangle::onAttach(tinyd3d::Application* app)
{
	auto device = app->getDevice().Get();
	auto swapchain = app->getSwapchain().Get();

	// create descriptor heaps
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = 2; //bug
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	app->getDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	m_rtvHeapSize = app->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// create frame res, front buffer and back buffer in this case
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (uint16_t idx = 0; idx < 2; ++idx) {
		app->getSwapchain()->GetBuffer(idx, IID_PPV_ARGS(&m_renderTarget[idx]));
		app->getDevice()->CreateRenderTargetView(m_renderTarget[idx].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvHeapSize);
	}

	// or I get the device, app info here and pass to the functions
	LoadPipeline(device, swapchain);
	//LoadAssets(app);
}

void ElemHelloTriangle::onDetach()
{
	// destory the res
}

void ElemHelloTriangle::preRender()
{
	// necessary update for the next frame
}

void ElemHelloTriangle::onRender(ID3D12CommandList* cmd)
{
	// populate cmd list, include reset the allocator, cmd list and record the new cmds
}

void ElemHelloTriangle::onUIRender()
{
}

void ElemHelloTriangle::onResize()
{
}

void ElemHelloTriangle::LoadPipeline(ID3D12Device* device, IDXGISwapChain3* swapchain)
{
	// create descriptor heaps
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = 2; //bug
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
	m_rtvHeapSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// create frame res, front buffer and back buffer in this case
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (uint16_t idx = 0; idx < 2; ++idx) {
		swapchain->GetBuffer(idx, IID_PPV_ARGS(&m_renderTarget[idx]));
		device->CreateRenderTargetView(m_renderTarget[idx].Get(), nullptr, rtvHandle);
		rtvHandle.Offset(1, m_rtvHeapSize);
	}

	// create cmd allocator
	device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAlloc));

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

		device->CreateRootSignature(0, rootSignature.Get(), rootSignature->GetBufferSize(), IID_PPV_ARGS(&m_rootSig));
	}

	// create pipeline state, including shader compiling
	{
		D3DCompileFromFile(L"./shaders/transform.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", m_compileFlags, 0, &m_vs, nullptr);
		D3DCompileFromFile(L"./shaders/transform.hlsl", nullptr, nullptr, "PSMain", "vs_5_0", m_compileFlags, 0, &m_ps, nullptr);

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
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.NumRenderTargets = 1; // why 1?
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		/// what's these?
		/// multi sample anti aliasing
		psoDesc.SampleMask = UINT_MAX; // control which sample will be written to the RT
		psoDesc.SampleDesc.Count = 1; // msaa level
		// shaders
		psoDesc.VS = { m_vs->GetBufferPointer(), m_vs->GetBufferSize() };
		psoDesc.PS = { m_ps->GetBufferPointer(), m_ps->GetBufferSize() };

		device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pso));
	}

	// create cmd list

	// create sync objs
}
