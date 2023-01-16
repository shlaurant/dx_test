#include <SimpleMath.h>
#include <string>
#include "helper.h"

using namespace DirectX::SimpleMath;

DirectX::SimpleMath::Matrix conv_mat(const FbxMatrix &m) {
    DirectX::SimpleMath::Matrix ret;

    for (auto y = 0; y < 4; ++y) {
        for (auto x = 0; x < 4; ++x) {
            ret.m[y][x] = static_cast<float>(m.Get(y, x));
        }
    }

    return ret;
}

void animator::init(const std::shared_ptr<FbxAnimClipInfo> &anim,
                    const std::vector<std::shared_ptr<FbxBoneInfo>> &bones) {
    _start = anim->startTime.GetSecondDouble();
    _end = anim->endTime.GetSecondDouble();
    _time = 0;
    _frame = 0;

    _frame_times.clear();
    for (const auto &e: anim->keyFrames[0]) {
        _frame_times.push_back(e.time);
    }

    _offsets.resize(bones.size());
    for (auto i = 0; i < bones.size(); ++i) {
        _offsets[i] = conv_mat(bones[i]->matOffset);
    }

    _bone_srts.resize(anim->keyFrames.size());
    for (auto &e: _bone_srts) {
        e.clear();
        e.resize(frame_cnt());
    }

    for (auto frame = 0; frame < frame_cnt(); ++frame) {
        for (auto bone = 0; bone < bone_cnt(); ++bone) {
            auto &fv = anim->keyFrames[bone];
            if (fv.size() <= frame) {
                //do nothing
            } else {
                _bone_srts[bone][frame].scale.x = static_cast<float>(fv[frame].matTransform.GetS()[0]);
                _bone_srts[bone][frame].scale.y = static_cast<float>(fv[frame].matTransform.GetS()[1]);
                _bone_srts[bone][frame].scale.z = static_cast<float>(fv[frame].matTransform.GetS()[2]);

                _bone_srts[bone][frame].rotation.x = static_cast<float>(fv[frame].matTransform.GetQ()[0]);
                _bone_srts[bone][frame].rotation.y = static_cast<float>(fv[frame].matTransform.GetQ()[1]);
                _bone_srts[bone][frame].rotation.z = static_cast<float>(fv[frame].matTransform.GetQ()[2]);
                _bone_srts[bone][frame].rotation.w = static_cast<float>(fv[frame].matTransform.GetQ()[3]);

                _bone_srts[bone][frame].translation.x = static_cast<float>(fv[frame].matTransform.GetT()[0]);
                _bone_srts[bone][frame].translation.y = static_cast<float>(fv[frame].matTransform.GetT()[1]);
                _bone_srts[bone][frame].translation.z = static_cast<float>(fv[frame].matTransform.GetT()[2]);
            }
        }
    }
}

void animator::reset() {
    _frame = 0;
    _time = _start;
}

DirectX::SimpleMath::Matrix animator::srt::root_matrix() {
    return Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rotation) *
           Matrix::CreateTranslation(translation);
}

void
animator::final_matrices_after(float delta, fuse::directx::skin_matrix &out) {
    std::vector<DirectX::SimpleMath::Matrix> ret;
    ret.resize(bone_cnt(), Matrix::Identity);
//
//    _time += delta;
//    if (_time > _end) {
//        reset();
//    }
//
//    if (frame_cnt() > _frame + 1 && _time > _frame_times[_frame + 1]) {
//        ++_frame;
//    }
//
//    if (_frame == frame_cnt() - 1) {
//        for (auto i = 0; i < bone_cnt(); ++i) {
//            ret[i] = _offsets[i] * _bone_srts[i][_frame].root_matrix();
//        }
//    } else {
//        for (auto i = 0; i < bone_cnt(); ++i) {
//            auto t = (_time - _frame_times[_frame]) /
//                     (_frame_times[_frame + 1] - _frame_times[_frame]);
//            srt lerp_srt;
//            lerp_srt.scale =
//                    Vector3::Lerp(_bone_srts[i][_frame].scale,
//                                  _bone_srts[i][_frame].scale, t);
//            lerp_srt.rotation =
//                    Vector4::Lerp(_bone_srts[i][_frame].rotation,
//                                  _bone_srts[i][_frame].rotation, t);
//            lerp_srt.translation = Vector3::Lerp(
//                    _bone_srts[i][_frame].translation,
//                    _bone_srts[i][_frame].translation, t);
//            ret[i] = _offsets[i] * lerp_srt.root_matrix();
//        }
//    }
//
    for (auto i = 0; i < min(sizeof(out.matrices) / sizeof(out.matrices[0]),
                             ret.size()); ++i) {
        out.matrices[i] = ret[i];
    }
}

