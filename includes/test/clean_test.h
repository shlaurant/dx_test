#pragma once

#include "test_app.h"

class clean_test : public test_app {
public:
    void init(const directx_renderer::window_info &info,
              std::shared_ptr<Input> ptr) override;
    void update(float delta) override;
    void draw() override;
private:
};