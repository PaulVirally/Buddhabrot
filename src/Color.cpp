#include <algorithm>
#include <cmath>
#include <png.h>
#include "Color.hpp"

Color::Color(png_byte r, png_byte g, png_byte b) {
    this->r = r;
    this->g = g;
    this->b = b;
}

Color& Color::operator+=(const Color &rhs) {
    if (255 - this->r >= rhs.r) this->r += rhs.r;
    if (255 - this->g >= rhs.g) this->g += rhs.g;
    if (255 - this->b >= rhs.b) this->b += rhs.b;
    return *this;
}

ColorHSV::ColorHSV(int h, float s, float v) {
    this->h = h;
    this->s = s;
    this->v = v;
}

ColorHSV::operator Color() const {
    float kr = std::fmodf(5 + h/60, 6);
    float kg = std::fmodf(3 + h/60, 6);
    float kb = std::fmodf(1 + h/60, 6);
    png_byte r = static_cast<png_byte>(255.f * (v - v*s * std::max(std::min(kr, std::min(4-kr, 1.f)), 0.f)));
    png_byte g = static_cast<png_byte>(255.f * (v - v*s * std::max(std::min(kg, std::min(4-kg, 1.f)), 0.f)));
    png_byte b = static_cast<png_byte>(255.f * (v - v*s * std::max(std::min(kb, std::min(4-kb, 1.f)), 0.f)));
    return Color(r, g, b);
}