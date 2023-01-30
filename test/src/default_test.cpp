#include "default_test.h"

using namespace DirectX::SimpleMath;

void default_test::init(const directx_renderer::window_info &info,
                        std::shared_ptr<Input> input) {
    _input = input;
    _input->Init(info.hwnd);
    _renderer.init(info);
    create_light_info();
    _dragon_fbx.LoadFbx(L"resource\\Dragon.fbx", "dragon");
    _animator.init(_dragon_fbx.GetAnimClip()[0], _dragon_fbx.GetBones());
    create_geometries();
    load_geometries();
    load_textures();
    load_materials();
    build_renderees();
    _renderer.init_renderees(_renderees);
    _camera.tr.rotation.x = DirectX::XM_PI / 4.f;
    _camera.tr.position.z = -2.f;
    _camera.tr.position.y = 5.f;
    _ocv.resize(_renderees.size());
    _smv.resize(1);
    for (auto &e: _smv) {
        _animator.final_matrices_after(0.f, e);
    }
}

void default_test::update(float delta) {
    _input->Update();
    for (auto &e: _smv)
        _animator.final_matrices_after(delta, e);
    handle_input(delta);
    _frame_globals.camera_position = _camera.tr.position;
    _frame_globals.camera_vp = _camera.view() * _camera.projection();

    Vector3 center = Vector3::Zero;
    Vector3 light_vec = Vector3(0.f, -1.f, 1.f);
    light_vec.Normalize();
    directx_renderer::camera light_cam;
    light_cam.tr.position = center + (-light_vec * 2) * 10.f;
    light_cam.tr.rotation.x = DirectX::XM_PI / 4.f;
    auto view = light_cam.view();
    auto proj = DirectX::XMMatrixOrthographicLH(160.f, 160.f, 1.f,
                                                100.f);
    Matrix ndc_to_uv = {.5f, .0f, .0f, .0f,
                        .0f, -.5f, .0f, .0f,
                        .0f, .0f, 1.f, 0.f,
                        .5f, .5f, .0f, 1.f};

    _frame_globals.light_vp = view * proj;
    _frame_globals.shadow_uv = _frame_globals.light_vp * ndc_to_uv;

    for (auto i = 0; i < _renderees.size(); ++i) {
        _ocv[i].position = _trv[i].position;
        _ocv[i].world_matrix = _trv[i].world_matrix();
        _ocv[i].mat_id = _renderees[i]->material;
    }

    _renderer.update_frame(_frame_globals, _ocv, _smv);
}

void default_test::draw() {
    if (_blur)_renderer.render(directx_renderer::OPTION_BLUR);
    else _renderer.render();
}

void default_test::create_geometries() {
    auto cube0 = create_cube_uv();
    cube0.names.emplace_back("cube");
    _geometries.emplace_back(cube0);

    auto plane = create_plain(100, 100);
    plane.names.emplace_back("plane");
    _geometries.emplace_back(plane);

    auto mirror = create_plain(30, 20);
    mirror.names.emplace_back("mirror");
    _geometries.emplace_back(mirror);

    auto skull = load_mesh("resource/skull.txt");
    skull.names.emplace_back("skull");
    _geometries.emplace_back(skull);

    auto terrain = create_terrain(10, 4);
    terrain.names.emplace_back("terrain");
    _geometries.emplace_back(terrain);

    for (const auto &e: _dragon_fbx.geometries()) {
        _geometries.emplace_back(e);
    }
}

void default_test::create_light_info() {
    _frame_globals.light_count = 2;

    _frame_globals.lights[0].type = 0;
    _frame_globals.lights[0].color = DirectX::SimpleMath::Vector3(1.f, 1.f,
                                                                  1.f);
    _frame_globals.lights[0].fo_start;
    _frame_globals.lights[0].direction = Vector3(0.f, -1.f, 1.f);
    _frame_globals.lights[0].direction.Normalize();
    _frame_globals.lights[0].fo_end;
    _frame_globals.lights[0].position;
    _frame_globals.lights[0].spot_pow;

    _frame_globals.lights[1].type = 3;
    _frame_globals.lights[1].color = DirectX::SimpleMath::Vector3(.4f, .4f,
                                                                  .4f);
    _frame_globals.lights[1].fo_start;
    _frame_globals.lights[1].direction;
    _frame_globals.lights[1].fo_end;
    _frame_globals.lights[1].position;
    _frame_globals.lights[1].spot_pow;
}

