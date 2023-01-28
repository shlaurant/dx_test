#pragma once

#include "test_app.h"
#include "helper.h"

class default_test:public test_app {
public:
    void init(const directx_renderer::window_info &, std::shared_ptr<Input>) override;
    void update(float delta) override;
    void draw() override;
private:
    FBXLoader _dragon_fbx;
    std::vector<directx_renderer::transform> _trv;
    std::vector<std::shared_ptr<directx_renderer::renderee>> _renderees;
    directx_renderer::light_info _light_info;
    std::vector<directx_renderer::geometry<directx_renderer::vertex>> _geometries;
    animator _animator;

    void create_geometries();
    void create_light_info();
    void build_renderees();
    void load_geometries();
    void load_materials();
    void load_textures();
    void handle_input(float delta);
};