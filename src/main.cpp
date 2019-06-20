#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>
#include <png.h>
#include "Color.hpp"
#include "Utils.hpp"

using namespace std::chrono_literals;

const long double min_x = -2.L;
const long double max_x = 1.L;
const long double min_y = -1.125L;
const long double max_y = 1.125L;
const long double x_skip = 0.001L;
const long double y_skip = 0.001L;
const size_t num_xs = (max_x - min_x)/x_skip;
const size_t num_ys = (max_y - min_y)/y_skip;
std::vector<Color> data(num_xs * num_ys * 3, Color(0, 0, 0));
std::mutex data_mutex;

const size_t max_iterations = 500;

const size_t num_sections = std::thread::hardware_concurrency(); // The number of cores
const size_t section_offset = num_ys/num_sections;
std::atomic<size_t> sections_completed = 0;

std::vector<long double> progresses;
auto update_freq = 100ms; // Update the user every 100ms
int num_update_cols = 40; // Number of columns in the progress bar

void compute_section(size_t section) {
    long double area = static_cast<long double>(section_offset * num_xs); // the area this thread will cover
    long double delta_progress = 0;

    for (size_t y_idx = section; y_idx < num_ys; y_idx += num_sections) {
        for (size_t x_idx = 0; x_idx < num_xs; ++x_idx) {
            const long double a = x_idx*x_skip + min_x;
            const long double b = y_idx*y_skip + min_y;

            bool in_set = true;
            long double new_a = a;
            long double new_b = b;
            for (size_t i = 0; i < max_iterations; ++i) {
                long double temp_a = new_a;
                long double temp_b = new_b;
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
    while (true) {
        // Go to the top
        std::cout << "\033[" << num_sections + 1 << "A";

        for (size_t section = 0; section < num_sections; ++section) {
            std::cout << "\r\033[K"; // Clear the line
            std::cout << "Section " << section + 1 << ": [";

            int prog_bar_fill = static_cast<int>(std::round(progresses[section] * num_update_cols));
            std::cout << std::setfill('#') << std::setw(prog_bar_fill) << "";
            std::cout << std::setfill(' ') << std::setw(num_update_cols - prog_bar_fill) << "";

            std::cout << "] (" << static_cast<int>(std::round(progresses[section] * 100)) << "% done)\n";
        }
        std::cout << "\033[1mTotal:     [";

        long double total = std::accumulate(progresses.begin(), progresses.end(), 0.f);
        total /= static_cast<long double>(num_sections);

        int prog_bar_fill = static_cast<int>(std::round(total * num_update_cols));
        std::cout << std::setfill('#') << std::setw(prog_bar_fill) << "";
        std::cout << std::setfill(' ') << std::setw(num_update_cols - prog_bar_fill) << "";
        std::cout << "] (" << static_cast<int>(std::round(total * 100)) << "% done)\033[0m\n";

        std::chrono::duration<long double> duration = std::chrono::high_resolution_clock::now() - start_time;
        int hours = duration.count()/3600;
        int mins = duration.count()/60 - hours*60;
        int secs = duration.count() - mins*60 - hours*3600;

        long double least_area = *std::min_element(progresses.begin(), progresses.end());
        int time_rem = duration.count()/least_area - duration.count();
        int hours_rem = time_rem/3600;
        int mins_rem = time_rem/60 - hours_rem*60;
        int secs_rem = time_rem - mins_rem*60 - hours_rem*3600;

        std::cout << std::setfill('0') << std::setw(2) << hours << ":" << std::setw(2) << mins << ":" << std::setw(2) << secs << " ("; 
        std::cout << std::setw(2) << hours_rem << ":" << std::setw(2) << mins_rem << ":" << std::setw(2) << secs_rem << " remaining)" << std::flush;

        if (sections_completed == num_sections) {
            break;
        }
        std::this_thread::sleep_for(100ms);
    }
}

int main() {

    std::cout << "\033[?25l"; // Hide the cursor

    progresses = std::vector<long double>(num_sections, 0.L);
    for (size_t i = 0; i < num_sections+1; ++i) {
        std::cout << "\n";
    }

    // Do the computations
    std::vector<std::thread> threads;
    for (size_t i = 0; i < num_sections; ++i) {
        threads.push_back(std::thread(compute_section, i));
    }
    std::thread notify_thread(notify_user);

    // Wait for everything to be finished;
    for (auto it = threads.begin(); it != threads.end(); ++it) {
        it->join();
    }
    notify_thread.join();

    std::cout << "\r\033[?25h" << std::endl; // Show the cursor again

    // Save the image
    write_png("test.png", num_xs, num_ys, data);
    system("open test.png");

    return 0;
}