Matrix transform::translation_matrix() const {
    return Matrix::CreateTranslation(position);
}

Matrix transform::rotation_matrix() const {
    Matrix ret = Matrix::Identity;
    ret *= Matrix::CreateRotationX(rotation.x);
    ret *= Matrix::CreateRotationY(rotation.y);
    ret *= Matrix::CreateRotationZ(rotation.z);
    return ret;
}

Matrix transform::scale_matrix() const {
    return Matrix::CreateScale(scale);
}

DirectX::SimpleMath::Matrix camera::view() const {
    return (transform.rotation_matrix() *
            transform.translation_matrix()).Invert();
}

DirectX::SimpleMath::Matrix camera::projection() const {
    return DirectX::XMMatrixPerspectiveFovLH(fov, ratio(), near_plane,
                                             far_plane);
}

void handle_input(Input &input, std::shared_ptr<fuse::directx::camera> &camera,
                  const GameTimer &timer) {
    const float speed_c = 20.f;
    const float rot_c = .2f;

    int dx, dy;
    dx = input.mouse_delta().first;
    dy = input.mouse_delta().second;
    camera->tr.rotation.y += (float) dx * rot_c * timer.DeltaTime();
    camera->tr.rotation.x += (float) dy * rot_c * timer.DeltaTime();
    camera->tr.rotation.x = std::clamp(camera->tr.rotation.x,
                                       -DirectX::XM_PI / 2.f,
                                       DirectX::XM_PI / 2.f);


    if (input.GetButton(KEY_TYPE::W)) {
        camera->tr.position +=
                look_vector(camera) * speed_c * timer.DeltaTime();
    }

    if (input.GetButton(KEY_TYPE::S)) {
        camera->tr.position -=
                look_vector(camera) * speed_c * timer.DeltaTime();
    }

    if (input.GetButton(KEY_TYPE::A)) {
        camera->tr.position -=
                right_vector(camera) * speed_c * timer.DeltaTime();
    }

    if (input.GetButton(KEY_TYPE::D)) {
        camera->tr.position +=
                right_vector(camera) * speed_c * timer.DeltaTime();
    }

    if (input.GetButtonDown(KEY_TYPE::Q)) {
//        print_transform(camera->tr);
//        OutputDebugStringA("view:\n");
//        print_matrix(camera.view());
//        OutputDebugStringA("projection:\n");
//        print_matrix(camera.projection());
//        OutputDebugStringA("vp:\n");
//        print_matrix(camera.view() * camera.projection());
//        OutputDebugStringA("w:\n");
//        print_matrix(Matrix::CreateTranslation(Vector3(1.f, 0.f, 3.f)));
//        OutputDebugStringA("wvp:\n");
//        auto wvp = Matrix::CreateTranslation(Vector3(1.f, 0.f, 3.f)) *
//                   camera.view() * camera.projection();
//        print_matrix(wvp);
//        OutputDebugStringA("test mult:\n");
//        print_vector4(Vector4::Transform({0.f, 0.f, 0.f, 1.f}, wvp));
    }
}

void print_matrix(const Matrix &matrix) {
    auto m = matrix.m;
    for (auto i = 0; i < 4; ++i) {
        std::string str;
        for (auto e: m[i]) {
            str += std::to_string(e);
            str += " ";
        }
        str += "\n";
        OutputDebugStringA(str.c_str());
    }
}

void print_vector3(const Vector3 &v) {
    std::string str;

    str += std::to_string(v.x) + " ";
    str += std::to_string(v.y) + " ";
    str += std::to_string(v.z) + "\n";

    OutputDebugStringA(str.c_str());
}

void print_vector4(const Vector4 &v) {
    std::string str;

    str += std::to_string(v.x) + " ";
    str += std::to_string(v.y) + " ";
    str += std::to_string(v.z) + " ";
    str += std::to_string(v.w) + "\n";

    OutputDebugStringA(str.c_str());
}

