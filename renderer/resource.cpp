#include "resource.h"
#include "dx_util.h"
#include "debug.h"

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;

namespace directx_renderer {
    void
    resource::init(int obj_cnt, Microsoft::WRL::ComPtr<ID3D12Device> device,
                   Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmd_list) {
        _obj_cnt = obj_cnt;
        _device = device;
        _cmd_list = cmd_list;
        _w_buffer = create_const_buffer<Matrix>(obj_cnt, _device);

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
        heap_desc.NodeMask = 0;
        heap_desc.NumDescriptors = TABLE_SIZE * _obj_cnt;
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        _device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&_desc_heap));

        auto desc_size = _device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        auto cpu_handle = _desc_heap->GetCPUDescriptorHandleForHeapStart();
        for (auto i = 0; i < _obj_cnt; ++i) {
            D3D12_CONSTANT_BUFFER_VIEW_DESC view_desc;
            view_desc.SizeInBytes = size_of_256<Matrix>();
            view_desc.BufferLocation = _w_buffer->GetGPUVirtualAddress() +
                                       view_desc.SizeInBytes * i;

            _device->CreateConstantBufferView(&view_desc, cpu_handle);
            cpu_handle.ptr += desc_size * TABLE_SIZE;
        }
    }

    void resource::update_world_matrix(int obj_index, const Matrix &matrix) {
        update_const_buffer(_w_buffer, &matrix, obj_index);
    }

    size_t resource::load_texture(const std::wstring &path) {
        DirectX::ScratchImage image;
        DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr,
                                 image);
        ComPtr<ID3D12Resource> buf;
        ThrowIfFailed(
                DirectX::CreateTexture(_device.Get(), image.GetMetadata(),
                                       &buf));

        std::vector<D3D12_SUBRESOURCE_DATA> sub_reses;
        ThrowIfFailed(DirectX::PrepareUpload(_device.Get(), image.GetImages(),
                                             image.GetImageCount(),
                                             image.GetMetadata(), sub_reses));

        auto ub = create_upload_buffer(1,
                                       GetRequiredIntermediateSize(buf.Get(), 0,
                                                                   static_cast<uint32_t>(sub_reses.size())),
                                       _device);
        UpdateSubresources(_cmd_list.Get(), buf.Get(), ub.Get(),
                           0, 0, static_cast<uint32_t>(sub_reses.size()),
                           sub_reses.data());

        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        desc.Format = image.GetMetadata().format;
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        desc.Texture2D.MipLevels = 1;

        _texture_buffers.emplace_back(std::make_pair(desc, buf));
        return _texture_buffers.size() - 1;
    }

    void resource::bind_texture(int obj_index, size_t texture_index) {
        auto target_tex = _texture_buffers[texture_index];
        auto handle = _desc_heap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += _device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * TABLE_SIZE *
                      obj_index + _device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        _device->CreateShaderResourceView(target_tex.second.Get(),
                                          &target_tex.first, handle);
    }

    void resource::commit_table(int obj_index) {
        auto desc_size = _device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        auto handle = _desc_heap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += desc_size * TABLE_SIZE * obj_index;
        _cmd_list->SetGraphicsRootDescriptorTable(1, handle);
    }
}