#include "ai.h"

Snake::Direction Ai::getAiCmd(Point start) const {
    Snake::Direction dir = getAStarCmd(start);
    if (dir == Snake::Direction::None) {
        return getBFSCmd(start);
    }
    return dir;
}

Snake::Direction Ai::getAStarCmd(Point start) const {
    Point goal = gameBoard.getFoodPos();

    if (goal.x() < 0 || !gameBoard.isWalkable(goal)) return Snake::Direction::None;
    if (start == goal) return Snake::Direction::None;

    std::array<std::array<Node, GameBoard::cols>, GameBoard::rows> nodes{};
    nodes[start.y()][start.x()].g = 0;
    nodes[start.y()][start.x()].h = heuristic(start, goal);
    nodes[start.y()][start.x()].f = nodes[start.y()][start.x()].h;
    nodes[start.y()][start.x()].parent = Point(-1, -1);

    struct QueueElement {
        double f;
        Point pos;
        bool operator>(const QueueElement& other) const { return f > other.f; }
    };
    std::priority_queue<QueueElement, std::vector<QueueElement>, std::greater<QueueElement>> openSet;
    openSet.push({nodes[start.y()][start.x()].f, start});

    const Point dirs[4] = {Point(1,0), Point(-1,0), Point(0,1), Point(0,-1)};

    while (!openSet.empty()) {
        QueueElement current = openSet.top();
        openSet.pop();
        Point pos = current.pos;

        if (nodes[pos.y()][pos.x()].closed || current.f != nodes[pos.y()][pos.x()].f) continue;

        if (pos == goal) {
            // 回溯找到第一步
            Point stepPos = pos;
            while (true) {
                Point parent = nodes[stepPos.y()][stepPos.x()].parent;
                if (parent == start) {
                    return Snake::directionFromDelta(nodes[stepPos.y()][stepPos.x()].dirFromParent);
                }
                stepPos = parent;
            }
        }

        nodes[pos.y()][pos.x()].closed = true;

        for (const Point& dir : dirs) {

            Point nextPos = getNextPos(pos, dir);
            if (!gameBoard.isWalkable(nextPos) || nodes[nextPos.y()][nextPos.x()].closed) continue;

            // 根据地形计算代价
            double moveCost = normalCost;
            auto terrainOpt = gameBoard.getTerrainCell(nextPos);
            if (terrainOpt && *terrainOpt == GameBoard::Terrain::Track) {
                moveCost = trackCost;
            }

            double new_g = nodes[pos.y()][pos.x()].g + moveCost;
            if (new_g < nodes[nextPos.y()][nextPos.x()].g) {
                nodes[nextPos.y()][nextPos.x()].g = new_g;
                nodes[nextPos.y()][nextPos.x()].h = heuristic(nextPos, goal);
                nodes[nextPos.y()][nextPos.x()].f = new_g + nodes[nextPos.y()][nextPos.x()].h;
                nodes[nextPos.y()][nextPos.x()].parent = pos;
                nodes[nextPos.y()][nextPos.x()].dirFromParent = dir;
                openSet.push({nodes[nextPos.y()][nextPos.x()].f, nextPos});
            }
        }
    }
    return Snake::Direction::None;
}

Snake::Direction Ai::getBFSCmd(Point start) const {
    bool visited[GameBoard::rows][GameBoard::cols] = {false};
    Point parent[GameBoard::rows][GameBoard::cols];
    int dist[GameBoard::rows][GameBoard::cols] = {0};

    for(int y = 0; y < GameBoard::rows; ++y)
        for(int x = 0; x < GameBoard::cols; ++x)
            parent[y][x] = Point(-1, -1);

    std::queue<Point> q;
    q.push(start);
    visited[start.y()][start.x()] = true;

    Point furthestNode = start;
    int maxDist = 0;
    const Point dirs[4] = {Point(1,0), Point(-1,0), Point(0,1), Point(0,-1)};

    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        if (dist[curr.y()][curr.x()] > maxDist) {
            maxDist = dist[curr.y()][curr.x()];
            furthestNode = curr;
        }

        for (const Point& dir : dirs) {

            Point nextPos = getNextPos(curr, dir);
            if (gameBoard.isWalkable(nextPos) && !visited[nextPos.y()][nextPos.x()]) {
                visited[nextPos.y()][nextPos.x()] = true;
                parent[nextPos.y()][nextPos.x()] = curr;
                dist[nextPos.y()][nextPos.x()] = dist[curr.y()][curr.x()] + 1;
                q.push(nextPos);
            }
        }
    }

    if (furthestNode == start) return Snake::Direction::None;

    // 回溯寻找第一步方向
    Point stepPos = furthestNode;
    while (true) {
        Point p = parent[stepPos.y()][stepPos.x()];
        if (p == start) break;
        stepPos = p;
    }

    return Snake::directionFromDelta(stepPos - start);
}


double Ai::heuristic(Point a, Point b) const {
    return abs(a.x() - b.x()) + abs(a.y() - b.y());
}

Point Ai::getNextPos(Point current, Point step) const {
    Point next = current + step;
    int maxLoop = 10;
    while (maxLoop-- > 0) {
        auto terrainOpt = gameBoard.getTerrainCell(next);
        if (terrainOpt && *terrainOpt == GameBoard::Terrain::Portal) {
            auto connectedOpt = gameBoard.findConnectedPortal(next);
            if (connectedOpt) {
                next = *connectedOpt + step;
                continue;
            } else {
                break;
            }
        }
        break;
    }
    return next;
}
