#pragma once

#include "common.h"

namespace directx_renderer {
    struct frame_resource {
        Microsoft::WRL::ComPtr<ID3D12Resource> global;
        Microsoft::WRL::ComPtr<ID3D12Resource> object_constants;
        Microsoft::WRL::ComPtr<ID3D12Resource> final_matrices;
    };

    class frame_resource_buffer {
    public:
        struct buffer_sizes {
            int buf_cnt;//size of circular buffer
            int obj_cnt;
            int skinned_cnt;
        };

        frame_resource_buffer(Microsoft::WRL::ComPtr<ID3D12Device> device,
                              const buffer_sizes &);

        bool has_room() const;
        void put(const global_frame &, const std::vector<object_constant> &,
                 const std::vector<skin_matrix> &);
        frame_resource peek() const;
        frame_resource get();

    private:
        std::vector<frame_resource> _frame_resources;

        int _size = 0;
        int _write = 0;
        int _read = 0;

        inline int next_of(int index) const { return (index + 1) % _size; };
    };
}