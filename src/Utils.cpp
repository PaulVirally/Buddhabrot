#include <algorithm>
#include <stdio.h>
#include "Utils.hpp"

void write_png(const std::string &filename, const size_t width, const size_t height, std::vector<Color> &buffer) {	
	// Open file for writing (binary mode)
	FILE* fp = fopen(filename.c_str(), "wb");
	if (fp == nullptr) {
		fprintf(stderr, "Could not open file %s for writing\n", filename.c_str());
        return;
	}

	// Initialize write structure
	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (png_ptr == nullptr) {
		fprintf(stderr, "Could not allocate write struct\n");
        fclose(fp);
        return;
	}

	// Initialize info structure
	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == nullptr) {
		fprintf(stderr, "Could not allocate info struct\n");
        fclose(fp);
        png_destroy_write_struct(&png_ptr, nullptr);
        return;
	}
	png_init_io(png_ptr, fp);

	// Write header (8 bit colour depth)
	png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(png_ptr, info_ptr);

	// Write image data
    std::vector<png_byte> row(3 * width * sizeof(png_byte), 0); // Allocate memory for one row (3 bytes per pixel - RGB)
	for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            row[x*3 + 0] = buffer[y*width + x].r;
            row[x*3 + 1] = buffer[y*width + x].g;
            row[x*3 + 2] = buffer[y*width + x].b;
        }
		png_write_row(png_ptr, &(row[0]));
	}
	png_write_end(png_ptr, nullptr);

	if (fp != nullptr) fclose(fp);
	if (info_ptr != nullptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
	if (png_ptr != nullptr) png_destroy_write_struct(&png_ptr, nullptr);
}

template<class ForwardIt, class T>
ForwardIt binary_search_elem(ForwardIt first, ForwardIt last, T &value) {
	ForwardIt it = std::lower_bound(first, last, val);
	if (it != last && *(value < *it)) {
		return it
	}
	return last;
}