void default_test::build_renderees() {
    enum mat {
        def, metal, rough, glass, ter
    };
    auto skull0 = std::make_shared<directx_renderer::renderee>();
    skull0->name = "skull0";
    skull0->type = directx_renderer::renderee_type::opaque;
    skull0->geometry = "skull";
    skull0->texture[0] = "marble_diffuse";
    skull0->texture[1] = "default_normal";
    skull0->material = def;
    {
        directx_renderer::transform tr;
        tr.scale = Vector3(1.f, 1.f, 1.f);
        tr.position = Vector3(-5.f, 6.f, 3.f);
        _trv.push_back(tr);
    }
    _renderees.emplace_back(skull0);

    auto skull1 = std::make_shared<directx_renderer::renderee>();
    skull1->name = "skull1";
    skull1->type = directx_renderer::renderee_type::opaque;
    skull1->geometry = "skull";
    skull1->texture[0] = "marble_diffuse";
    skull1->texture[1] = "default_normal";
    skull1->material = rough;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(-12.f, 6.f, 3.f);
        _trv.push_back(tr);
    }
    _renderees.emplace_back(skull1);

    auto skull2 = std::make_shared<directx_renderer::renderee>();
    skull2->name = "skull2";
    skull2->type = directx_renderer::renderee_type::opaque;
    skull2->geometry = "skull";
    skull2->texture[0] = "marble_diffuse";
    skull2->texture[1] = "default_normal";
    skull2->material = metal;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(-19.f, 6.f, 3.f);
        _trv.push_back(tr);
    }
    _renderees.emplace_back(skull2);

    auto skull3 = std::make_shared<directx_renderer::renderee>();
    skull3->name = "skull3";
    skull3->type = directx_renderer::renderee_type::translucent;
    skull3->geometry = "skull";
    skull3->texture[0] = "marble_diffuse";
    skull3->texture[1] = "default_normal";
    skull3->material = glass;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(-26.f, 6.f, 3.f);
        _trv.push_back(tr);
    }
    _renderees.emplace_back(skull3);

    auto wire = std::make_shared<directx_renderer::renderee>();
    wire->name = "wire";
    wire->type = directx_renderer::renderee_type::translucent;
    wire->geometry = "cube";
    wire->texture[0] = "wire";
    wire->material = rough;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(0.f, 6.f, 3.f);
        _trv.push_back(tr);
    }
    _renderees.emplace_back(wire);

    auto cube0 = std::make_shared<directx_renderer::renderee>();
    cube0->name = "cube0";
    cube0->type = directx_renderer::renderee_type::translucent;
    cube0->geometry = "cube";
    cube0->texture[0] = "marble_diffuse";
    cube0->texture[1] = "marble_normal";
    cube0->material = def;
//    {
//        directx_renderer::transform tr;
//        tr.position = Vector3(3.f, 20.f, 3.f);
//        _trv.push_back(tr);
//    }
//    _renderees.emplace_back(cube0);

    auto tree_billboard = std::make_shared<directx_renderer::renderee>();
    tree_billboard->name = "tree_billboard";
    tree_billboard->type = directx_renderer::renderee_type::billboard;
    tree_billboard->geometry = "billboard";
    tree_billboard->texture[0] = "tree_arr";
    tree_billboard->material = rough;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(-26.f, 4.f, 15.f);
        _trv.push_back(tr);
    }
    _renderees.emplace_back(tree_billboard);

    auto terrain = std::make_shared<directx_renderer::renderee>();
    terrain->name = "terrain";
    terrain->type = directx_renderer::renderee_type::terrain;
    terrain->geometry = "terrain";
    terrain->texture[0] = "terrain_d";
    terrain->texture[1] = "terrain_h";
    terrain->material = ter;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(0.f, 0.f, 0.f);
        _trv.push_back(tr);
    }
    _renderees.emplace_back(terrain);

    auto sky = std::make_shared<directx_renderer::renderee>();
    sky->name = "sky";
    sky->type = directx_renderer::renderee_type::skybox;
    sky->geometry = "cube";
    sky->texture[0] = "skybox";
    sky->material = def;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(0.f, 0.f, 0.f);
        _trv.push_back(tr);
    }
    _renderees.emplace_back(sky);

    auto body = std::make_shared<directx_renderer::renderee>();
    body->name = "body";
    body->type = directx_renderer::renderee_type::opaque;
    body->geometry = "maleBodymesh";
    body->texture[0] = "male_diffuse";
    body->texture[1] = "male_normal";
    body->material = rough;
