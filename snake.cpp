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
    switch(dir){
        case Direction::Down:  return Point(0,1);
        case Direction::Up:    return Point(0,-1);
        case Direction::Left:  return Point(-1,0);
        case Direction::Right: return Point(1,0);
        case Direction::None:  return Point(0,0);
    }
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
}

void Snake::init(Direction dir)
{
    for(int i = 0;i < cmdLength;i++){
        cmd.push(Direction::None);
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
    Point oldHead = snake.back();
    snake.pop_back();
    snakeLength--;
    return oldHead;
}

