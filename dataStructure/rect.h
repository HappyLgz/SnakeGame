#ifndef RECT_H
#define RECT_H

#include "point.h"

struct Rect {
    int x_ = 0;
    int y_ = 0;
    int width_ = 0;
    int height_ = 0;

    constexpr Rect() = default;
    constexpr Rect(int x, int y, int width, int height)
        : x_(x), y_(y), width_(width), height_(height) {}

    constexpr int left() const { return x_; }
    constexpr int right() const { return x_ + width_ - 1; }
    constexpr int top() const { return y_; }
    constexpr int bottom() const { return y_ + height_ - 1; }

    constexpr Point topLeft() const { return Point(x_, y_); }
    constexpr Point bottomRight() const { return Point(right(), bottom()); }

    constexpr int size() const { return width_ * height_; }
    constexpr bool isEmpty() const { return width_ <= 0 || height_ <= 0; }

    constexpr bool contains(const Point& p) const {
        return p.x() >= x_ && p.x() < x_ + width_ &&
               p.y() >= y_ && p.y() < y_ + height_;
    }

    constexpr int width() const { return width_; }
    constexpr int height() const { return height_; }
    constexpr int x() const {return x_;}
    constexpr int y() const {return y_;}
};

#endif // RECT_H
