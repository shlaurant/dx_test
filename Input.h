#pragma once

enum class KEY_TYPE {
    UP = VK_UP,
    DOWN = VK_DOWN,
    LEFT = VK_LEFT,
    RIGHT = VK_RIGHT,

    W = 'W',
    A = 'A',
    S = 'S',
    D = 'D',

    Q = 'Q',
    E = 'E'
};

enum class KEY_STATE {
    NONE,
    PRESS,
    DOWN,
    UP,
    END
};

enum {
    KEY_TYPE_COUNT = static_cast<int32_t>(UINT8_MAX + 1),
    KEY_STATE_COUNT = static_cast<int32_t>(KEY_STATE::END),
};

class Input {
public:
    static Input *instance();

    void Init(HWND);
    void Update();

    bool GetButton(KEY_TYPE key) { return GetState(key) == KEY_STATE::PRESS; }
    bool GetButtonDown(KEY_TYPE key) {
        return GetState(key) == KEY_STATE::DOWN;
    }
    bool GetButtonUp(KEY_TYPE key) { return GetState(key) == KEY_STATE::UP; }

    void set_mouse_pos(int x, int y);
    std::pair<int, int> mouse_delta();

private:
    inline KEY_STATE
    GetState(KEY_TYPE key) { return _states[static_cast<uint8_t>(key)]; }

private:
    HWND _hwnd;
    std::vector<KEY_STATE> _states;
    int cx, cy, lx = -1, ly = -1, dx, dy;
};
