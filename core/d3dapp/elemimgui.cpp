#include "elemimgui.hpp"

static tinyd3d::SimpleDescriptorHeapAlloctor m_srvHeapAlloc;

void tinyd3d::ElemImgui::onAttach(Application* app)
{
	m_device = app->getDevice();
	m_swapchain = app->getSwapchain();
	m_queue = app->getContext().getGfxQueue();
	m_hInstance = app->getMainWindow();

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	// TODO
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	
	Verify(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap)));

	m_srvHeapAlloc.Create(m_device.Get(), m_srvHeap);

	initImgui();
}

void tinyd3d::ElemImgui::onDetach()
{
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void tinyd3d::ElemImgui::preRender()
{
}

void tinyd3d::ElemImgui::onRender(ID3D12GraphicsCommandList* cmd)
{
	// need to set the descriptor heap
	ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
	// this func you take a descriptor heaps array
	cmd->SetDescriptorHeaps(_countof(heaps), heaps);
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void tinyd3d::ElemImgui::onUIRender()
{
	//ImGui::ShowDemoWindow();
}

void tinyd3d::ElemImgui::onResize()
{
}

void tinyd3d::ElemImgui::postRender(ID3D12GraphicsCommandList* cmd)
{
}

void tinyd3d::ElemImgui::initImgui() {
	IMGUI_CHECKVERSION();
	ImGui_ImplWin32_EnableDpiAwareness();
	float scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));
	RECT rc;
	GetClientRect(m_hInstance, &rc);
	DXGI_SWAP_CHAIN_DESC1 desc;
	m_swapchain->GetDesc1(&desc);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;          IF using Docking Branch
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable render the imgui window outside the main window (docking)

	/// Cursor offset error
	//io.DisplaySize = ImVec2((float)rc.right, (float)rc.bottom);
	//auto fbScale = ImVec2((float)rc.right / desc.Width, (float)rc.bottom / desc.Height);
	//io.DisplayFramebufferScale = fbScale;

	// Setup style
	ImGui::StyleColorsDark();

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(scale);
	style.FontScaleDpi = scale;

	// Set up render backend
	ImGui_ImplWin32_Init(m_hInstance);

	ImGui_ImplDX12_InitInfo info{};

	info.Device = m_device.Get();
	info.CommandQueue = m_queue.getQueue().Get();
	info.NumFramesInFlight = desc.BufferCount;
	info.RTVFormat = desc.Format;

	// Allocating SRV descriptors for texture
	// current version of imgui required a single SRV descriptor passed
	info.SrvDescriptorHeap = m_srvHeap.Get();
	info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {return m_srvHeapAlloc.Alloc(out_cpu_handle, out_gpu_handle); };
	info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return m_srvHeapAlloc.Delete(cpu_handle, gpu_handle); };

	ImGui_ImplDX12_Init(&info);
}
