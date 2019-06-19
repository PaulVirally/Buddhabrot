#include <cmath>
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
const unsigned num_xs = static_cast<unsigned>((max_x - min_x)/x_skip);
const unsigned num_ys = static_cast<unsigned>((max_y - min_y)/y_skip);
std::vector<Color> data(num_xs * num_ys * 3, Color(0, 0, 0));
std::mutex data_mutex;

const unsigned max_iterations = 500;

const unsigned num_sections = std::thread::hardware_concurrency(); // The number of cores
const unsigned section_offset = static_cast<unsigned>(num_ys/num_sections);

std::vector<float> progresses;
unsigned progress_line = num_sections-1; // Start at the bottom of the progress lines since we initialize the console with new lines
std::mutex progress_line_mutex;
float perc_update = 0.01; // Update the user every 1% change in area covered by each thread
unsigned num_update_cols = 40; // Number of columns in the progress bar

void compute_section(unsigned section) {
    unsigned area = section_offset * num_xs; // the area this thread will cover
    float delta_progress = 0;

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

            // Update the user about the progress of this thread
            progresses[section] += 1.f/static_cast<float>(area);
            delta_progress += 1.f/static_cast<float>(area);
            if (delta_progress >= perc_update) {
                std::lock_guard<std::mutex> lock(progress_line_mutex);

                int delta = static_cast<int>(progress_line) - static_cast<int>(section);
                if (delta > 0) {
                    std::cout << "\033[" << delta << "A\r\033[K"; // Move the cursor to the right line and clear it
                }
                else if (delta < 0) {
                    std::cout << "\033[" << -delta << "B\r\033[K"; // Move the cursor to the right line and clear it
                }
                else {
                    std::cout << "\r\033[K";
                }

                std::cout << "Section " << section + 1 << ": ";
                std::cout << "[";
                unsigned covered = static_cast<unsigned>(std::round(progresses[section]*num_update_cols));
                for (unsigned i = 0; i < covered; ++i) {
                    std::cout << "#";
                }
                for (unsigned i = 0; i < num_update_cols - covered; ++i) {
                    std::cout << " ";
                }
                std::cout << "] (" << static_cast<int>(std::round(100 * progresses[section])) << "% done)" << std::flush;

                progress_line = section;
                delta_progress = 0;
            }
        }
    }
}

int main() {

    std::cout << "\033[?25l"; // Hide the cursor

    progresses = std::vector<float>(num_sections, 0);
    for (unsigned i = 0; i < num_sections-1; ++i) {
        std::cout << "\n";
    }

    // Do the computations
    std::thread threads[4];
    for (unsigned i = 0; i < num_sections; ++i) {
        threads[i] = std::thread(compute_section, i);
    }

    // Wait for everything to be finished;
    for (unsigned i = 0; i < num_sections; ++i) {
        threads[i].join();
    }

    int delta = static_cast<int>(num_sections) - static_cast<int>(progress_line);
    if (delta != 0) {
        std::cout << "\033[" << delta << "B"; // Move the cursor to the bottom
    }
    std::cout << "\r\033[?25h" << std::flush; // Show the cursor again

    // Save the image
    write_png("test.png", num_xs, num_ys, data);
    system("open test.png");

    return 0;
}