#include <WinUser.h>
#include <windowsx.h>

#include "directx_renderer.h"
#include "debug.h"
#include "helper.h"
#include "Input.h"
#include "GameTimer.h"
#include "FBXLoader.h"
#include "default_test.h"

using namespace DirectX::SimpleMath;

std::shared_ptr<Input> input;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

std::vector<directx_renderer::geometry<directx_renderer::vertex>>
create_geometries();
directx_renderer::light_info create_light_info();

std::vector<std::shared_ptr<directx_renderer::renderee>> build_renderees();
void load_geometries(directx_renderer::dx12_renderer &dx12);
void load_materials(directx_renderer::dx12_renderer &dx12);
void load_textures(directx_renderer::dx12_renderer &dx12);
int
WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine,
                int nCmdShow) {

    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

// Create the window.

    HWND hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            nullptr,    // Window text
            WS_OVERLAPPEDWINDOW,            // Window style

            // Size and position
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

            NULL,       // Parent window
            NULL,       // Menu
            hInstance,  // Instance handle
            NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    default_test app;

    try {
        input = std::make_shared<Input>();
        app.init({hwnd, 1920, 1080, true}, input);

        GameTimer timer;
        timer.Reset();
        timer.Start();

        while (msg.message != WM_QUIT) {
            if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {
                timer.Tick();
                std::wstring txt = L"fps: ";
                txt += std::to_wstring(1 / timer.DeltaTime());
                SetWindowText(hwnd, txt.c_str());
                app.update(timer.DeltaTime());
                app.draw();
            }
        }
    } catch (DxException &e) {
        MessageBoxExW(nullptr, e.ToString().c_str(), nullptr, MB_OK, 0);
    }

    return 0;
}

LRESULT
CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

// All painting occurs here, between BeginPaint and EndPaint.

            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));

            EndPaint(hwnd, &ps);
        }
            return 0;

        case WM_MOUSEMOVE: {
            input->set_mouse_pos(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
