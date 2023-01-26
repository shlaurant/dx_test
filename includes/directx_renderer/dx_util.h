#pragma once

#include "debug.h"

namespace directx_renderer {
    template<typename T>
    Microsoft::WRL::ComPtr<ID3D12Resource> create_const_buffer(int elem_cnt,
                                                               Microsoft::WRL::ComPtr<ID3D12Device> device) {
        return create_upload_buffer(elem_cnt, size_of_256<T>(), device);
    }

    template<typename T>
    void
    update_const_buffer(Microsoft::WRL::ComPtr<ID3D12Resource> buffer,
                        const T *data,
                        int index) {
        update_upload_buffer<T>(buffer, data, index, size_of_256<T>());
    }

    Microsoft::WRL::ComPtr<ID3D12Resource>
    create_upload_buffer(int elem_cnt, UINT64 elem_size,
                         Microsoft::WRL::ComPtr<ID3D12Device> device);

    template<typename T>
    void update_upload_buffer(Microsoft::WRL::ComPtr<ID3D12Resource> buffer,
                              const T *data, int index, UINT64 elem_size) {
        BYTE *mapped_data;
        ThrowIfFailed(buffer->Map(0, nullptr,
                                  reinterpret_cast<void **>(&mapped_data)));
        memcpy(&mapped_data[elem_size * index], data, sizeof(T));
        buffer->Unmap(0, nullptr);
        mapped_data = nullptr;
    }

    template<typename T>
    UINT size_of_256() { return (sizeof(T) + 255) & ~255; }

    Microsoft::WRL::ComPtr<ID3D12Resource>
    create_default_buffer(const void *data, UINT64 byte_size,
                          Microsoft::WRL::ComPtr<ID3D12Resource> &upload_buffer,
                          Microsoft::WRL::ComPtr<ID3D12Device> device,
                          Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd_list);

    template<typename T>
    D3D12_GPU_VIRTUAL_ADDRESS
    gpu_address(const Microsoft::WRL::ComPtr<ID3D12Resource> &buf, int index) {
        auto ret = buf->GetGPUVirtualAddress();
        ret += size_of_256<T>() * index;
        return ret;
    }
}