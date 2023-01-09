#pragma once

using namespace Microsoft::WRL;

namespace fuse::directx {
    class shadow_map {
    public:
        void init(ComPtr<ID3D12Device> device, int width, int height, D3D12_CPU_DESCRIPTOR_HANDLE dest);

        D3D12_VIEWPORT viewport;
        D3D12_RECT scissors_rect;

        ComPtr<ID3D12Resource> depth_map;
        ComPtr<ID3D12DescriptorHeap> dsv_heap;
        ComPtr<ID3D12DescriptorHeap> srv_heap;
    };
}
