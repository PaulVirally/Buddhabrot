#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <png.h>
#include "Color.hpp"
#include "Utils.hpp"

const float min_x = -2;
const float max_x = 1;
const float min_y = -1.125;
const float max_y = 1.125;
const float x_skip = 0.001;
const float y_skip = 0.001;
const unsigned max_iterations = 1000;
const unsigned num_sections = 4;

const unsigned num_xs = static_cast<unsigned>((max_x - min_x)/x_skip);
const unsigned num_ys = static_cast<unsigned>((max_y - min_y)/y_skip);
std::vector<Color> data(num_xs * num_ys * 3, Color(0, 0, 0));
std::mutex data_mutex;

const unsigned section_offset = static_cast<unsigned>(num_ys/num_sections);

void compute_section(unsigned section) {
    for (unsigned y_idx = section*section_offset; y_idx < (section+1)*section_offset; ++y_idx) {
        for (unsigned x_idx = 0; x_idx < num_xs; ++x_idx) {
            const float a = x_idx*x_skip + min_x;
            const float b = y_idx*y_skip + min_y;

            bool in_set = true;
            float new_a = a;
            float new_b = b;
            for (unsigned i = 0; i < max_iterations; ++i) {
                float temp_a = new_a;
                float temp_b = new_b;
                new_a = temp_a*temp_a - temp_b*temp_b + a;
                new_b = 2*temp_a*temp_b + b;

                if (new_a*new_a + new_b*new_b >= 4) {
                    in_set = false;
                    break;
                }
            }

            if (in_set) {
                std::lock_guard<std::mutex> lock(data_mutex);
                data[y_idx*num_xs + x_idx] = Color(255, 255, 255);
            }
            else {
                std::lock_guard<std::mutex> lock(data_mutex);
                data[y_idx*num_xs + x_idx] = Color(0, 0, 0);
            }
        }
    }
}

int main() {

    // Do the computations
    std::thread threads[4];
    for (unsigned i = 0; i < num_sections; ++i) {
        threads[i] = std::thread(compute_section, i);
    }

    // Wait for everything to be finished;
    for (unsigned i = 0; i < num_sections; ++i) {
        threads[i].join();
    }

    // Save the image
    write_png("test.png", num_xs, num_ys, data);
    system("open test.png");

    return 0;
}