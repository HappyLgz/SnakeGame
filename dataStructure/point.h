#ifndef POINT_H
#define POINT_H

#include <cstdint>
#include <utility>
#include <functional>

struct Point {
    int x_ = 0;
    int y_ = 0;

    constexpr Point() = default;
    constexpr Point(int x, int y) : x_(x), y_(y) {}

    constexpr int x() const{return x_;}
    constexpr int y() const{return y_;}

    constexpr Point operator+(const Point& other) const {
        return Point(x_ + other.x_, y_ + other.y_);
    }

    constexpr Point operator-(const Point& other) const {
        return Point(x_ - other.x_, y_ - other.y_);
    }

    constexpr Point operator*(int scalar) const {
        return Point(x_ * scalar, y_ * scalar);
    }

    constexpr Point operator/(int scalar) const {
        return Point(x_ / scalar, y_ / scalar);
    }

    constexpr Point& operator+=(const Point& other) {
        x_ += other.x_;
        y_ += other.y_;
        return *this;
    }

    constexpr Point& operator-=(const Point& other) {
        x_ -= other.x_;
        y_ -= other.y_;
        return *this;
    }

    constexpr Point& operator*=(int scalar) {
        x_ *= scalar;
        y_ *= scalar;
        return *this;
    }

    constexpr Point& operator/=(int scalar) {
        x_ /= scalar;
        y_ /= scalar;
        return *this;
    }

    constexpr bool operator==(const Point& other) const {
        return x_ == other.x_ && y_ == other.y_;
    }

    constexpr bool operator!=(const Point& other) const {
        return !(*this == other);
    }

    constexpr bool operator<(const Point& other) const {
        return std::tie(x_, y_) < std::tie(other.x_, other.y_);
    }

    constexpr int manhattanLength() const {
        return std::abs(x_) + std::abs(y_);
    }

    constexpr bool isNull() const {
        return x_ == 0 && y_ == 0;
    }


    struct Hash {
        size_t operator()(const Point& p) const {
            return std::hash<int64_t>()((static_cast<int64_t>(p.x_) << 32) | p.y_);
        }
    };
};

struct PointPairHash {
    size_t operator()(const std::pair<Point, Point>& p) const {
        size_t h1 = Point::Hash()(p.first);
        size_t h2 = Point::Hash()(p.second);
        return h1 ^ (h2 << 1);
    }
};

#endif // POINT_H