void print_transform(const transform &t) {
    OutputDebugStringA("transform:\n");
    OutputDebugStringA("scale   : ");
    print_vector3(t.scale);
    OutputDebugStringA("rotation: ");
    print_vector3(t.rotation);
    OutputDebugStringA("position: ");
    print_vector3(t.position);
}

DirectX::SimpleMath::Vector4 sub_vec(const Matrix &m, int col) {
    Vector4 ret;
    ret.x = m.m[0][col];
    ret.y = m.m[1][col];
    ret.z = m.m[2][col];
    ret.w = m.m[3][col];
    return ret;
}

DirectX::SimpleMath::Vector4 mult(const Vector4 &v, const Matrix &m) {
    Vector4 ret;

    ret.x = v.Dot(sub_vec(m, 0));
    ret.y = v.Dot(sub_vec(m, 1));
    ret.z = v.Dot(sub_vec(m, 2));
    ret.w = v.Dot(sub_vec(m, 3));

    return ret;
}

fuse::directx::geometry<fuse::directx::vertex> create_cube() {
    static const float d = 0.5f;
    fuse::directx::geometry<fuse::directx::vertex> ret;
//    ret.vertices = {{{-d, d,  -d}, white()},
//                    {{d,  d,  -d}, red()},
//                    {{-d, -d, -d}, green()},
//                    {{d,  -d, -d}, blue()},
//                    {{-d, d,  d},  white()},
//                    {{d,  d,  d},  red()},
//                    {{-d, -d, d},  green()},
//                    {{d,  -d, d},  blue()}};
//    ret.indices = {0, 1, 2, 1, 3, 2,
//                   1, 7, 3, 1, 5, 7,
//                   0, 4, 1, 4, 5, 1,
//                   0, 2, 6, 0, 6, 4,
//                   2, 3, 6, 3, 7, 6,
//                   4, 6, 5, 5, 6, 7};

    return ret;
}

fuse::directx::geometry<fuse::directx::vertex> create_tetra() {
    fuse::directx::geometry<fuse::directx::vertex> ret;
//    static const float d = 0.5f;
//    ret.vertices = {{{0.f, d * 2, 0.f}, red()},
//                    {{0.f, 0.f,   d},   green()},
//                    {{d,   0.f,   -d},  blue()},
//                    {{-d,  0.f,   -d},  white()}};
//    ret.indices = {0, 2, 3,
//                   0, 1, 2,
//                   0, 3, 1,
//                   2, 1, 3};
    return ret;
}

