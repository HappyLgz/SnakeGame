#ifndef AI_H
#define AI_H

#include <queue>
#include <vector>
#include <functional>
#include <limits>
#include "gameboard.h"
#include "snake.h"
#include "gameconstants.h"

class Ai {
public:
    explicit Ai(const GameBoard& gameBoard) : gameBoard(gameBoard) {}

    Snake::Direction getAiCmd(Point start) const;

private:
    const GameBoard& gameBoard;
    static constexpr double normalCost = static_cast<double>(GameConstants::trackSpeed) / GameConstants::normalSpeed;
    static constexpr double trackCost = 1.0;

    struct Node {
        double g;
        double h;
        double f;
        Point parent;
        Point dirFromParent;
        bool closed;
        Node() : g(std::numeric_limits<double>::max()), h(0),
                 f(std::numeric_limits<double>::max()), parent(-1, -1),
                 dirFromParent(0, 0),closed(false) {}
    };

    Snake::Direction getAStarCmd(Point start) const;
    Snake::Direction getBFSCmd(Point start) const;

    double heuristic(Point a, Point b) const;
    Point getNextPos(Point current, Point step) const;
};

#endif // AI_H
