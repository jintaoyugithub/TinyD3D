#include <directx/d3dx12.h>
#include "helloWorkGraph.hpp"
#include <stdexcept>
#include <filesystem>
#include "tiny_gltf.h"

void tinyd3d::HelloWrokGraph::onAttach(tinyd3d::Application* app)
{
	Verify(app->getDevice().As(&m_device));
	m_comQueue = app->getQueue(0).queue;
	m_compFence = app->getMainCompFence();
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, "Hello Work Graph Fence Event");

	loadDependencies();
	constructPipeline();
}

void tinyd3d::HelloWrokGraph::onDetach()
{
}

void tinyd3d::HelloWrokGraph::preRender()
{
}

void tinyd3d::HelloWrokGraph::onRender(ID3D12GraphicsCommandList* cmd)
{
}

void tinyd3d::HelloWrokGraph::onUIRender()
{
}

void tinyd3d::HelloWrokGraph::onResize()
{
}

void tinyd3d::HelloWrokGraph::postRender(ID3D12GraphicsCommandList* cmd)
{
}

void tinyd3d::HelloWrokGraph::loadDependencies()
{
	auto dxilMod = LoadLibrary("dxil.dll");
	if (dxilMod == 0) {
		throw std::runtime_error("Fail to load dxil dll");
	}

	auto dxcMod = LoadLibrary("dxcompiler.dll");
	if (dxcMod == 0) {
		throw std::runtime_error("Fail to load dxcompiler dll");
	}
	m_dxcInstance = (DxcCreateInstanceProc)GetProcAddress(dxcMod, "DxcCreateInstance");
}