fuse::directx::geometry<fuse::directx::vertex> create_cube_uv() {
    fuse::directx::geometry<fuse::directx::vertex> ret;

    float w2 = 0.5f;
    float h2 = 0.5f;
    float d2 = 0.5f;

    std::vector<fuse::directx::vertex> vec(24);

    vec[0] = {Vector3(-w2, -h2, -d2), Vector2(0.0f, 1.0f),
              Vector3(0.f, 0.f, -1.f)};
    vec[1] = {Vector3(-w2, +h2, -d2), Vector2(0.0f, 0.0f),
              Vector3(0.f, 0.f, -1.f)};
    vec[2] = {Vector3(+w2, +h2, -d2), Vector2(1.0f, 0.0f),
              Vector3(0.f, 0.f, -1.f)};
    vec[3] = {Vector3(+w2, -h2, -d2), Vector2(1.0f, 1.0f),
              Vector3(0.f, 0.f, -1.f)};
    vec[4] = {Vector3(-w2, -h2, +d2), Vector2(1.0f, 1.0f),
              Vector3(0.f, 0.f, 1.f)};
    vec[5] = {Vector3(+w2, -h2, +d2), Vector2(0.0f, 1.0f),
              Vector3(0.f, 0.f, 1.f)};
    vec[6] = {Vector3(+w2, +h2, +d2), Vector2(0.0f, 0.0f),
              Vector3(0.f, 0.f, 1.f)};
    vec[7] = {Vector3(-w2, +h2, +d2), Vector2(1.0f, 0.0f),
              Vector3(0.f, 0.f, 1.f)};
    vec[8] = {Vector3(-w2, +h2, -d2), Vector2(0.0f, 1.0f),
              Vector3(0.f, 1.f, 0.f)};
    vec[9] = {Vector3(-w2, +h2, +d2), Vector2(0.0f, 0.0f),
              Vector3(0.f, 1.f, 0.f)};
    vec[10] = {Vector3(+w2, +h2, +d2), Vector2(1.0f, 0.0f),
               Vector3(0.f, 1.f, 0.f)};
    vec[11] = {Vector3(+w2, +h2, -d2), Vector2(1.0f, 1.0f),
               Vector3(0.f, 1.f, 0.f)};
    vec[12] = {Vector3(-w2, -h2, -d2), Vector2(1.0f, 1.0f),
               Vector3(0.f, -1.f, 0.f)};
    vec[13] = {Vector3(+w2, -h2, -d2), Vector2(0.0f, 1.0f),
               Vector3(0.f, -1.f, 0.f)};
    vec[14] = {Vector3(+w2, -h2, +d2), Vector2(0.0f, 0.0f),
               Vector3(0.f, -1.f, 0.f)};
    vec[15] = {Vector3(-w2, -h2, +d2), Vector2(1.0f, 0.0f),
               Vector3(0.f, -1.f, 0.f)};
    vec[16] = {Vector3(-w2, -h2, +d2), Vector2(0.0f, 1.0f),
               Vector3(-1.f, 0.f, 0.f)};
    vec[17] = {Vector3(-w2, +h2, +d2), Vector2(0.0f, 0.0f),
               Vector3(-1.f, 0.f, 0.f)};
    vec[18] = {Vector3(-w2, +h2, -d2), Vector2(1.0f, 0.0f),
               Vector3(-1.f, 0.f, 0.f)};
    vec[19] = {Vector3(-w2, -h2, -d2), Vector2(1.0f, 1.0f),
               Vector3(-1.f, 0.f, 0.f)};
    vec[20] = {Vector3(+w2, -h2, -d2), Vector2(0.0f, 1.0f),
               Vector3(1.f, 0.f, 0.f)};
    vec[21] = {Vector3(+w2, +h2, -d2), Vector2(0.0f, 0.0f),
               Vector3(1.f, 0.f, 0.f)};
    vec[22] = {Vector3(+w2, +h2, +d2), Vector2(1.0f, 0.0f),
               Vector3(1.f, 0.f, 0.f)};
    vec[23] = {Vector3(+w2, -h2, +d2), Vector2(1.0f, 1.0f),
               Vector3(1.f, 0.f, 0.f)};

    std::vector<uint16_t> idx(36);

    idx[0] = 0;
    idx[1] = 1;
    idx[2] = 2;
    idx[3] = 0;
    idx[4] = 2;
    idx[5] = 3;
    idx[6] = 4;
    idx[7] = 5;
    idx[8] = 6;
    idx[9] = 4;
    idx[10] = 6;
    idx[11] = 7;
    idx[12] = 8;
    idx[13] = 9;
    idx[14] = 10;
    idx[15] = 8;
    idx[16] = 10;
    idx[17] = 11;
    idx[18] = 12;
    idx[19] = 13;
    idx[20] = 14;
    idx[21] = 12;
    idx[22] = 14;
    idx[23] = 15;
    idx[24] = 16;
    idx[25] = 17;
    idx[26] = 18;
    idx[27] = 16;
    idx[28] = 18;
    idx[29] = 19;
    idx[30] = 20;
    idx[31] = 21;
    idx[32] = 22;
    idx[33] = 20;
    idx[34] = 22;
    idx[35] = 23;

    ret.vertices = vec;
    ret.indices = idx;

    return ret;
}

fuse::directx::geometry<fuse::directx::vertex>
create_plain(int width, int height) {
    fuse::directx::geometry<fuse::directx::vertex> ret;

    auto cnt = (width + 1) * (height + 1);
    ret.vertices.resize(cnt);
    for (auto i = 0; i < cnt; ++i) {
        int x = i % (width + 1);
        int y = i / (width + 1);
        ret.vertices[i].position = Vector3(x * 2, y * 2, 0.f);
        ret.vertices[i].normal = Vector3::Backward;
        auto ux = x % 2 == 0 ? 0.f : 1.f;
        auto uy = y % 2 == 0 ? 0.f : 1.f;
        ret.vertices[i].uv = Vector2(ux, uy);
    }

    for (auto i = 0; i < height; ++i) {
        for (auto j = 0; j < width; ++j) {
            auto index = (width + 1) * i + j;
            ret.indices.push_back(index);
            ret.indices.push_back(index + width + 1);
            ret.indices.push_back(index + 1);
            ret.indices.push_back(index + width + 1);
            ret.indices.push_back(index + width + 2);
            ret.indices.push_back(index + 1);
        }
    }

    return std::move(ret);
}

