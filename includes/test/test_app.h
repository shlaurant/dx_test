#pragma once

#include <directx_renderer.h>
#include "Input.h"

class test_app {
public:
    virtual void init(const directx_renderer::window_info &, std::shared_ptr<Input>) = 0;
    virtual void update(float delta) = 0;
    virtual void draw() = 0;

protected:
    std::shared_ptr<Input> _input;
    directx_renderer::dx12_renderer _renderer;
    directx_renderer::camera _camera;
    directx_renderer::frame_globals _frame_globals;
    std::vector<directx_renderer::object_constant> _ocv;
    std::vector<directx_renderer::skin_matrix> _smv;
};