void tinyd3d::HelloWrokGraph::constructPipeline()
{
	ComPtr<ID3D12GraphicsCommandList10> tempCmd;
	ComPtr<ID3D12CommandAllocator> compAlloc;
	Verify(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&compAlloc)));
	Verify(m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&tempCmd)));
	//Verify(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, compAlloc.Get(), nullptr, IID_PPV_ARGS(&tempCmd)));

	///
	/// Compile the shader
	/// 
	// create the dxil util lib and compiler
	m_dxcInstance(CLSID_DxcUtils, IID_PPV_ARGS(&m_dxcUtils));
	m_dxcInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&m_dxcompiler));

	// load the shader file from the disk
	std::filesystem::path curFilePath = __FILE__;
	auto shaderPath = curFilePath.parent_path() / "shaders";
	auto helloWorkGraphShader = shaderPath / "helloWorkGraph.hlsl";


	ComPtr<IDxcBlobEncoding> byteCode;
	ComPtr<IDxcIncludeHandler> includeHanlder;
	Verify(m_dxcUtils->LoadFile(helloWorkGraphShader.c_str(), nullptr, &byteCode));
	Verify(m_dxcUtils->CreateDefaultIncludeHandler(&includeHanlder));

	// latest compiler3 use dxbuffer* as the parameter where store the shader data
	DxcBuffer shaderBuffer{};
	shaderBuffer.Ptr = byteCode->GetBufferPointer();
	shaderBuffer.Size = byteCode->GetBufferSize();
	shaderBuffer.Encoding = DXC_CP_ACP;

	// add compile flags, might be the place to specify which node is the entry point
	std::vector<LPCWSTR> flags = {
		// lib_6_9 will cause unsigned compiled shader bytecode
		// lib_6_9 is preview/experimental shader model, don't match the Agility SDK version
		L"-T", L"lib_6_8",
		//L"-enable-16bit-types",
	};

	ComPtr<IDxcResult> compileResult;
	Verify(m_dxcompiler->Compile(
		&shaderBuffer,
		flags.data(),
		flags.size(),
		includeHanlder.Get(),
		IID_PPV_ARGS(&compileResult)
	));

	HRESULT hr;
	compileResult->GetStatus(&hr);

	if (FAILED(hr)) {
		ComPtr<IDxcBlobEncoding> error;
		compileResult->GetErrorBuffer(&error);
		const char* msg = reinterpret_cast<const char*>(error->GetBufferPointer());
		throw std::runtime_error(msg);
	}

	// get the dxil shader
	ComPtr<IDxcBlob> dxilShader;
	Verify(compileResult->GetResult(&dxilShader));


	///
	/// Set up work graph state object
	/// 

	ComPtr<ID3D12RootSignature> globalRS;
	CD3DX12_STATE_OBJECT_DESC soDesc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);

	///
	/// Set up sub objects
	/// 

	// create lib sub object
	CD3DX12_DXIL_LIBRARY_SUBOBJECT* dxilSubObj = soDesc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

	// Set up dxil library sub obj
	ComPtr<ID3DBlob> tempDxilShaderBlob;
	Verify(dxilShader.As(&tempDxilShaderBlob));
	CD3DX12_SHADER_BYTECODE shaderByteCode(tempDxilShaderBlob.Get());
	dxilSubObj->SetDXILLibrary(&shaderByteCode);
	// create global root signature, this require device 14
	// Note: if you don't have a [RootSignature()] definition in the shader
	// this will report error: E_INVALIDARG One or more arguments are invalid.
	Verify(m_device->CreateRootSignatureFromSubobjectInLibrary(
		0,
		shaderByteCode.pShaderBytecode,
		shaderByteCode.BytecodeLength,
		L"globalRS", // this name should be the same in the hlsl code 
		IID_PPV_ARGS(&m_globalRS)
	));

	// Set up work graph sub obj
	ComPtr<ID3D12StateObjectProperties1> SOProp;
	ComPtr<ID3D12WorkGraphProperties> WGProp;
	ComPtr<ID3D12Resource> backMem;
	ComPtr<ID3D12Resource> readBackMem;
	D3D12_WORK_GRAPH_MEMORY_REQUIREMENTS memReqs{};
	D3D12_GPU_VIRTUAL_ADDRESS_RANGE backingMemAddr{};
	D3D12_PROGRAM_IDENTIFIER wgIdentifier{};
	LPCWSTR programName = L"helloWorkGraph";


	// create work graph sub object
	CD3DX12_WORK_GRAPH_SUBOBJECT* wgSubObj = soDesc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
	// how work graph know which know to inlcude?
	wgSubObj->IncludeAllAvailableNodes();
	// entry point name????
	wgSubObj->SetProgramName(programName);

	// Create state object
	Verify(m_device->CreateStateObject(soDesc, IID_PPV_ARGS(&m_workgraphSO)));

	// specify which node is the entry point
	// with the NodeID
	D3D12_NODE_ID entryID{};
	entryID.ArrayIndex = 0;
	entryID.Name = L"rootNode";
	wgSubObj->AddEntrypoint(entryID);
	// what if I have multiple entry point???
	// we could add multiple entry points
	// set the entry number in the D3D12_DISPATCH_GRAPH_DESC

	// how to get the state obj and work graph obj properties?
	Verify(m_workgraphSO.As(&SOProp));
	Verify(m_workgraphSO.As(&WGProp));

	wgIdentifier = SOProp->GetProgramIdentifier(programName);
	// what's work graph index
	auto wgIdx = WGProp->GetWorkGraphIndex(programName);
	WGProp->GetWorkGraphMemoryRequirements(wgIdx, &memReqs);
	backingMemAddr.SizeInBytes = memReqs.MaxSizeInBytes; // choose max for optimal useage
	
	// create the actual back memory here
	CD3DX12_HEAP_PROPERTIES defaultHeap(D3D12_HEAP_TYPE_DEFAULT);

	// no need to update sub resource
	// only need to alloc a mem block for the backing mem
	Verify(m_device->CreateCommittedResource(
		&defaultHeap,
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(backingMemAddr.SizeInBytes),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_backingBuffer)
	));

	backingMemAddr.StartAddress = m_backingBuffer->GetGPUVirtualAddress();

	// create a readback buffer here


	///
	/// Set up the program
	/// 

	// reset the cmd list for cmd recording
	tempCmd->Reset(compAlloc.Get(), nullptr);

	D3D12_SET_PROGRAM_DESC setProg{};
	setProg.Type = D3D12_PROGRAM_TYPE_WORK_GRAPH;
	setProg.WorkGraph.ProgramIdentifier = wgIdentifier;
	setProg.WorkGraph.Flags = D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE;
	setProg.WorkGraph.BackingMemory = backingMemAddr;

	tempCmd->SetProgram(&setProg);

	// get entry node id
	auto rootNodeIdx = WGProp->GetEntrypointIndex(wgIdx, entryID);

	// construct the input record
	struct entryRecord { // need to equivalent to the definition in HLSL code
		UINT gridSize;
		UINT inputTest;
	};

	entryRecord inputData{};
	inputData.gridSize = 8;
	inputData.inputTest = 2;

	// set necessary res for executing
	tempCmd->SetComputeRootSignature(m_globalRS.Get());
	// TODO: UAV buffer here

	// spawn work
	D3D12_DISPATCH_GRAPH_DESC DGDesc{};
	DGDesc.Mode = D3D12_DISPATCH_MODE_NODE_CPU_INPUT; // you can spcify multi node input
	DGDesc.NodeCPUInput.EntrypointIndex = rootNodeIdx;
	DGDesc.NodeCPUInput.NumRecords = 1;
	DGDesc.NodeCPUInput.pRecords = &inputData;
	DGDesc.NodeCPUInput.RecordStrideInBytes = sizeof(entryRecord);
	tempCmd->DispatchGraph(&DGDesc);


	// execute the cmd list
	tempCmd->Close();
	m_comQueue->ExecuteCommandLists(1, CommandListCast(tempCmd.GetAddressOf()));

	// Sync here
	auto curFenceVal = ++m_compFence->fenceValue;
	m_comQueue->Signal(m_compFence->fence.Get(), curFenceVal);

	if (m_compFence->fence->GetCompletedValue() < curFenceVal) {
		m_compFence->fence->SetEventOnCompletion(curFenceVal, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void tinyd3d::HelloWrokGraph::dispatchGraph(ID3D12GraphicsCommandList* cmd)
{
}
