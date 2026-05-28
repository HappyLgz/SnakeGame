#ifndef SNAKE_H
#define SNAKE_H

#include "point.h"
#include <deque>
#include <optional>
#include "boundedqueue.h"

class Snake
{
public:
    explicit Snake(uint8_t id);

    enum class Direction {Up, Down, Left, Right,None};

    void pushCmd(Direction dir);
    void setSpeed(int speed);
    void changeSpeed(int deltaSpeed);
    void tick();
    bool isReady() const;
    void readyMove();
    void init(Direction dir);
    int getLength() const;
    Point getStep() const;
    void setHead(Point head,bool popTail);
    std::optional<Point> getHead() const;
    std::optional<Point> getTail() const;
    std::optional<Point> shrink();

    uint8_t id = 0;

private:
    static constexpr int cmdLength = 4;

//    static constexpr int dirLength = 4; //保存过去四次移动的方向

    static constexpr int threshold = 100;

    std::deque<Point> snake;
    int snakeLength = 0;

    FixedFifo<Direction,cmdLength> cmd;
    std::deque<Point> dirBuffer; //备份每次移动的方向
    Point step;

    int actionPoints = 0;

    int speed = 10;

    Point directionToStep(Direction dir);

};

#endif // SNAKE_H
