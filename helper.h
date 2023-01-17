#pragma once

#include <FBXLoader.h>
#include "Input.h"
#include "src/common.h"
#include "GameTimer.h"

struct transform {
    DirectX::SimpleMath::Vector3 position = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 rotation = DirectX::SimpleMath::Vector3::Zero;
    DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One;

    DirectX::SimpleMath::Matrix translation_matrix() const;
    DirectX::SimpleMath::Matrix rotation_matrix() const;
    DirectX::SimpleMath::Matrix scale_matrix() const;
};

struct camera {
    inline static float ratio() { return 1920.f / 1080.f; }

    const float speed_c = 20.f;
    const float rot_c = .2f;

    transform transform;

    float near_plane = 1.f;
    float far_plane = 100.f;
    float fov = DirectX::XM_PI / 4.f;

    DirectX::SimpleMath::Matrix view() const;
    DirectX::SimpleMath::Matrix projection() const;
    DirectX::SimpleMath::Vector3 look_vector() const;
    DirectX::SimpleMath::Vector3 right_vector() const;
};

class animator {
public:
    void init(const std::shared_ptr<FbxAnimClipInfo> &anim,
              const std::vector<std::shared_ptr<FbxBoneInfo>> &bones);
    void reset();
    void
    final_matrices_after(float delta, fuse::directx::skin_matrix &out); // linear interpolation

private:
    struct srt {
        DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::One;
        DirectX::SimpleMath::Vector4 rotation = DirectX::SimpleMath::Vector4::Zero;
        DirectX::SimpleMath::Vector3 translation = DirectX::SimpleMath::Vector3::Zero;

        DirectX::SimpleMath::Matrix affine_matrix() const;
    };

    double _start;
    double _end;
    double _time;
    int _frame;
    std::vector<double> _frame_times;
    std::vector<DirectX::SimpleMath::Matrix> _offsets;
    std::vector<int> _bone_parents;
    std::vector<std::vector<srt>> _bone_srts;//bone - frame

    inline size_t bone_cnt() {return _offsets.size();}
    inline size_t frame_cnt() {return _frame_times.size();}

    std::shared_ptr<FbxAnimClipInfo> _anim;
};

DirectX::SimpleMath::Matrix conv_mat(const FbxMatrix &);

DirectX::SimpleMath::Vector3 look_vector(std::shared_ptr<fuse::directx::camera> &);
DirectX::SimpleMath::Vector3 right_vector(std::shared_ptr<fuse::directx::camera> &);

void handle_input(Input &, std::shared_ptr<fuse::directx::camera> &,
                  const GameTimer &);

void print_matrix(const DirectX::SimpleMath::Matrix &);

void print_vector3(const DirectX::SimpleMath::Vector3 &);
void print_vector4(const DirectX::SimpleMath::Vector4 &);

void print_transform(const transform &);

DirectX::SimpleMath::Vector4
mult(const DirectX::SimpleMath::Vector4 &, const DirectX::SimpleMath::Matrix &);

fuse::directx::geometry<fuse::directx::vertex> create_cube();

fuse::directx::geometry<fuse::directx::vertex> create_tetra();

fuse::directx::geometry<fuse::directx::vertex> create_cube_uv();

fuse::directx::geometry<fuse::directx::vertex>
create_plain(int width, int height);

DirectX::SimpleMath::Vector4 white();
DirectX::SimpleMath::Vector4 red();
DirectX::SimpleMath::Vector4 green();
DirectX::SimpleMath::Vector4 blue();

fuse::directx::geometry<fuse::directx::vertex>
load_mesh(const std::string &path);

fuse::directx::geometry<fuse::directx::vertex>
create_terrain(int half, int unit_sz);