//        body->tr.scale = Vector3(.1f, .1f, .1f);
//        body->tr.position = Vector3(6.f, 12.f, 0.f);
//        _renderees.emplace_back(body);

    auto house = std::make_shared<directx_renderer::renderee>();
    house->name = "house";
    house->type = directx_renderer::renderee_type::opaque;
    house->geometry = "houseCube.009";
    house->texture[0] = "house_diffuse";
    house->material = rough;
//        house->tr.position = Vector3(0.f, 6.f, -5.f);
//        _renderees.emplace_back(house);

    auto dragon0 = std::make_shared<directx_renderer::renderee>();
    dragon0->name = "dragon";
    dragon0->type = directx_renderer::renderee_type::opaque_skinned;
    dragon0->geometry = "dragon0_Dragon_Mesh";
    dragon0->texture[0] = "dragon_diffuse";
    dragon0->texture[1] = "dragon_normal";
    dragon0->material = rough;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(15.f, 4.f, -5.f);
        tr.scale = Vector3(0.25f, 0.25f, 0.25f);
        _trv.push_back(tr);
    }
    dragon0->skin_matrix = 0;
    _renderees.emplace_back(dragon0);

    auto dragon1 = std::make_shared<directx_renderer::renderee>();
    dragon1->name = "dragon";
    dragon1->type = directx_renderer::renderee_type::opaque_skinned;
    dragon1->geometry = "dragon1_Dragon_Mesh";
    dragon1->texture[0] = "dragon_diffuse";
    dragon1->texture[1] = "dragon_normal";
    dragon1->material = rough;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(15.f, 4.f, -5.f);
        tr.scale = Vector3(0.25f, 0.25f, 0.25f);
        _trv.push_back(tr);
    }
    dragon1->skin_matrix = 0;
    _renderees.emplace_back(dragon1);

    auto dragon2 = std::make_shared<directx_renderer::renderee>();
    dragon2->name = "dragon";
    dragon2->type = directx_renderer::renderee_type::opaque_skinned;
    dragon2->geometry = "dragon2_Dragon_Mesh";
    dragon2->texture[0] = "dragon_diffuse";
    dragon2->texture[1] = "dragon_normal";
    dragon2->material = rough;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(15.f, 4.f, -5.f);
        tr.scale = Vector3(0.25f, 0.25f, 0.25f);
        _trv.push_back(tr);
    }
    dragon2->skin_matrix = 0;
    _renderees.emplace_back(dragon2);

    auto mirror = std::make_shared<directx_renderer::renderee>();
    mirror->name = "mirror";
    mirror->type = directx_renderer::renderee_type::mirror;
    mirror->geometry = "mirror";
    mirror->texture[0] = "white";
    mirror->texture[1] = "default_normal";
    mirror->material = glass;
    {
        directx_renderer::transform tr;
        tr.position = Vector3(-40.f, 20.f, -20.f);
        tr.scale = Vector3(1.f, 1.f, 1.f);
        tr.rotation.y = -DirectX::XM_PI/2.f;
        _trv.push_back(tr);
        auto p = Plane(tr.position, Vector3::Right);
        _frame_globals.reflection_matrices[0] = Matrix::CreateReflection(p);
        _frame_globals.ref_cnt = 0;
    }
    mirror->skin_matrix = 0;
    _renderees.emplace_back(mirror);
}

void default_test::load_geometries() {
    std::vector<directx_renderer::geometry<directx_renderer::vertex_billboard>> geo1;
    geo1.resize(1);
    geo1[0].indices.resize(1);
    geo1[0].names.push_back("billboard");
    geo1[0].vertices = {{Vector3(0.0f, 0.0f, 0.f),   Vector2(6.f, 6.f)},
                        {Vector3(6.0f, 0.0f, 0.f),   Vector2(6.f, 6.f)},
                        {Vector3(12.0f, 0.0f, 0.f),  Vector2(6.f, 6.f)},
                        {Vector3(0.0f, 0.0f, 6.f),   Vector2(6.f, 6.f)},
                        {Vector3(6.0f, 0.0f, 6.f),   Vector2(6.f, 6.f)},
                        {Vector3(12.0f, 0.0f, 6.f),  Vector2(6.f, 6.f)},
                        {Vector3(0.0f, 0.0f, 12.f),  Vector2(6.f, 6.f)},
                        {Vector3(6.0f, 0.0f, 12.f),  Vector2(6.f, 6.f)},
                        {Vector3(12.0f, 0.0f, 12.f), Vector2(6.f, 6.f)}};
    geo1[0].indices[0].emplace_back(0);
    geo1[0].indices[0].emplace_back(1);
    geo1[0].indices[0].emplace_back(2);
    geo1[0].indices[0].emplace_back(3);
    geo1[0].indices[0].emplace_back(4);
    geo1[0].indices[0].emplace_back(5);
    geo1[0].indices[0].emplace_back(6);
    geo1[0].indices[0].emplace_back(7);
    geo1[0].indices[0].emplace_back(8);

    _renderer.init_geometries<directx_renderer::vertex>(_geometries);
    _renderer.init_geometries<directx_renderer::vertex_billboard>(geo1);
}

