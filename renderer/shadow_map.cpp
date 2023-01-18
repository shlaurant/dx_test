#include "shadow_map.h"
#include "debug.h"
#include "constants.h"

namespace directx_renderer {
    void shadow_map::init(ComPtr<ID3D12Device> device, int width, int height, D3D12_CPU_DESCRIPTOR_HANDLE dest) {
        viewport = {0.f, 0.f, (float) width, (float) height, 0.f, 1.f};
        scissors_rect = {0, 0, width, height};

        auto res_desc = CD3DX12_RESOURCE_DESC::Tex2D(SHADOW_FORMAT, width, height);
        res_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        auto heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        D3D12_CLEAR_VALUE clear_value = {};
        clear_value.Format = DSV_FORMAT;
        clear_value.DepthStencil.Depth = 1.f;
        clear_value.DepthStencil.Stencil = 0;

        ThrowIfFailed(device->CreateCommittedResource(&heap_prop,
                                                      D3D12_HEAP_FLAG_NONE,
                                                      &res_desc,
                                                      D3D12_RESOURCE_STATE_GENERIC_READ,
                                                      &clear_value,
                                                      IID_PPV_ARGS(
                                                              &depth_map)));

        D3D12_DESCRIPTOR_HEAP_DESC dsh_desc = {};
        dsh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dsh_desc.NodeMask = 0;
        dsh_desc.NumDescriptors = 1;
        dsh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

        device->CreateDescriptorHeap(&dsh_desc, IID_PPV_ARGS(&dsv_heap));

        D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
        dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
        dsv_desc.Format = DSV_FORMAT;
        dsv_desc.Flags = D3D12_DSV_FLAG_NONE;

        device->CreateDepthStencilView(depth_map.Get(), &dsv_desc,
                                       dsv_heap->GetCPUDescriptorHandleForHeapStart());

        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
        srv_desc.Format = SHADOW_SRV_FORMAT;
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc.Texture2D.MipLevels = 1;
        srv_desc.Texture2D.MostDetailedMip = 0;

        device->CreateShaderResourceView(depth_map.Get(), &srv_desc, dest);
    }
}