#pragma once

namespace fuse::directx {
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
        std::string material;
        transform tr;
        std::vector<DirectX::SimpleMath::Matrix> skinning_matrices;

    private:
        int id;
        geo_info geo;
        object_constant constants;

        friend class directx_12;
    };
}