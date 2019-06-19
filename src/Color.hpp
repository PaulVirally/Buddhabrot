#ifndef COLOR_HPP
#define COLOR_HPP

#include <png.h>

struct Color {
    png_byte r;
    png_byte g;
    png_byte b;

    Color(png_byte r, png_byte g, png_byte b);
};

struct ColorHSV {
    png_byte h;
    png_byte s;
    png_byte v;

    ColorHSV(png_byte h, png_byte s, png_byte v);

    operator Color() const;
};

#endif /* COLOR_HPP */