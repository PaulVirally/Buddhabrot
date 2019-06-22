#ifndef COLOR_HPP
#define COLOR_HPP

#include <png.h>

struct Color {
    png_byte r;
    png_byte g;
    png_byte b;

    Color(png_byte r, png_byte g, png_byte b);

    Color& operator+=(const Color &rhs);
};

struct ColorHSV {
    int h;
    float s;
    float v;

    ColorHSV(int h, float s, float v);

    operator Color() const;
};

#endif /* COLOR_HPP */