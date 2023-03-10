#include "mem_leak_test.h"

void mem_leak_test::init(const directx_renderer::window_info &info,
                         std::shared_ptr<Input> ptr) {
    _input = ptr;
    _renderer.init(info);
}

void mem_leak_test::update(float delta) {
    _renderer.update_frame(_frame_globals, _ocv, _smv);
}

void mem_leak_test::draw() {
    _renderer.render();
}
