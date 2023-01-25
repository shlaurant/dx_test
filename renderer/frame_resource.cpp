#include "frame_resource.h"
#include "dx_util.h"

namespace directx_renderer {
    frame_resource_buffer::frame_resource_buffer(
            Microsoft::WRL::ComPtr<ID3D12Device> device,
            const frame_resource_buffer::buffer_sizes &size) : _frame_resources(
            std::vector<frame_resource>(size.buf_cnt)) {

        for (auto i = 0; i < size.buf_cnt; ++i) {
            _frame_resources[i].global = create_const_buffer<global_frame>(1,
                                                                           device);
            _frame_resources[i].object_constants = create_const_buffer<object_constant>(
                    size.obj_cnt, device);
            _frame_resources[i].final_matrices = create_const_buffer<skin_matrix>(
                    size.skinned_cnt, device);
        }
    }

    bool frame_resource_buffer::is_waiting(UINT64 last_comp_fence) {
        return _frame_resources[_write].fence == 0 &&
               _frame_resources[_write].fence > last_comp_fence;
    }

    UINT64 frame_resource_buffer::fence_in_wait() {
        return _frame_resources[_write].fence;
    }

    void frame_resource_buffer::put(const global_frame &gf,
                                    const std::vector<object_constant> &ocv,
                                    const std::vector<skin_matrix> &smv,
                                    UINT64 fence) {
        update_const_buffer<global_frame>(_frame_resources[_write].global, &gf,
                                          0);
        for (auto i = 0; i < ocv.size(); ++i) {
            update_const_buffer<object_constant>
                    (_frame_resources[_write].object_constants, &(ocv[i]), i);
        }

        for (auto i = 0; i < ocv.size(); ++i) {
            update_const_buffer<skin_matrix>
                    (_frame_resources[_write].final_matrices, &(smv[i]), i);
        }

        _frame_resources[_write].fence = fence;

        _write = next_of(_write);
    }

    frame_resource frame_resource_buffer::get() {
        auto ret = _frame_resources[_read];
        _read = next_of(_read);
        return ret;
    }

    frame_resource frame_resource_buffer::peek() {
        return _frame_resources[_read];
    }
}