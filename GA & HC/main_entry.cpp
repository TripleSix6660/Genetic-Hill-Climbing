#include "genetic/genetic.h"
#include "hill_climbing/hill_climbing.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    if (!renderer::initialize()) {
        LI_FN(MessageBoxA).safe()(NULL, encrypt("Failed to initialize renderer!"), encrypt("ERROR"), MB_ICONERROR);
        return FALSE;
    }

    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

    D2D1_BITMAP_PROPERTIES props = {
    { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED },
    96.f, 96.f
    };

    HRESULT hr = renderer::d2d1::render_target->CreateBitmap(
        D2D1::SizeU(globals::image_width, globals::image_height), nullptr, 0, &props, &globals::current_color_bitmap);
    ASSERT(SUCCEEDED(hr), encrypt("Failed to create bitmap."));

    hr = renderer::d2d1::render_target->CreateBitmap(
        D2D1::SizeU(globals::image_width, globals::image_height), nullptr, 0, &props, &globals::current_gray_scale_bitmap);
    ASSERT(SUCCEEDED(hr), encrypt("Failed to create bitmap."));

    hr = renderer::d2d1::render_target->CreateBitmap(
        D2D1::SizeU(globals::image_width, globals::image_height), nullptr, 0, &props, &globals::current_binary_bitmap);
    ASSERT(SUCCEEDED(hr), encrypt("Failed to create bitmap."));

    hr = renderer::d2d1::render_target->CreateBitmap(
        D2D1::SizeU(globals::image_width, globals::image_height),
        globals::image_byte_code_array, globals::image_width * 4, &props, &globals::goal_bitmap);
    ASSERT(SUCCEEDED(hr), encrypt("Failed to create goal bitmap."));

	genetic_algorithm::initialize();
    hill_climbing::initialize();

    globals::goal_bitmap->Release();
    globals::current_color_bitmap->Release();
    globals::current_gray_scale_bitmap->Release();
    globals::current_binary_bitmap->Release();

    globals::goal_bitmap = nullptr;
    globals::current_color_bitmap = nullptr;
    globals::current_gray_scale_bitmap = nullptr;
    globals::current_binary_bitmap = nullptr;

    renderer::shutdown();
    return 0;
}