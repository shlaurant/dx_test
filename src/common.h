#pragma once

#include "SimpleMath.h"

namespace fuse::directx {
    struct transform {
        DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero;
        DirectX::SimpleMath::Vector3 rotation = DirectX::SimpleMath::Vector3::Zero;
        DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One;

        inline DirectX::SimpleMath::Matrix rotation_matrix() const {
            return DirectX::SimpleMath::Matrix::CreateRotationX(rotation.x) *
                   DirectX::SimpleMath::Matrix::CreateRotationY(rotation.y) *
                   DirectX::SimpleMath::Matrix::CreateRotationY(rotation.z);
        }

        inline DirectX::SimpleMath::Matrix translation_matrix() const {
            return DirectX::SimpleMath::Matrix::CreateTranslation(position);
        }

        inline DirectX::SimpleMath::Matrix world_matrix() const {
            return DirectX::SimpleMath::Matrix::CreateScale(scale) *
                   rotation_matrix() * translation_matrix();
        }
    };

    struct camera {
        float w_h_ratio = 1920.f / 1080.f;
        float n = 1.f;
        float f = 1000.f;
        float fov = DirectX::XM_PI / 4.f;
        transform tr;

        inline DirectX::SimpleMath::Matrix view() const {
            return (tr.rotation_matrix() * tr.translation_matrix()).Invert();
        }

        inline DirectX::SimpleMath::Matrix projection() const {
            return DirectX::XMMatrixPerspectiveFovLH(fov, w_h_ratio, n, f);
        }
    };

    struct render_info {
        bool is_billboard;
        int object_index;
        int index_count;
        int index_offset;
        int vertex_offset;
        bool is_transparent;
        bool do_reflect;
        bool is_mirror;
        DirectX::SimpleMath::Vector4 mirror_plane;
        bool do_shadow;
        bool is_terrain = false;
    };

    struct vertex {
        DirectX::SimpleMath::Vector3 position;
        DirectX::SimpleMath::Vector2 uv;
        DirectX::SimpleMath::Vector3 normal;
        DirectX::SimpleMath::Vector3 tangent = DirectX::SimpleMath::Vector3::Right;
        DirectX::SimpleMath::Vector4 weights;
        DirectX::SimpleMath::Vector4 indices;
    };

    struct vertex_billboard {
        DirectX::SimpleMath::Vector3 position;
        DirectX::SimpleMath::Vector2 size;
    };

    template<typename T>
    struct geometry {
        std::string name;
        std::vector<T> vertices;
        std::vector<uint16_t> indices;

        DirectX::SimpleMath::Matrix world_matrix = DirectX::SimpleMath::Matrix::Identity;

        size_t vertex_offset = 0;
        size_t index_offset = 0;
    };

    struct material {
        DirectX::SimpleMath::Vector4 diffuse_albedo;
        DirectX::SimpleMath::Vector3 fresnel_r0;
        float roughness;
    };

    struct object_constant {
        DirectX::SimpleMath::Vector3 position;
        int mat_id;
        DirectX::SimpleMath::Matrix world_matrix;
    };

    struct camera_buf {
        DirectX::SimpleMath::Matrix vp;
        DirectX::SimpleMath::Vector3 position;
        float pad0;
    };

    enum light_type {
        DIRECTIONAL = 0, POINT, SPOT, AMBIENT, NONE
    };

    struct light {
        int type = (int) light_type::NONE;
        DirectX::SimpleMath::Vector3 color;
        float fo_start;
        DirectX::SimpleMath::Vector3 direction;
        float fo_end;
        DirectX::SimpleMath::Vector3 position;
        float spot_pow;
        DirectX::SimpleMath::Vector3 pad0;
    };

    struct light_info {
        light lights[50];//should be same with LIGHT_COUNT in shader code
        int active_count;
        DirectX::SimpleMath::Vector3 pad0;
    };

    struct global {
        DirectX::SimpleMath::Matrix reflection_matrix[10];
        int reflection_count;
        DirectX::SimpleMath::Vector3 pad0;
        DirectX::SimpleMath::Matrix shadow_matrix;
        DirectX::SimpleMath::Matrix light_vp;
        DirectX::SimpleMath::Matrix shadow_uv_matrix;
    };

    struct skin_matrix {
        DirectX::SimpleMath::Matrix matrices[256];
    };
}