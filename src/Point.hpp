#ifndef POINT_HPP
#define POINT_HPP

struct Point {
    size_t y; // y first should lower cache misses when we sort a Point array and access a Point array (I think)
    size_t x;

    Point(size_t x, size_t y);
};

#endif /* POINT_HPP */