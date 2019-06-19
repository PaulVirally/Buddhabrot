#include "Color.hpp"

Color::Color(png_byte r, png_byte g, png_byte b) {
    this->r = r;
    this->g = g;
    this->b = b;
}

ColorHSV::ColorHSV(png_byte h, png_byte s, png_byte v) {
    this->h = h;
    this->s = s;
    this->v = v;
}

// ColorHSV::operator Color() const {
//     // TODO: this
// }