#include "clean_test.h"
#include "helper.h"

using namespace directx_renderer;
using namespace DirectX::SimpleMath;

void clean_test::init(const directx_renderer::window_info &info,
                      std::shared_ptr<Input> ptr) {
    _renderer.init(info);
    _input = std::move(ptr);

    std::vector<geometry<vertex>> geometries;
    auto terrain = create_terrain(10, 4);
    terrain.names = {"mountain"};
    geometries.emplace_back(terrain);

    _renderer.load_geometries<vertex>(geometries);

    _renderer.load_texture("mountain_diffuse", L"resource\\mountain_d.png");
    _renderer.load_texture("mountain_diffuse", L"resource\\mountain_d.png");

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


    std::vector<std::shared_ptr<directx_renderer::renderee>> renderees;

    auto mountain = std::make_shared<renderee>();
    mountain->type = directx_renderer::renderee_type::terrain;
    mountain->name = "mountain";
    mountain->geometry = "mountain";
    mountain->material = 4;
    mountain->texture[0] = "mountain_diffuse";
    mountain->texture[1] = "mountain_height";

    _renderer.init_renderees(renderees);
}

void clean_test::update(float delta) {


    _renderer.update_frame(_frame_globals, _ocv, _smv);
}

void clean_test::draw() {
    _renderer.render();
}
