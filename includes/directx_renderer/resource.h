#pragma once

#include "common.h"

namespace fuse::directx {
    class resource {
    public:
        void init(int, Microsoft::WRL::ComPtr<ID3D12Device>,
                  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>);

        void update_world_matrix(int, const DirectX::SimpleMath::Matrix &);

        size_t load_texture(const std::wstring &);
        void bind_texture(int obj_index, size_t texture_index);

        void commit_table(int obj_index);
    private:
        static const int TABLE_SIZE = 2;

        int _obj_cnt;

        Microsoft::WRL::ComPtr<ID3D12Device> _device;
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> _cmd_list;

        Microsoft::WRL::ComPtr<ID3D12Resource> _w_buffer;
        std::vector<std::pair<D3D12_SHADER_RESOURCE_VIEW_DESC, Microsoft::WRL::ComPtr<ID3D12Resource>>> _texture_buffers;

        Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> _desc_heap;
    };
}
