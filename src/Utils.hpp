#ifndef UTILS_HPP
#define UTILS_HPP

#include <png.h>
#include <string>
#include <vector>
#include "Color.hpp"

void write_png(const std::string &filename, const size_t width, const size_t height, std::vector<Color> &buffer);

#endif /* UTILS_HPP */