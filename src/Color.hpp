#ifndef COLOR_HPP
#define COLOR_HPP

#include <png.h>

struct Color {
    png_byte r;
    png_byte g;
    png_byte b;

    Color(png_byte r=0, png_byte g=0, png_byte b=0);

    Color& operator+=(const Color &rhs);
};

struct ColorHSV {
    int h;
    float s;
    float v;

    ColorHSV(int h=0, float s=0, float v=0);

    operator Color() const;
};

#endif /* COLOR_HPP */