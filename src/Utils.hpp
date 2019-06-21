#ifndef UTILS_HPP
#define UTILS_HPP

#include <png.h>
#include <string>
#include <vector>
#include "Color.hpp"

void write_png(const std::string &filename, const size_t width, const size_t height, std::vector<Color> &buffer);

template<class ForwardIt, class T>
ForwardIt binary_search_elem(ForwardIt first, ForwardIt last, T &value);

#endif /* UTILS_HPP */