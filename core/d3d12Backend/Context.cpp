#include "Context.hpp"
#include <stdexcept>
#include <dxgi1_6.h>

void tinyd3d::D3DContext::Init(const ContextInfo& info)
{
	if (info.bEnableDbg) {
		ComPtr<ID3D12Debug> dbgController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dbgController))))
			dbgController->EnableDebugLayer();
		else
			throw std::runtime_error("Fail to enable debug layer!");
	}

	createDevice(info.features);
	createQueues(info.queueDescs);
}

void tinyd3d::D3DContext::createDevice(const std::vector<FeatureInfo>& features)
{
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory4> factor;

}

void tinyd3d::D3DContext::createQueues(const std::vector<D3D12_COMMAND_QUEUE_DESC> queueDescs)
{
}
