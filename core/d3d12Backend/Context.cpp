#include "Context.hpp"
#include "../utils/helper.hpp"
#include <stdexcept>
#include <dxgi1_6.h>
#include <format>

#define GET_NAME(Var) #Var

void tinyd3d::D3DContext::Init(const ContextInfo& info)
{
	m_featureLevel = info.featureLevel;

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

void tinyd3d::D3DContext::createDevice(const std::vector<ExtensionInfo>& extensions)
{
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIFactory4> factor;

	Verify(CreateDXGIFactory1(IID_PPV_ARGS(&factor)));

	for (auto idx = 0; factor->EnumAdapters1(idx, &adapter) != DXGI_ERROR_NOT_FOUND; ++idx) {
		Verify(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)));

		// Check support
		for (auto& extension : extensions) {
			Verify(m_device->CheckFeatureSupport(extension.feature, extension.data, extension.dataSize));

			if (!extension.isSupported(extension.data)) {
				throw std::runtime_error(std::format("Fail to load feature: {}", GET_NAME(extension.feature)));
			}
		}
		break;
	}
}

void tinyd3d::D3DContext::createQueues(const std::vector<D3D12_COMMAND_QUEUE_DESC> queueDescs)
{
	for (auto& desc : queueDescs) {
		//Queue queue(m_device, desc);

		switch (desc.Type)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			m_gfxQueue.emplace_back(m_device, desc);
			break;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			m_compQueue.emplace_back(m_device, desc);
			break;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			m_cpyQueue.emplace_back(m_device, desc);
			break;
		default:
			break;
		}
	}
}
