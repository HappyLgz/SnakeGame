#include "snake.h"

Snake::Snake(uint8_t id) : id(id)
{
    init(Direction::Down);
}

void Snake::pushCmd(Direction dir)
{
    cmd.push(dir);
}

Point Snake::directionToStep(Direction dir)
{
    if(dir == Direction::Down) return Point(0,1);
    if(dir == Direction::Up) return Point(0,-1);
    if(dir == Direction::Left) return Point(-1,0);
    if(dir == Direction::Right) return Point(1,0);
    return Point(0,0);
}

Snake::Direction Snake::directionFromDelta(Point delta)
{
    if (delta.x() == 1 && delta.y() == 0) return Direction::Right;
    if (delta.x() == -1 && delta.y() == 0) return Direction::Left;
    if (delta.x() == 0 && delta.y() == 1) return Direction::Down;
    if (delta.x() == 0 && delta.y() == -1) return Direction::Up;
    return Direction::None;
}

void Snake::setSpeed(int speed)
{
    if(speed < 0){
        return;
    }
    this->speed = speed;
}

void Snake::changeSpeed(int deltaSpeed)
{
    int newSpeed = speed + deltaSpeed;
    speed = newSpeed < 0 ? 0 : newSpeed;
}

void Snake::tick(){
    actionPoints += speed;
}

bool Snake::isReady() const
{
    return actionPoints > threshold;
}

void Snake::readyMove()
{
    actionPoints -= threshold;

    if(!cmd.empty()){
        Point newStep = directionToStep(cmd.pop());
        if(newStep != Point(0,0) && step + newStep != Point(0,0)){
            step = newStep;
        }
    }
    if(dirBuffer.size() < dirLength){
        dirBuffer.push_back(step);
    }
}

void Snake::init(Direction dir)
{
    while (!cmd.empty()) {
        cmd.pop();
    }
    step = directionToStep(dir);
    actionPoints = 0;
    speed = 10;
}

int Snake::getLength() const
{
    return snakeLength;
}

Point Snake::getStep() const
{
    return step;
}

void Snake::setHead(Point head,bool popTail)
{
    snake.push_back(head);
    if(popTail){
        snake.pop_front();
        return;
    }
    snakeLength++;
}

std::optional<Point> Snake::getHead() const
{
    if(snake.empty()){
        return std::nullopt;
    }
    return snake.back();
}

std::optional<Point> Snake::getTail() const
{
    if(snake.empty()){
        return std::nullopt;
    }
    return snake.front();
}

std::optional<Point> Snake::shrink()
{
    if(snake.empty()){
        return std::nullopt;
    }
    if(!dirBuffer.empty()){
        dirBuffer.pop_back();
        step = dirBuffer.back();
    }
    Point oldHead = snake.back();
    snake.pop_back();
    snakeLength--;
    return oldHead;
}

