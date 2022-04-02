#include "Attributes.hpp"
#include <cmath>
#include <algorithm>

Color::Color(float r, float g, float b, float a):
    r(r), g(g), b(b), a(a)
{}

Color::Color(const HSVColor& hsv)
{
    int range = (int)floor(hsv.h / 60.0f);
    float c = hsv.v*hsv.s;
    float x = c*(1 - std::abs(fmod(hsv.h / 60.0f, 2.0f) - 1));
    float m = hsv.v - c;

    switch (range) 
    {
        case 0:
            r = (c + m);
            g = (x + m);
            b = m;
            break;
        case 1:
            r = (x + m);
            g = (c + m);
            b = m;
            break;
        case 2:
            r = m;
            g = (c + m);
            b = (x + m);
            break;
        case 3:
            r = m;
            g = (x + m);
            b = (c + m);
            break;
        case 4:
            r = (x + m);
            g = m;
            b = (c + m);
            break;
        default:		// case 5:
            r = (c + m);
            g = m;
            b = (x + m);
            break;
    }
}

HSVColor::HSVColor(float h, float s, float v, float a) :
    h(h), s(s), v(v), a(a) {  }

HSVColor::HSVColor(const Color& rgb)
{
    double min = std::min(rgb.r, std::min(rgb.g, rgb.b));
    double max = std::max(rgb.r, std::max(rgb.g, rgb.b));
    double delta = max - min;

    v = max;
    s = (max > 1e-3) ? (delta / max) : 0;
    a = rgb.a;

    if (delta == 0) 
    {
        h = 0;
    }
    else 
    {
        if (rgb.r == max) 
            h = (rgb.g - rgb.b) / delta;
        else if (rgb.g == max)
            h = 2 + (rgb.b - rgb.r) / delta;
        else if (rgb.b == max)
            h = 4 + (rgb.r - rgb.g) / delta;

        h *= 60;
        h = fmod(h + 360, 360);
    }
}