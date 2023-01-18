#include "dx_util.h"

using namespace Microsoft::WRL;

namespace directx_renderer {
    ComPtr <ID3D12Resource> create_upload_buffer(int elem_cnt, UINT64 elem_size,
                                                 ComPtr <ID3D12Device> device) {
        auto sz = elem_size * elem_cnt;
        ComPtr<ID3D12Resource> ret;
        auto heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto res_desc = CD3DX12_RESOURCE_DESC::Buffer(sz);
        device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
                                        &res_desc,
                                        D3D12_RESOURCE_STATE_GENERIC_READ,
                                        nullptr, IID_PPV_ARGS(&ret));
        return ret;
    }

    ComPtr <ID3D12Resource>
    create_default_buffer(const void *data, UINT64 byte_size,
                          ComPtr <ID3D12Resource> &upload_buffer,
                          ComPtr <ID3D12Device> device,
                          Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd_list) {
        upload_buffer = create_upload_buffer(1, byte_size, device);

        ComPtr<ID3D12Resource> default_buffer;
        auto dbh_desc = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto db_desc = CD3DX12_RESOURCE_DESC::Buffer(byte_size);
        device->CreateCommittedResource(&dbh_desc, D3D12_HEAP_FLAG_NONE,
                                        &db_desc, D3D12_RESOURCE_STATE_COMMON,
                                        nullptr,
                                        IID_PPV_ARGS(&default_buffer));

        D3D12_SUBRESOURCE_DATA sub_data = {};
        sub_data.pData = data;
        sub_data.RowPitch = byte_size;
        sub_data.SlicePitch = sub_data.RowPitch;

        auto b0 = CD3DX12_RESOURCE_BARRIER::Transition(default_buffer.Get(),
                                                       D3D12_RESOURCE_STATE_COMMON,
                                                       D3D12_RESOURCE_STATE_COPY_DEST);
        cmd_list->ResourceBarrier(1, &b0);
        UpdateSubresources<1>(cmd_list.Get(), default_buffer.Get(),
                              upload_buffer.Get(),
                              0, 0, 1, &sub_data);
        auto b1 = CD3DX12_RESOURCE_BARRIER::Transition(default_buffer.Get(),
                                                       D3D12_RESOURCE_STATE_COPY_DEST,
                                                       D3D12_RESOURCE_STATE_GENERIC_READ);
        cmd_list->ResourceBarrier(1, &b1);

        return default_buffer;
    }
}