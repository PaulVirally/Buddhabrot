#include <atomic>
#include <chrono>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <thread>
#include <vector>
#include <png.h>
#include "Color.hpp"
#include "Utils.hpp"

using namespace std::chrono_literals;

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
std::atomic<unsigned> sections_completed = 0;

std::vector<float> progresses;
auto update_freq = 100ms; // Update the user every 100ms
unsigned num_update_cols = 40; // Number of columns in the progress bar

void compute_section(unsigned section) {
    float area = static_cast<float>(section_offset * num_xs); // the area this thread will cover
    float delta_progress = 0;

    for (unsigned y_idx = section; y_idx < num_ys; y_idx += num_sections) {
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

            progresses[section] += 1.f/area;
        }
    }

    sections_completed += 1;
}

void notify_user() {
    auto start_time = std::chrono::high_resolution_clock::now();
    while (sections_completed != num_sections) {
        // Go to the top
        std::cout << "\033[" << num_sections + 1 << "A";

        for (unsigned section = 0; section < num_sections; ++section) {
            std::cout << "\r\033[K"; // Clear the line
            std::cout << "Section " << section + 1 << ": [";
            unsigned covered = static_cast<unsigned>(std::round(progresses[section] * num_update_cols));
            for (unsigned i = 0; i < covered; ++i) {
                std::cout << "#";
            }
            for (unsigned i = 0; i < num_update_cols - covered; ++i) {
                std::cout << " ";
            }
            std::cout << "] (" << static_cast<unsigned>(std::round(progresses[section] * 100)) << "% done)\n";
        }
        std::cout << "\033[1mTotal:     [";
        float total = 0;
        for (auto it = progresses.begin(); it != progresses.end(); ++it) {
            total += *it;
        }
        total /= static_cast<float>(num_sections);
        unsigned covered = static_cast<unsigned>(std::round(total * num_update_cols));
        for (unsigned i = 0; i < covered; ++i) {
            std::cout << "#";
        }
        for (unsigned i = 0; i < num_update_cols - covered; ++i) {
            std::cout << " ";
        }
        std::cout << "] (" << static_cast<unsigned>(std::round(total * 100)) << "% done)\033[0m\n";

        std::chrono::duration<float> duration = std::chrono::high_resolution_clock::now() - start_time;
        int hours = duration.count()/3600;
        int mins = duration.count()/60 - hours*60;
        int secs = duration.count() - mins*60 - hours*3600;

        float least_area = 1;
        for (auto it = progresses.begin(); it != progresses.end(); ++it) {
            if (*it < least_area) {
                least_area = *it;
            }
        }
        int time_rem = duration.count()/least_area - duration.count();
        int hours_rem = time_rem/3600;
        int mins_rem = time_rem/60 - hours_rem*60;
        int secs_rem = time_rem - mins_rem*60 - hours_rem*3600;

        std::cout << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << mins << ":" << std::setw(2) << secs << " ("; 
        std::cout << std::setw(2) << hours_rem << ":" << std::setw(2) << mins_rem << ":" << std::setw(2) << secs_rem << " remaining)" << std::flush;

        std::this_thread::sleep_for(100ms);
    }
}

int main() {

    std::cout << "\033[?25l"; // Hide the cursor

    progresses = std::vector<float>(num_sections, 0);
    for (unsigned i = 0; i < num_sections+1; ++i) {
        std::cout << "\n";
    }

    // Do the computations
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < num_sections; ++i) {
        threads.push_back(std::thread(compute_section, i));
    }
    std::thread notify_thread(notify_user);

    // Wait for everything to be finished;
    for (unsigned i = 0; i < num_sections; ++i) {
        threads[i].join();
    }
    notify_thread.join();

    std::cout << "\r\033[?25h" << std::endl; // Show the cursor again

    // Save the image
    write_png("test.png", num_xs, num_ys, data);
    system("open test.png");

    return 0;
}