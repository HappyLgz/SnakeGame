#include "gameboard.h"
#include <algorithm>
#include <random>

GameBoard::GameBoard()
{
    entity_.fill({Entity::None,0});
    terrain_.fill({Terrain::Empty});
}

void GameBoard::init()
{
    entity_.fill({Entity::None,0});
    terrain_.fill({Terrain::Empty});
    portalPairs_.clear();
}

std::optional<Point> GameBoard::findConnectedPortal(Point portal) const
{
    for (auto &p : portalPairs_) {
        if (p.first == portal) return p.second;
        if (p.second == portal) return p.first;
    }
    return std::nullopt;
}

bool GameBoard::isCellEmpty(Point coord) const
{
    if (coord.x() < 0 || coord.x() >= cols || coord.y() < 0 || coord.y() >= rows) {
        return false;
    }

    bool isTerrainEmpty = (terrain_[coord.y()][coord.x()] == Terrain::Empty ||
                          terrain_[coord.y()][coord.x()] == Terrain::Track);
    bool isEntityNone = (entity_[coord.y()][coord.x()].type == Entity::None);
    return isTerrainEmpty && isEntityNone;
}

bool GameBoard::isWalkable(Point coord) const
{
    if (coord.x() < 0 || coord.x() >= cols || coord.y() < 0 || coord.y() >= rows)
        return false;

    auto entityOpt = getEntityCell(coord);
    if (!entityOpt) return false;

    if (entityOpt->type == Entity::Food) return true;

    if (entityOpt->type == Entity::Snake) return false;

    auto terrainOpt = getTerrainCell(coord);
    if (!terrainOpt) return false;

    return *terrainOpt != Terrain::Wall;
}


void GameBoard::makeMap(uint8_t mapId) //默认空地图
{
    switch (mapId) {
    case 1:
        for(int i = 1;i < rows;i = i + 2){
            for(int j = 1;j < cols;j = j + 2){
                terrain_[i][j] = Terrain::Wall;
            }
        }
        break;
    case 2:
        for(int i = 7;i < cols - 7;i++){
            terrain_[7][i] = Terrain::Wall;
            terrain_[rows - 8][i] = Terrain::Wall;
        }
        for(int i = 7;i < rows - 7;i++){
            terrain_[i][7] = Terrain::Wall;
            terrain_[i][cols - 8] = Terrain::Wall;
        }
        setPortals({Point((1 + cols) / 2 - 1,(1 + rows) / 2 - 1),Point((1 + cols) / 2 - 1,rows - 4)});
        break;
    case 3:
        for(int i = 0;i < cols;i++){
            terrain_[(rows - 1) / 4][i] = Terrain::Track;
            terrain_[3 * (rows - 1) / 4][i] = Terrain::Track;
        }
        for(int i = 0;i < rows;i++){
            terrain_[i][3 * (cols - 1) / 4] = Terrain::Track;
            terrain_[i][(cols - 1) / 4] = Terrain::Track;
            terrain_[i][(cols - 1) / 2] = Terrain::Wall;
        }
        setPortals({Point((cols - 1) / 4,(rows - 1) / 4),Point(3 * (cols - 1) / 4,3 * (rows - 1) / 4)});
        setPortals({Point((cols - 1) / 4,3 * (rows - 1) / 4),Point(3 * (cols - 1) / 4,(rows - 1) / 4)});
        break;
    default:
        break;
    }
}

void GameBoard::setPortals(std::pair<Point, Point> portalPair)
{
    if(portalPair.first == portalPair.second) return;
    if(setTerrainCell(portalPair.first,Terrain::Portal) &&
    setTerrainCell(portalPair.second,Terrain::Portal)){
        portalPairs_.push_back(portalPair);
    }
}

void GameBoard::clearEntitys()
{
    entity_.fill({Entity::None,0});
}

std::optional<Rect> GameBoard::findMaxEmptyRectangle() const {
    // left[y][x] 表示以 (x,y) 结尾的连续空单元格数量
    std::array<std::array<int, cols>, rows> left{};

    // 预处理 left
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (isCellEmpty(Point(x, y))) {
                left[y][x] = (x == 0) ? 1 : left[y][x - 1] + 1;
            } else {
                left[y][x] = 0;
            }
        }
    }

    int maxArea = 0;
    Rect bestRect;

    // 枚举每个格子作为矩形的右下角
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (!isCellEmpty(Point(x, y))) continue;

            int width = left[y][x];
            int height = 1;
            if (width > maxArea) {  // 单行矩形
                maxArea = width;
                bestRect = Rect(x - width + 1, y, width, height);
            }

            // 向上扩展
            for (int k = y - 1; k >= 0; --k) {
                if (!isCellEmpty(Point(x, k))) break;
                width = std::min(width, left[k][x]);
                height = y - k + 1;
                int area = width * height;
                if (area > maxArea) {
                    maxArea = area;
                    bestRect = Rect(x - width + 1, k, width, height);
                }
            }
        }
    }

    if (maxArea == 0) return std::nullopt;
    return bestRect;
}

static std::mt19937& getRandomEngine() {
    static thread_local std::mt19937 engine(std::random_device{}());
    return engine;
}

bool GameBoard::setFood()
{
    std::vector<Point> emptyPositions;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            if (isCellEmpty(Point(x, y))) {
                emptyPositions.push_back(Point(x, y));
            }
        }
    }

    if (emptyPositions.empty()) {
        return false;
    }

    std::uniform_int_distribution<size_t> dist(0, emptyPositions.size() - 1);
    size_t index = dist(getRandomEngine());
    Point newFood = emptyPositions[index];
    setEntityCell(newFood, GameBoard::Entity::Food);
    food = newFood;

    return true;
}
