#ifndef UTILS_HPP
#define UTILS_HPP

#include <png.h>
#include <string>
#include <vector>
#include "Color.hpp"
#include "Point.hpp"

void write_png(const std::string &filename, const size_t width, const size_t height, std::vector<Color> &buffer);

std::vector<Point>::iterator find_first_y(std::vector<Point>::iterator start, std::vector<Point>::iterator end, size_t y);

#endif /* UTILS_HPP */