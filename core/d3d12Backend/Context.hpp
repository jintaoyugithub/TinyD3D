/*
* D3DContext hold the device configuration such as specific features
* and the correspondingcreated device
*/

#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <cassert>
#include <functional>
#include "Queue.hpp"

namespace tinyd3d {
using Microsoft::WRL::ComPtr;

struct ExtensionInfo {
	D3D12_FEATURE feature;
	void* data;
	uint32_t dataSize;
	// TODO: better use a general callback, not bool required every time
	std::function<bool(const void*)> isSupported;
};

struct ContextInfo {
	std::vector<ExtensionInfo> features;
	D3D_FEATURE_LEVEL featureLevel;
	std::vector<D3D12_COMMAND_QUEUE_DESC> queueDescs;

#ifdef _DEBUG
	bool bEnableDbg = true;
#else
	bool bEnableDbg = false;
#endif

	void addQueue(
		D3D12_COMMAND_LIST_TYPE type, 
		D3D12_COMMAND_QUEUE_FLAGS flag = D3D12_COMMAND_QUEUE_FLAG_NONE, 
		UINT priority = 0, 
		UINT nodeMask = 0) {

		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Type = type;
		desc.Flags = flag;
		desc.Priority = priority;
		desc.NodeMask = nodeMask;
		queueDescs.push_back(desc);
	}
};

class D3DContext {
public:
	D3DContext() = default;
	//~D3DContext() { assert(m_device.Get() == nullptr); }
	~D3DContext() = default;

	void init(const ContextInfo& info);

	/// Getters
	inline ComPtr<ID3D12Device> getDevice() const { return m_device; };
	inline Queue getGfxQueue(uint32_t index = 0) { return m_gfxQueue[index]; };
	inline Queue getCompQueue(uint32_t index = 0) { return m_compQueue[index]; };
	inline Queue getCpyQueue(uint32_t index = 0) { return m_cpyQueue[index]; };

private:
	void createDevice(const std::vector<ExtensionInfo>& features);
	void createQueues(const std::vector<D3D12_COMMAND_QUEUE_DESC> queueDescs);

private:
	// For device creation
	ComPtr<ID3D12Device> m_device;
	D3D_FEATURE_LEVEL m_featureLevel{ D3D_FEATURE_LEVEL_11_0 };

	// For queues creation
	std::vector<Queue> m_gfxQueue;
	std::vector<Queue> m_compQueue;
	std::vector<Queue> m_cpyQueue;
};
}