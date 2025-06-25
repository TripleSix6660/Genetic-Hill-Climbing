#include "renderer.h"
#include <wrl/client.h>
using namespace Microsoft::WRL;

class CustomTextRenderer : public IDWriteTextRenderer {
public:
    CustomTextRenderer(ID2D1GeometrySink* sink) : m_sink(sink) {}

    HRESULT DrawGlyphRun(void*, FLOAT baselineOriginX, FLOAT baselineOriginY, DWRITE_MEASURING_MODE measuringMode, DWRITE_GLYPH_RUN const* glyphRun, DWRITE_GLYPH_RUN_DESCRIPTION const*, IUnknown*) override {
        if (!glyphRun || glyphRun->glyphCount == 0) return E_INVALIDARG;
        if (!m_sink) return E_POINTER;

        HRESULT hr = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize, glyphRun->glyphIndices, glyphRun->glyphAdvances,
            glyphRun->glyphOffsets, glyphRun->glyphCount, glyphRun->isSideways,
            glyphRun->bidiLevel % 2, m_sink);

        if (FAILED(hr)) return hr;
        return m_sink->Close();
    }
    HRESULT DrawUnderline(void*, FLOAT, FLOAT, DWRITE_UNDERLINE const*, IUnknown*) override { return S_OK; }
    HRESULT DrawStrikethrough(void*, FLOAT, FLOAT, DWRITE_STRIKETHROUGH const*, IUnknown*) override { return S_OK; }
    HRESULT DrawInlineObject(void*, FLOAT, FLOAT, IDWriteInlineObject*, BOOL, BOOL, IUnknown*) override { return S_OK; }
    HRESULT IsPixelSnappingDisabled(void*, BOOL* isDisabled) override { *isDisabled = FALSE; return S_OK; }
    HRESULT GetCurrentTransform(void*, DWRITE_MATRIX* transform) override { *transform = { 1, 0, 0, 1, 0, 0 }; return S_OK; }
    HRESULT GetPixelsPerDip(void*, FLOAT* pixelsPerDip) override { *pixelsPerDip = 1.0f; return S_OK; }
    HRESULT QueryInterface(REFIID riid, void** ppvObject) override {
        if (riid == __uuidof(IDWriteTextRenderer)) { *ppvObject = this; AddRef(); return S_OK; }
        return E_NOINTERFACE;
    }
    ULONG AddRef() override { return 1; }
    ULONG Release() override { return 1; }

private:
    ID2D1GeometrySink* m_sink;
};

void renderer::items::render_text(IDWriteTextFormat* font, const vector2 position, const wchar_t* text, const color text_color, float outline_thickness, const color outline_color) {
    
    ComPtr<IDWriteTextLayout> text_layout;
    HRESULT hr = renderer::d2d1::dwrite_factory->CreateTextLayout(text, wcslen(text), font, FLT_MAX, FLT_MAX, &text_layout);
    if (FAILED(hr)) return;

    vector2 scaled_pos = { position.x , position.y  };
    ComPtr<ID2D1PathGeometry> path_geometry;
    hr = renderer::d2d1::d2d1_factory->CreatePathGeometry(&path_geometry);
    if (FAILED(hr)) return;

    ComPtr<ID2D1GeometrySink> geometry_sink;
    hr = path_geometry->Open(&geometry_sink);
    if (FAILED(hr)) return;

    CustomTextRenderer text_renderer(geometry_sink.Get());
    hr = text_layout->Draw(nullptr, &text_renderer, 0, 0);
    if (FAILED(hr)) return;

    d2d1::render_target->SetTransform(D2D1::Matrix3x2F::Translation(scaled_pos.x, scaled_pos.y));

    d2d1::render_target->DrawGeometry(path_geometry.Get(), GET_COLOR(outline_color), outline_thickness);

    d2d1::render_target->FillGeometry(path_geometry.Get(), GET_COLOR(text_color));

    d2d1::render_target->SetTransform(D2D1::Matrix3x2F::Identity());
}

void renderer::items::render_rectangle(const vector2 start_position, const vector2 end_position, const color _color, const float thickness, const float rounding) {
    
    const D2D1_ROUNDED_RECT rect = {
        D2D1::RectF(start_position.x , start_position.y , end_position.x , end_position.y ),
        rounding , rounding 
    };
    d2d1::render_target->DrawRoundedRectangle(&rect, GET_COLOR(_color), thickness);
}

void renderer::items::render_rectangle_filled(const vector2 start_position, const vector2 end_position, const color _color, const float rounding) {
    
    const D2D1_ROUNDED_RECT rect = {
        D2D1::RectF(start_position.x , start_position.y , end_position.x , end_position.y ),
        rounding , rounding 
    };
    d2d1::render_target->FillRoundedRectangle(&rect, GET_COLOR(_color));
}

void renderer::items::render_line(const vector2 start_position, const vector2 end_position, const color _color, const float width) {
    
    const D2D1_POINT_2F point0 = D2D1::Point2F(start_position.x , start_position.y );
    const D2D1_POINT_2F point1 = D2D1::Point2F(end_position.x , end_position.y );
    d2d1::render_target->DrawLine(point0, point1, GET_COLOR(_color), width );
}

void renderer::items::render_circle(const vector2 position, const color _color, const float radius, const float thickness) {
    
    const D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(position.x , position.y ), radius , radius );
    d2d1::render_target->DrawEllipse(ellipse, GET_COLOR(_color), thickness );
}

