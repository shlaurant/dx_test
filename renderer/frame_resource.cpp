#include "frame_resource.h"
#include "dx_util.h"

namespace directx_renderer {

    typedef std::pair<UINT64, frame_resource> fres;

    frame_resource_buffer::frame_resource_buffer(
            Microsoft::WRL::ComPtr<ID3D12Device> device, uint8_t buf_size,
            size_t obj_cnt) : _resources(std::vector<fres>(buf_size)) {
        for (auto &e: _resources) {
            e.first = 0;
            e.second.frame_global =
                    create_const_buffer<frame_globals>(1, device);
            e.second.object_const =
                    create_const_buffer<object_constant>(obj_cnt, device);
            e.second.skin_matrix =
                    create_const_buffer<skin_matrix>(obj_cnt, device);
        }
    }

    bool frame_resource_buffer::can_put(UINT64 fence) {
        return _resources[cur_index].first == 0 ||
               _resources[cur_index].first <= fence;
    }

    void frame_resource_buffer::put(const frame_globals &fg,
                                    const std::vector<object_constant> &ocv,
                                    const std::vector<skin_matrix> &smv,
                                    UINT64 fence) {
        auto &cur_res = _resources[cur_index];

        update_const_buffer<frame_globals>(cur_rems.second.frame_global, &fg, 0);

        for (size_t i = 0; i < ocv.size(); ++i) {
            update_const_buffer<object_constant>(cur_res.second.object_const,
                                                 &(ocv[i]), i);
        }

        for (size_t i = 0; i < smv.size(); ++i) {
            update_const_buffer<skin_matrix>(cur_res.second.skin_matrix,
                                             &(smv[i]), i);
        }

        cur_index = (cur_index + 1) % _resources.size();
    }
}