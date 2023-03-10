#include <SimpleMath.h>
#include <string>
#include "helper.h"
#include "DirectXMath.h"

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
    _anim = anim;

    _frame_times.clear();
    for (const auto &e: anim->keyFrames[0]) {
        _frame_times.push_back(e.time);
    }

    _offsets.resize(bones.size());
    for (auto i = 0; i < bones.size(); ++i) {
        _offsets[i] = conv_mat(bones[i]->matOffset.Transpose());
    }

    _bone_parents.resize(bones.size());
    for (auto i = 0; i < bones.size(); ++i) {
        _bone_parents[i] = bones[i]->parentIndex;
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
                _bone_srts[bone][frame].scale.x = static_cast<float>(fv[frame].matTransform.GetS().mData[0]);
                _bone_srts[bone][frame].scale.y = static_cast<float>(fv[frame].matTransform.GetS().mData[1]);
                _bone_srts[bone][frame].scale.z = static_cast<float>(fv[frame].matTransform.GetS().mData[2]);

                _bone_srts[bone][frame].rotation.x = static_cast<float>(fv[frame].matTransform.GetQ().mData[0]);
                _bone_srts[bone][frame].rotation.y = static_cast<float>(fv[frame].matTransform.GetQ().mData[1]);
                _bone_srts[bone][frame].rotation.z = static_cast<float>(fv[frame].matTransform.GetQ().mData[2]);
                _bone_srts[bone][frame].rotation.w = static_cast<float>(fv[frame].matTransform.GetQ().mData[3]);

                _bone_srts[bone][frame].translation.x = static_cast<float>(fv[frame].matTransform.GetT().mData[0]);
                _bone_srts[bone][frame].translation.y = static_cast<float>(fv[frame].matTransform.GetT().mData[1]);
                _bone_srts[bone][frame].translation.z = static_cast<float>(fv[frame].matTransform.GetT().mData[2]);
            }
        }
    }
}

void animator::reset() {
    _frame = 0;
    _time = _start;
}

DirectX::SimpleMath::Matrix animator::srt::affine_matrix() const {
    auto zero = Vector4(0.f, 0.f, 0.f, 1.f);
    return DirectX::XMMatrixAffineTransformation(scale, zero, rotation,
                                                 translation);
}

