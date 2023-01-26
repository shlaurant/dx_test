#pragma once

#include "common.h"

namespace directx_renderer {
    struct frame_resource {
        Microsoft::WRL::ComPtr<ID3D12Resource> frame_global;
        Microsoft::WRL::ComPtr<ID3D12Resource> object_const;
        Microsoft::WRL::ComPtr<ID3D12Resource> skin_matrix;
    };

    class frame_resource_buffer {
    public:
        frame_resource_buffer(Microsoft::WRL::ComPtr<ID3D12Device> device, uint8_t buf_size, size_t obj_cnt);
        bool can_put(UINT64 last_comp_fence);
        void put(const frame_globals &, const std::vector<object_constant> &,
                 const std::vector<skin_matrix> &, UINT64 fence);

    private:
        std::vector<std::pair<UINT64, frame_resource>> _resources;
        int cur_index = 0;
    };
}