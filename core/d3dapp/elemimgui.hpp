#pragma once

#include <common.hpp>
#include "application.hpp"
#include <imgui.h>
#include <backends/imgui_impl_dx12.h>
#include <backends/imgui_impl_win32.h>

namespace tinyd3d {
    struct SimpleDescriptorHeapAlloctor {
        SimpleDescriptorHeapAlloctor() = default;
        ~SimpleDescriptorHeapAlloctor() = default;

        uint64_t m_heapIncrement;
        std::vector<int> m_freeList;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescStart;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuDescStart;
        ComPtr<ID3D12DescriptorHeap> m_heap{ nullptr };

        void Create(ID3D12Device* device, ComPtr<ID3D12DescriptorHeap> heap) {
            m_heap = heap;
            auto desc = heap->GetDesc();
            cpuDescStart = heap->GetCPUDescriptorHandleForHeapStart();
            gpuDescStart = heap->GetGPUDescriptorHandleForHeapStart();
            m_heapIncrement = device->GetDescriptorHandleIncrementSize(desc.Type);

            // init free list
            m_freeList.reserve((int)desc.NumDescriptors);
            for (int idx = 0; idx < desc.NumDescriptors; ++idx) {
                m_freeList.push_back(idx);
            }
        }

        void Destory() {
            m_heap->Release();
            m_freeList.clear();
        }

        void Alloc(D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle) {
            assert(m_freeList.size()> 0);
            auto idx = m_freeList.back();
            m_freeList.pop_back();
            out_cpu_desc_handle->ptr = cpuDescStart.ptr + (m_heapIncrement * idx);
            out_gpu_desc_handle->ptr = gpuDescStart.ptr + (m_heapIncrement * idx);
        }

        void Delete(D3D12_CPU_DESCRIPTOR_HANDLE out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE out_gpu_desc_handle) {
            int cpu_idx = (int)((out_cpu_desc_handle.ptr - cpuDescStart.ptr) / m_heapIncrement);
            int gpu_idx = (int)((out_gpu_desc_handle.ptr - gpuDescStart.ptr) / m_heapIncrement);
            assert(cpu_idx == gpu_idx);
            m_freeList.push_back(cpu_idx);
        }
    };

    class ElemImgui : public IAppElement {
    public:
        ElemImgui() = default;
        ~ElemImgui() = default;

    public:
        void onAttach(Application* app) override;
        void onDetach() override;
        void preRender() override;
        void onRender(ID3D12GraphicsCommandList* cmd) override;
        void onUIRender() override;
        void onResize() override;
        void postRender(ID3D12GraphicsCommandList* cmd) override;

    private:
        void initImgui();

    private:
        ComPtr<ID3D12Device> m_device;
        QueueInfo m_queue;
        ComPtr<IDXGISwapChain3> m_swapchain;
        WindowHandler m_hInstance;
        
        // for texture in the imgui
        ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    };
}