void
animator::final_matrices_after(float delta, directx_renderer::skin_matrix &out) {
    std::vector<DirectX::SimpleMath::Matrix> ret;
    ret.resize(bone_cnt(), Matrix::Identity);

    _time += delta;
    if (_time > _end) {
        reset();
    }

    if (frame_cnt() > _frame + 1 && _time > _frame_times[_frame + 1]) {
        ++_frame;
    }

    if (_frame == frame_cnt() - 1) {
        for (auto i = 0; i < bone_cnt(); ++i) {
            ret[i] = _offsets[i] * _bone_srts[i][_frame].affine_matrix();
        }
    } else {
        for (auto i = 0; i < bone_cnt(); ++i) {
            auto t = (_time - _frame_times[_frame]) /
                     (_frame_times[_frame + 1] - _frame_times[_frame]);
            srt lerp_srt;
            lerp_srt.scale =
                    Vector3::Lerp(_bone_srts[i][_frame].scale,
                                  _bone_srts[i][_frame].scale, t);
            lerp_srt.rotation =Quaternion::Slerp(_bone_srts[i][_frame].rotation,
                                  _bone_srts[i][_frame].rotation, t);
            lerp_srt.translation = Vector3::Lerp(
                    _bone_srts[i][_frame].translation,
                    _bone_srts[i][_frame].translation, t);
            ret[i] = _offsets[i] * lerp_srt.affine_matrix();
        }
    }

    for (auto i = 0; i < min(sizeof(out.matrices) / sizeof(out.matrices[0]),
                             ret.size()); ++i) {
        out.matrices[i] = ret[i];
    }
//
///** test code **/
//    for (auto i = 0; i < min(sizeof(out.matrices) / sizeof(out.matrices[0]),
//                             ret.size()); ++i) {
//        if (_frame < _anim->keyFrames[i].size()) {
//            auto t = _offsets[i] * _bone_srts[i][_frame].affine_matrix();
//            out.matrices[i] = t;
//        } else {
//            out.matrices[i] = Matrix::Identity;
//        }
//    }
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

void handle_input(Input &input, std::shared_ptr<directx_renderer::camera> &camera,
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

directx_renderer::geometry<directx_renderer::vertex> create_cube() {
    static const float d = 0.5f;
    directx_renderer::geometry<directx_renderer::vertex> ret;
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

directx_renderer::geometry<directx_renderer::vertex> create_tetra() {
    directx_renderer::geometry<directx_renderer::vertex> ret;
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

directx_renderer::geometry<directx_renderer::vertex> create_cube_uv() {
    directx_renderer::geometry<directx_renderer::vertex> ret;

    float w2 = 0.5f;
    float h2 = 0.5f;
    float d2 = 0.5f;

    std::vector<directx_renderer::vertex> vec(24);

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
    ret.indices.push_back(idx);

    return ret;
}

directx_renderer::geometry<directx_renderer::vertex>
create_plain(float width, float height) {
    directx_renderer::geometry<directx_renderer::vertex> ret;

        auto normal = Vector3::Forward;
        Vector3 tangent(1.f, 0.f, 0.f);

    {
        Vector3 pos(-width/2.f, height/2.f, 0.f);
        Vector2 uv(0.f, 0.f);
        ret.vertices.push_back({pos, uv, normal, tangent});
    }

    {
        Vector3 pos(width/2.f, height/2.f, 0.f);
        Vector2 uv(1.f, 0.f);
        ret.vertices.push_back({pos, uv, normal, tangent});
    }

    {
        Vector3 pos(-width/2.f, -height/2.f, 0.f);
        Vector2 uv(0.f, 1.f);
        ret.vertices.push_back({pos, uv, normal, tangent});
    }

    {
        Vector3 pos(width/2.f, -height/2.f, 0.f);
        Vector2 uv(1.f, 1.f);
        ret.vertices.push_back({pos, uv, normal, tangent});
    }

    ret.indices.push_back({0, 1, 2, 1, 3, 2});

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

directx_renderer::geometry<directx_renderer::vertex>
load_mesh(const std::string &path) {
    directx_renderer::geometry<directx_renderer::vertex> ret;
    ret.indices.resize(1);

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
            directx_renderer::vertex v;
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
            ret.indices[0].push_back(ind);
            fs >> str;
            ind = std::stoi(str);
            ret.indices[0].push_back(ind);
            fs >> str;
            ind = std::stoi(str);
            ret.indices[0].push_back(ind);
        }
    }

    return ret;
}

directx_renderer::geometry<directx_renderer::vertex>
create_terrain(int half, int unit_sz) {
    directx_renderer::geometry<directx_renderer::vertex> ret;
    ret.indices.resize(1);

    for (auto z = half; z >= -half; --z) {
        for (auto x = -half; x <= half; ++x) {
            directx_renderer::vertex vert;
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
            ret.indices[0].push_back(point_index);
            ret.indices[0].push_back(point_index + 1);
            ret.indices[0].push_back(point_index + 1 + line_cnt);
            ret.indices[0].push_back(point_index);
            ret.indices[0].push_back(point_index + line_cnt + 1);
            ret.indices[0].push_back(point_index + line_cnt);
        }
    }

    return ret;
}

DirectX::SimpleMath::Vector3
look_vector(std::shared_ptr<directx_renderer::camera> &p) {
    return {Vector4::Transform(Vector4::UnitZ, p->tr.rotation_matrix() *
                                               p->tr.translation_matrix())};
}

DirectX::SimpleMath::Vector3 look_vector(const directx_renderer::camera &c) {
    return {Vector4::Transform(Vector4::UnitZ, c.tr.rotation_matrix() *
                                               c.tr.translation_matrix())};
}

DirectX::SimpleMath::Vector3
right_vector(std::shared_ptr<directx_renderer::camera> &p) {
    return {Vector4::Transform(Vector4::UnitX, p->tr.rotation_matrix() *
                                               p->tr.translation_matrix())};
}

DirectX::SimpleMath::Vector3 right_vector(const directx_renderer::camera &c) {
    return {Vector4::Transform(Vector4::UnitX, c.tr.rotation_matrix() *
                                               c.tr.translation_matrix())};
}


DirectX::SimpleMath::Vector3 camera::look_vector() const {
    return {Vector4::Transform(Vector4::UnitZ, transform.rotation_matrix() *
                                               transform.translation_matrix())};
}

DirectX::SimpleMath::Vector3 camera::right_vector() const {
    return {Vector4::Transform(Vector4::UnitX, transform.rotation_matrix() *
                                               transform.translation_matrix())};
}