DirectX::SimpleMath::Vector4 white() {
    return {1.f, 1.f, 1.f, 1.f};
}
DirectX::SimpleMath::Vector4 red() {
    return {1.f, 0.f, 0.f, 1.f};
}
DirectX::SimpleMath::Vector4 green() {
    return {0.f, 1.f, 0.f, 1.f};
}
DirectX::SimpleMath::Vector4 blue() {
    return {0.f, 0.f, 1.f, 1.f};
}

DirectX::SimpleMath::Vector3 read_v3(std::ifstream &ifs) {
    std::string str;
    ifs >> str;
    auto x = std::stof(str);
    ifs >> str;
    auto y = std::stof(str);
    ifs >> str;
    auto z = std::stof(str);
    return Vector3(x, y, z);
}

fuse::directx::geometry<fuse::directx::vertex>
load_mesh(const std::string &path) {
    fuse::directx::geometry<fuse::directx::vertex> ret;

    std::ifstream fs(path.data());
    if (fs.is_open()) {
        std::string str;
        fs >> str >> str;
        auto vcnt = std::stoi(str);
        fs >> str >> str;
        auto tcnt = std::stoi(str);

        std::getline(fs, str);
        std::getline(fs, str);
        std::getline(fs, str);

        for (auto i = 0; i < vcnt; ++i) {
            fuse::directx::vertex v;
            v.position = read_v3(fs);
            v.normal = read_v3(fs);
            ret.vertices.emplace_back(v);
            v.uv = Vector2(0.f, 0.f);
        }

        std::getline(fs, str);
        std::getline(fs, str);
        std::getline(fs, str);
        std::getline(fs, str);

        for (auto i = 0; i < tcnt; ++i) {
            fs >> str;
            uint16_t ind = std::stoi(str);
            ret.indices.push_back(ind);
            fs >> str;
            ind = std::stoi(str);
            ret.indices.push_back(ind);
            fs >> str;
            ind = std::stoi(str);
            ret.indices.push_back(ind);
        }
    }

    return ret;
}

fuse::directx::geometry<fuse::directx::vertex>
create_terrain(int half, int unit_sz) {
    fuse::directx::geometry<fuse::directx::vertex> ret;


    for (auto z = half; z >= -half; --z) {
        for (auto x = -half; x <= half; ++x) {
            fuse::directx::vertex vert;
            vert.position.x = x * unit_sz;
            vert.position.y = 0;
            vert.position.z = z * unit_sz;
            vert.uv.x = ((float) (x + half)) / (2 * half);
            vert.uv.y = ((float) (half - z)) / (2 * half);

            ret.vertices.push_back(vert);
        }
    }

    for (auto z = half; z > -half; --z) {
        for (auto x = -half; x < half; ++x) {
            int line_cnt = 2 * half + 1;
            auto point_index = (half - z) * line_cnt + x + half;
            ret.indices.push_back(point_index);
            ret.indices.push_back(point_index + 1);
            ret.indices.push_back(point_index + 1 + line_cnt);
            ret.indices.push_back(point_index);
            ret.indices.push_back(point_index + line_cnt + 1);
            ret.indices.push_back(point_index + line_cnt);
        }
    }

    return ret;
}

DirectX::SimpleMath::Vector3
look_vector(std::shared_ptr<fuse::directx::camera> &p) {
    return {Vector4::Transform(Vector4::UnitZ, p->tr.rotation_matrix() *
                                               p->tr.translation_matrix())};
}

DirectX::SimpleMath::Vector3
right_vector(std::shared_ptr<fuse::directx::camera> &p) {
    return {Vector4::Transform(Vector4::UnitX, p->tr.rotation_matrix() *
                                               p->tr.translation_matrix())};
}


DirectX::SimpleMath::Vector3 camera::look_vector() const {
    return {Vector4::Transform(Vector4::UnitZ, transform.rotation_matrix() *
                                               transform.translation_matrix())};
}

DirectX::SimpleMath::Vector3 camera::right_vector() const {
    return {Vector4::Transform(Vector4::UnitX, transform.rotation_matrix() *
                                               transform.translation_matrix())};
}