void renderer::items::render_circle_filled(const vector2 position, const color _color, const float radius) {
    
    const D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(position.x , position.y ), radius , radius );
    d2d1::render_target->FillEllipse(ellipse, GET_COLOR(_color));
}

void renderer::items::render_triangle(const vector2 point1, const vector2 point2, const vector2 point3, const color _color, const float thickness) {
    ID2D1PathGeometry* triangle_geometry = nullptr;
    d2d1::d2d1_factory->CreatePathGeometry(&triangle_geometry);

    if (!triangle_geometry) return;

    ID2D1GeometrySink* sink = nullptr;
    triangle_geometry->Open(&sink);

    if (sink) {
        sink->BeginFigure(D2D1::Point2F(point1.x, point1.y), D2D1_FIGURE_BEGIN_HOLLOW);
        sink->AddLine(D2D1::Point2F(point2.x, point2.y));
        sink->AddLine(D2D1::Point2F(point3.x, point3.y));
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
        sink->Release();
    }

    d2d1::render_target->DrawGeometry(triangle_geometry, GET_COLOR(_color), thickness);
    triangle_geometry->Release();
}

void renderer::items::render_triangle_filled(const vector2 point1, const vector2 point2, const vector2 point3, const color _color) {
    ID2D1PathGeometry* triangle_geometry = nullptr;
    d2d1::d2d1_factory->CreatePathGeometry(&triangle_geometry);

    if (!triangle_geometry) return;

    ID2D1GeometrySink* sink = nullptr;
    triangle_geometry->Open(&sink);

    if (sink) {
        sink->BeginFigure(D2D1::Point2F(point1.x, point1.y), D2D1_FIGURE_BEGIN_FILLED);
        sink->AddLine(D2D1::Point2F(point2.x, point2.y));
        sink->AddLine(D2D1::Point2F(point3.x, point3.y));
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();
        sink->Release();
    }

    d2d1::render_target->FillGeometry(triangle_geometry, GET_COLOR(_color));
    triangle_geometry->Release();
}

ID2D1Bitmap* renderer::items::create_bitmap_from_memory(const void* data, size_t size) {
    if (!data || size == 0) return nullptr;

    IWICStream* stream = nullptr;
    HRESULT hr = d2d1::wic_factory->CreateStream(&stream);

    if (FAILED(hr)) return nullptr;

    hr = stream->InitializeFromMemory(static_cast<BYTE*>(const_cast<void*>(data)), static_cast<DWORD>(size));
    if (FAILED(hr)) {
        stream->Release();
        return nullptr;
    }

    IWICBitmapDecoder* decoder = nullptr;
    hr = d2d1::wic_factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnDemand, &decoder);
    stream->Release();
    if (FAILED(hr)) return nullptr;

    IWICBitmapFrameDecode* frame = nullptr;
    hr = decoder->GetFrame(0, &frame);
    decoder->Release();
    if (FAILED(hr)) return nullptr;

    IWICFormatConverter* converter = nullptr;
    hr = d2d1::wic_factory->CreateFormatConverter(&converter);
    if (FAILED(hr)) return nullptr;

    hr = converter->Initialize(frame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    frame->Release();
    if (FAILED(hr)) return nullptr;

    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(D2D1::PixelFormat(), overlay::dpi_x * 96.f, overlay::dpi_y * 96.f);
    ID2D1Bitmap* bitmap = nullptr;
    hr = d2d1::render_target->CreateBitmapFromWicBitmap(converter, &props, &bitmap);
    converter->Release();
    if (FAILED(hr)) return nullptr;

    return bitmap;
}

void renderer::items::render_bitmap(ID2D1Bitmap* bitmap, vector2 start_position, vector2 end_position, const float opacity) {
    if (!bitmap) return;
    D2D1_RECT_F rect = D2D1::RectF(start_position.x , start_position.y , end_position.x , end_position.y );
    d2d1::render_target->DrawBitmap(bitmap, rect, opacity);
}

vector2 renderer::items::measure_text(const wchar_t* text, IDWriteTextFormat* font) {
    if (!text || !font) return { 0, 0 };

    std::wstring key = text;
    if (text_cache.find(key) != text_cache.end()) {
        return text_cache[key];
    }

    IDWriteTextLayout* text_layout = nullptr;
    HRESULT hr = renderer::d2d1::dwrite_factory->CreateTextLayout(
        text, wcslen(text), font, FLT_MAX, FLT_MAX, &text_layout);

    if (FAILED(hr) || !text_layout) return { 0, 0 };

    DWRITE_TEXT_METRICS metrics;
    text_layout->GetMetrics(&metrics);
    text_layout->Release();

    vector2 result = { std::ceil(metrics.width), std::ceil(metrics.height) };
    text_cache[key] = result;
    return result;
}

vector2 renderer::items::get_viewport_size(IDXGISwapChain* swap_chain) {
    if (!swap_chain) {
        return { 0.0f, 0.0f };
    }

    DXGI_SWAP_CHAIN_DESC swap_desc;
    HRESULT hr = swap_chain->GetDesc(&swap_desc);

    if (FAILED(hr)) {
        return { 0.0f, 0.0f };
    }

    return vector2(static_cast<float>(swap_desc.BufferDesc.Width), static_cast<float>(swap_desc.BufferDesc.Height));
}