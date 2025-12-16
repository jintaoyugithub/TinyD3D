#include "Queue.hpp"
#include "../utils/helper.hpp"

tinyd3d::Queue::Queue(ComPtr<ID3D12Device> device, D3D12_COMMAND_QUEUE_DESC& desc)
{
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, "Fence Evnet");
	Verify(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_queue)));
}

void tinyd3d::Queue::executeCmdList(ID3D12GraphicsCommandList* list)
{
}

void tinyd3d::Queue::signal()
{
	auto curFenceVar = ++m_fenceValue;
	m_queue->Signal(m_fence.Get(), curFenceVar);

	if (m_fence->GetCompletedValue() < curFenceVar) {
		m_fence->SetEventOnCompletion(curFenceVar, m_fenceEvent);
		// Better not infinite?
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}
