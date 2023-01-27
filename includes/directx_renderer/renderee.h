#pragma once

#include "common.h"

namespace directx_renderer {
    enum class renderee_type : uint8_t {
        opaque, translucent, billboard, terrain, skybox, opaque_skinned, count
    };

    enum renderee_option {
    };

    struct geo_info {
        UINT vertex_offset;
        UINT index_offset;
        UINT index_count;
    };

    class renderee {
    public:
        renderee_type type;
        std::string name;
        std::string geometry;
        std::string texture[2] = {"", ""};
        int material;
        int skin_matrix;

    private:
        int id;
        geo_info geo;

        friend class dx12_renderer;
    };
}