void default_test::load_materials() {
    _renderer.load_material({"default", "metal", "rough", "glass", "terrain"},
                            {{Vector4(.5f, .5f, .5f, 1.f),
                                     Vector3(0.5f, 0.5f, 0.5f),      .5f},
                             {Vector4(.5f, .5f, .5f, 1.f),
                                     Vector3(0.9f, 0.9f, 0.9f),      .1f},
                             {Vector4(.5f, .5f, .5f, 1.f),
                                     Vector3(0.1f, 0.1f, 0.1f),      .9f},
                             {Vector4(.5f, .5f, .5f, .5f),
                                     Vector3(0.5f, 0.5f, 0.5f),      .1f},
                             {Vector4(.5f, .5f, .5f, 1.f),
                                     Vector3(0.001f, 0.001, 0.001f), .99f}});
}

void default_test::load_textures() {
    _renderer.load_texture("kyaru", L"resource\\kyaru.png");
    _renderer.load_texture("white", L"resource\\white.png");
    _renderer.load_texture("ground", L"resource\\ground_color.jpg");
    _renderer.load_texture("wire", L"resource\\WireFence.dds");
    _renderer.load_texture("tree_arr", L"resource\\treeArray2.dds");
    _renderer.load_texture("terrain_d", L"resource\\terrain_d.png");
    _renderer.load_texture("terrain_h", L"resource\\terrain_h.png");
    _renderer.load_texture("skybox", L"resource\\skybox.dds");
    _renderer.load_texture("house_diffuse", L"resource\\house_diffuse.png");
    _renderer.load_texture("house_normal", L"resource\\house_normal.png");
    _renderer.load_texture("house_roughness", L"resource\\house_roughness.png");
    _renderer.load_texture("male_diffuse", L"resource\\male_diffuse.png");
    _renderer.load_texture("male_normal", L"resource\\male_normal.png");
    _renderer.load_texture("marble_diffuse", L"resource\\marble_diffuse.jpg");
    _renderer.load_texture("marble_normal", L"resource\\marble_normal.jpg");
    _renderer.load_texture("default_normal", L"resource\\default_normal.png");
    _renderer.load_texture("dragon_diffuse", L"resource\\dragon_diffuse.jpg");
    _renderer.load_texture("dragon_normal", L"resource\\dragon_normal.jpg");
}

void default_test::handle_input(float delta) {
    const float speed_c = 20.f;
    const float rot_c = .2f;

    int dx, dy;
    dx = _input->mouse_delta().first;
    dy = _input->mouse_delta().second;
    _camera.tr.rotation.y += (float) dx * rot_c * delta;
    _camera.tr.rotation.x += (float) dy * rot_c * delta;
    _camera.tr.rotation.x = std::clamp(_camera.tr.rotation.x,
                                       -DirectX::XM_PI / 2.f,
                                       DirectX::XM_PI / 2.f);


    if (_input->GetButton(KEY_TYPE::W)) {
        _camera.tr.position +=
                look_vector(_camera) * speed_c * delta;
    }

    if (_input->GetButton(KEY_TYPE::S)) {
        _camera.tr.position -=
                look_vector(_camera) * speed_c * delta;
    }

    if (_input->GetButton(KEY_TYPE::A)) {
        _camera.tr.position -=
                right_vector(_camera) * speed_c * delta;
    }

    if (_input->GetButton(KEY_TYPE::D)) {
        _camera.tr.position +=
                right_vector(_camera) * speed_c * delta;
    }

    if (_input->GetButtonUp(KEY_TYPE::Q)) {
        _blur = !_blur;
    }
}
