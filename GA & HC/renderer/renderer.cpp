#include "renderer.h"
#include <ShellScalingApi.h>

#pragma comment(lib, "Shcore.lib")

LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}

bool renderer::d2d1::initialize() {
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
        ASSERT(false, encrypt("Failed to initialize COM"));
    }

    D2D1_FACTORY_OPTIONS options = {};
#ifdef _DEBUG
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, &d2d1_factory))) {
        ASSERT(false, encrypt("Failed to create D2D1 factory"));
    }

    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&dwrite_factory)))) {
        ASSERT(false, encrypt("Failed to create DWrite factory"));
    }

    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wic_factory)))) {
        ASSERT(false, encrypt("Failed to create WIC factory"));
    }

    RECT rect;
    GetClientRect(renderer::overlay::hwnd, &rect);

    D2D1_RENDER_TARGET_PROPERTIES rt_props = D2D1::RenderTargetProperties(
        D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        0, 0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT
    );

    D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_props = D2D1::HwndRenderTargetProperties(
        renderer::overlay::hwnd,
        D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top),
        D2D1_PRESENT_OPTIONS_IMMEDIATELY
    );

    if (FAILED(d2d1_factory->CreateHwndRenderTarget(&rt_props, &hwnd_props, reinterpret_cast<ID2D1HwndRenderTarget**>(&render_target)))) {
        ASSERT(false, encrypt("Failed to create render target"));
    }

    UINT dpi_x = 0, dpi_y = 0;
    renderer::overlay::dpi_x = 96.0f, renderer::overlay::dpi_y = 96.0f;
    HMONITOR hMonitor = LI_FN(MonitorFromWindow)(LI_FN(GetActiveWindow)(), MONITOR_DEFAULTTONEAREST);
    if (hMonitor) {
        UINT dpi_int_x, dpi_int_y;
        if (SUCCEEDED(LI_FN(GetDpiForMonitor)(hMonitor, MDT_EFFECTIVE_DPI, &dpi_int_x, &dpi_int_y))) {
            renderer::overlay::dpi_x = static_cast<float>(dpi_int_x);
            renderer::overlay::dpi_y = static_cast<float>(dpi_int_y);
        }
    }

    render_target->SetDpi(renderer::overlay::dpi_x, renderer::overlay::dpi_y);
    renderer::overlay::dpi_x /= 96.f;
    renderer::overlay::dpi_y /= 96.f;

    return true;
}

bool renderer::initialize() {
    WNDCLASSEXW wc = { sizeof(wc) };
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = encrypt(L"AI_Project_Overlay");
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    ATOM atom = RegisterClassExW(&wc);

	ASSERT(atom != 0, encrypt("Failed to register window class!"));

    int sx = GetSystemMetrics(SM_CXSCREEN);
    int sy = GetSystemMetrics(SM_CYSCREEN);

    overlay::hwnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        wc.lpszClassName,
        encrypt(L"AI_Project_Overlay"),
        WS_POPUP,
        0, 0, sx, sy,
        nullptr, nullptr, wc.hInstance, nullptr
    );

	ASSERT(overlay::hwnd != nullptr, encrypt("Failed to create overlay window!"));

    SetLayeredWindowAttributes(overlay::hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(overlay::hwnd, &margins);

    ShowWindow(overlay::hwnd, SW_SHOW);
    UpdateWindow(overlay::hwnd);

    return renderer::d2d1::initialize();
}


bool renderer::d2d1::shutdown() {
    renderer::colors::shutdown();
    renderer::fonts::shutdown();
    dwrite_factory->Release();
    render_target->Release();
    d2d1_factory->Release();
    wic_factory->Release();
    renderer::items::text_cache.clear();
    return true;
}

void renderer::shutdown() {
    if (!renderer::d2d1::shutdown()) {
        MessageBoxA(NULL, encrypt("Failed to shutdown renderer!"), encrypt("ERROR"), MB_ICONERROR);
    }
}

ID2D1SolidColorBrush* renderer::colors::get(uint32_t color) {
    auto it = colors_map.find(color);
    if (it != colors_map.end())
        return it->second;

    ID2D1SolidColorBrush* brush = nullptr;
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;

    HRESULT hr = d2d1::render_target->CreateSolidColorBrush(D2D1::ColorF(r, g, b, a), &brush);
    if (SUCCEEDED(hr)) {
        colors_map.emplace(color, brush);
        return brush;
    }

    return nullptr;
}

bool renderer::colors::shutdown() {
    for (auto&& color : colors_map)
        color.second->Release();
    return true;
}

IDWriteTextFormat* renderer::fonts::get(const char* name, const float size, const DWRITE_FONT_WEIGHT weight) {
    char _id[256];
    sprintf_s(_id, encrypt("%s_%f_%d"), name, size, static_cast<int>(weight));
    const uint32_t id = hash::fnv1<uint32_t>::hash(_id);

    if (fonts_map.find(id) != fonts_map.end())
        return fonts_map.at(id);

    size_t font_name_size;
    wchar_t font_name[100];
    mbstowcs_s(&font_name_size, font_name, name, strlen(name));

    fonts_map[id] = NULL;
    d2d1::dwrite_factory->CreateTextFormat(font_name, NULL, weight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, encrypt(L"en-us"), &fonts_map.at(id));
    return fonts_map[id];
}

bool renderer::fonts::shutdown() {
    for (auto&& font : fonts_map)
        font.second->Release();
    return true;
}
