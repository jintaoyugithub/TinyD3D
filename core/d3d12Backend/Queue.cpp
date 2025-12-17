#include "Queue.hpp"
#include "../utils/helper.hpp"

tinyd3d::Queue::Queue(ComPtr<ID3D12Device> device, const D3D12_COMMAND_QUEUE_DESC& desc)
{
	Verify(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	Verify(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_queue)));
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, "Fence Evnet");
}

void tinyd3d::Queue::executeCmdList(ID3D12CommandList* list)
{
	m_queue->ExecuteCommandLists(1, &list);
}

uint64_t tinyd3d::Queue::signal()
{
	auto curFenceVar = ++m_fenceValue;
	m_queue->Signal(m_fence.Get(), curFenceVar);
	return curFenceVar;
}

void tinyd3d::Queue::wait(uint64_t fenceValue)
{
	if (m_fence->GetCompletedValue() < fenceValue) {
		m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		// Better not infinite?
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}
