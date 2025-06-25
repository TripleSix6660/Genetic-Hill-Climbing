#pragma once
#include <string>
#include <unordered_map>
#include <dwmapi.h> 
#include <d2d1.h>
#include <dwrite.h>
#include <dxgi.h>
#include <d3d11.h>
#include <wincodec.h>
#include "../utilities/fnv.h"
#include "../utilities/vector.h"
#include "../utilities/color.h"
#include "../utilities/lazy_importer.h"
#include "../utilities/encrypt.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "windowscodecs.lib")

#define ASSERT(condition, message) if (!(condition)) { LI_FN(MessageBoxA).safe()(NULL, message, encrypt("ERROR"), MB_ICONERROR);return false; }

#define GET_COLOR(color) ::renderer::colors::get(((static_cast<uint32_t>(color.a) & 0xFF) << 24) | ((static_cast<uint32_t>(color.r) & 0xFF) << 16) | ((static_cast<uint32_t>(color.g) & 0xFF) << 8) | (static_cast<uint32_t>(color.b) & 0xFF))
#define GET_FONT(font_name, font_size_float, weight) ::renderer::fonts::get(font_name, font_size_float, weight)
#define LENGTH(str) LI_FN(lstrlenW)(str)

#define COLOR_BLACK color(0, 0, 0, 255)
#define COLOR_WHITE color(255, 255, 255, 255)
#define COLOR_RED color(255, 0, 0, 255)
#define COLOR_GREEN color(0, 255, 0, 255)
#define COLOR_BLUE color(0, 0, 255, 255)

namespace renderer {
    namespace colors {
        inline std::unordered_map<uint32_t, ID2D1SolidColorBrush*> colors_map = {};
        ID2D1SolidColorBrush* get(uint32_t color);
        bool shutdown();
    }

    namespace fonts {
        inline std::unordered_map<uint32_t, IDWriteTextFormat*> fonts_map = {};
        IDWriteTextFormat* get(const char* name, const float size, const DWRITE_FONT_WEIGHT weight);
        bool shutdown();
    }

    namespace overlay {
        inline HWND hwnd;
        inline float dpi_x, dpi_y;
    }

    namespace d2d1 {
        inline ID2D1Factory* d2d1_factory = nullptr;
        inline ID2D1RenderTarget* render_target = nullptr;
        inline IDWriteFactory* dwrite_factory = nullptr;
        inline IWICImagingFactory* wic_factory = nullptr;

        bool initialize();
        bool shutdown();
    }

    namespace items {
        inline std::unordered_map<std::wstring, vector2> text_cache;
        void render_text(IDWriteTextFormat* font, const vector2 position, const wchar_t* text, const color _color, float outline = 1.f, const color outline_color = COLOR_BLACK);
        void render_rectangle(const vector2 start_position, const vector2 end_position, const color _color, const float thickness = 1.f, const float rounding = 0.f);
        void render_rectangle_filled(const vector2 start_position, const vector2 end_position, const color _color, const float rounding = 0.f);
        void render_line(const vector2 start_position, const vector2 end_position, const color _color, const float width = 1.f);
        void render_circle(const vector2 position, const color _color, const float radius, const float thickness = 1.f);
        void render_circle_filled(const vector2 position, const color _color, const float radius);
        void render_triangle(const vector2 point1, const vector2 point2, const vector2 point3, const color _color, const float thickness = 1.f);
        void render_triangle_filled(const vector2 point1, const vector2 point2, const vector2 point3, const color _color);
        ID2D1Bitmap* create_bitmap_from_memory(const void* data, size_t size);
        void render_bitmap(ID2D1Bitmap* bitmap, vector2 start_position, vector2 end_position, const float opacity);
        vector2 measure_text(const wchar_t* text, IDWriteTextFormat* font);
        vector2 get_viewport_size(IDXGISwapChain* swap_chain);
    }

    inline vector2 viewport_size;

    bool initialize();
    void shutdown();
}