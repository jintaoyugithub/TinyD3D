#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <Windows.h>
#include <cassert>
#include <cstdint>

namespace tinyd3d {
using Microsoft::WRL::ComPtr;

class Queue {
public:
	Queue() = default;
	Queue(ComPtr<ID3D12Device> device, const D3D12_COMMAND_QUEUE_DESC& desc);
	~Queue() { 
		//assert(m_queue.Get() == nullptr); 
		//CloseHandle(m_fenceEvent);
	}

	void executeCmdList(ID3D12CommandList* list);
	uint64_t signal();
	void wait(uint64_t fenceValue);

	///  Getters
	inline ComPtr<ID3D12CommandQueue> getQueue() const { assert(m_queue.Get() != nullptr); return m_queue; };

private:
	ComPtr<ID3D12CommandQueue> m_queue;
	ComPtr<ID3D12Fence> m_fence;
	uint64_t m_fenceValue{ 0u };
	HANDLE m_fenceEvent{ nullptr };
};
}
