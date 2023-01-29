#include "Input.h"

Input *Input::instance() {
    static Input instance;
    return &instance;
}

void Input::Init(HWND hwnd) {
    _hwnd = hwnd;
    _states.resize(KEY_TYPE_COUNT, KEY_STATE::NONE);
}

void Input::Update() {
    HWND hwnd = ::GetActiveWindow();
    if (_hwnd != hwnd) {
        for (uint32_t key = 0; key < KEY_TYPE_COUNT; key++)
            _states[key] = KEY_STATE::NONE;

        return;
    }

    BYTE asciiKeys[KEY_TYPE_COUNT] = {};
    if (::GetKeyboardState(asciiKeys) == false)
        return;

    for (uint32_t key = 0; key < KEY_TYPE_COUNT; key++) {
        if (asciiKeys[key] & 0x80) {
            KEY_STATE &state = _states[key];

            if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
                state = KEY_STATE::PRESS;
            else
                state = KEY_STATE::DOWN;
        } else {
            KEY_STATE &state = _states[key];

            if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
                state = KEY_STATE::UP;
            else
                state = KEY_STATE::NONE;
        }
    }

    if (lx == -1) {
        lx = cx;
        ly = cy;
    } else {
        dx = cx - lx;
        dy = cy - ly;
        lx = cx;
        ly = cy;
    }
}

void Input::set_mouse_pos(int x, int y) {
    cx = x;
    cy = y;
}

std::pair<int, int> Input::mouse_delta() {
    return std::make_pair(dx, dy);
}
