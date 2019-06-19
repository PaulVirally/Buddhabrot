cmake_minimum_required(VERSION 3.14.4)
project(Buddhabrot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Werror -Wno-unused")

file(GLOB SOURCES "src/*.cpp")
message("SOURCES = ${SOURCES}")

add_executable(Buddhabrot ${SOURCES})

target_include_directories(Buddhabrot PUBLIC /usr/local/include)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
target_link_libraries(Buddhabrot ${PNG_LIBRARY})
