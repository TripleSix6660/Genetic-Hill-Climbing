#pragma once
struct color {
    int r = 255, g = 255, b = 255, a = 255;

    constexpr color(int r = 255, int g = 255, int b = 255, int a = 255) : r(r), g(g), b(b), a(a) {}

    static color lerp(const color& start, const color& end, float t) {
        t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;

        int r = static_cast<int>(start.r + t * (end.r - start.r));
        int g = static_cast<int>(start.g + t * (end.g - start.g));
        int b = static_cast<int>(start.b + t * (end.b - start.b));
        int a = static_cast<int>(start.a + t * (end.a - start.a));

        return color(r, g, b, a);
    }

    static color hsv_to_rgb(float h, float s, float v) {
        float c = v * s;
        float x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));
        float m = v - c;

        float r = 0, g = 0, b = 0;
        if (h >= 0 && h < 60) {
            r = c; g = x; b = 0;
        }
        else if (h >= 60 && h < 120) {
            r = x; g = c; b = 0;
        }
        else if (h >= 120 && h < 180) {
            r = 0; g = c; b = x;
        }
        else if (h >= 180 && h < 240) {
            r = 0; g = x; b = c;
        }
        else if (h >= 240 && h < 300) {
            r = x; g = 0; b = c;
        }
        else {
            r = c; g = 0; b = x;
        }

        return color((r + m) * 255, (g + m) * 255, (b + m) * 255, 255);
    }
};
