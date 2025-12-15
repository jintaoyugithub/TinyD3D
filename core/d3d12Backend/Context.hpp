/*
* D3DContext hold the device configuration such as specific features
* and the correspondingcreated device
*/

#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>
#include <cassert>
#include "Queue.hpp"

namespace tinyd3d {
using Microsoft::WRL::ComPtr;

struct FeatureInfo {
	uint32_t featureRange;
	uint32_t featureName;
};

struct ContextInfo {
	std::vector<FeatureInfo> features;
	std::vector<D3D12_COMMAND_QUEUE_DESC> queueDescs;

#ifdef _DEBUG
	bool bEnableDbg = true;
#else
	bool bEnableDbg = false;
#endif
};

class D3DContext {
public:
	D3DContext() = default;
	~D3DContext() { assert(m_device.Get() == nullptr); }

	void Init(const ContextInfo& info);

	/// Getters
	inline ComPtr<ID3D12Device> getDevice() const { return m_device; };
	inline Queue getGfxQueue(uint32_t index = 0) { return m_gfxQueue[index]; };
	inline Queue getCompQueue(uint32_t index = 0) { return m_compQueue[index]; };
	inline Queue getCpyQueue(uint32_t index = 0) { return m_cpyQueue[index]; };

private:
	void createDevice(const std::vector<FeatureInfo>& features);
	void createQueues(const std::vector<D3D12_COMMAND_QUEUE_DESC> queueDescs);

private:
	// For device creation
	ComPtr<ID3D12Device> m_device;

	// For queues creation
	std::vector<Queue> m_gfxQueue;
	std::vector<Queue> m_compQueue;
	std::vector<Queue> m_cpyQueue;
};
}