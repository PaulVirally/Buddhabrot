#ifndef UTILS_HPP
#define UTILS_HPP

#include <png.h>
#include <string>
#include <vector>
#include "Color.hpp"

void write_png(const std::string &filename, const unsigned width, const unsigned height, std::vector<Color> &buffer);

#endif /* UTILS_HPP */