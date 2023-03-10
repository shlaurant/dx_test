#include "frame_resource.h"
#include "dx_util.h"

namespace directx_renderer {

    frame_resource_buffer::frame_resource_buffer(
            const Microsoft::WRL::ComPtr<ID3D12Device> &device,
            const frame_resource_size &size) : _resources(
            std::vector<frame_resource>(size.buf_size)) {
        for (auto &e: _resources) {
            e.frame_global =
                    create_const_buffer<frame_globals>(1, device);
            e.object_const =
                    create_const_buffer<object_constant>(size.obj_cnt, device);
            e.skin_matrix =
                    create_const_buffer<skin_matrix>(size.skin_cnt, device);
            device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                           IID_PPV_ARGS(&e.cmd_alloc));
        }
    }

    bool frame_resource_buffer::can_put(UINT64 fence) {
        return _resources[_put].fence == 0 ||
               _resources[_put].fence <= fence;
    }

    void frame_resource_buffer::put(const frame_globals &fg,
                                    const std::vector<object_constant> &ocv,
                                    const std::vector<skin_matrix> &smv,
                                    UINT64 fence) {
        auto &cur_res = _resources[_put];

        update_const_buffer<frame_globals>(cur_res.frame_global, &fg, 0);

        for (size_t i = 0; i < ocv.size(); ++i) {
            update_const_buffer<object_constant>
                    (cur_res.object_const, &(ocv[i]), i);
        }

        for (size_t i = 0; i < smv.size(); ++i) {
            update_const_buffer<skin_matrix>
                    (cur_res.skin_matrix, &(smv[i]), i);
        }

        cur_res.fence = fence;

        _put = (_put + 1) % _resources.size();
    }

    const frame_resource &frame_resource_buffer::peek() const {
        return _resources[_get];
    }

    const frame_resource &frame_resource_buffer::get() {
        const auto &ret = _resources[_get];

        _get = (_get + 1) % _resources.size();

        return ret;
    }
}