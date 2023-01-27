#pragma once

#include "common.h"

namespace directx_renderer {
    struct frame_resource {
        Microsoft::WRL::ComPtr<ID3D12Resource> frame_global;
        Microsoft::WRL::ComPtr<ID3D12Resource> object_const;
        Microsoft::WRL::ComPtr<ID3D12Resource> skin_matrix;

        UINT64 fence = 0;
    };

    struct frame_resource_size {
        int buf_size;
        int obj_cnt;
        int skin_cnt;
    };

    class frame_resource_buffer {
    public:
        frame_resource_buffer(Microsoft::WRL::ComPtr<ID3D12Device> device,const frame_resource_size &size);
        bool can_put(UINT64 last_comp_fence);
        inline UINT64 cur_fence() const { return _resources[_put].fence; }
        void put(const frame_globals &, const std::vector<object_constant> &,
                 const std::vector<skin_matrix> &, UINT64 fence);
        const frame_resource &peek() const;
        const frame_resource &get();

    private:
        std::vector<frame_resource> _resources;
        int _put = 0;
        int _get = 0;
    };
}