#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <optional>
#include <array>
#include <vector>
#include "point.h"
#include "rect.h"

class GameBoard
{
public:
    GameBoard();

    static constexpr int rows = 25;
    static constexpr int cols = 25;

    enum class Entity{None,Food,Snake};
    enum class Terrain{Empty,Wall,Portal,Track};

    struct EntityCell{
        Entity type = Entity::None;
        uint8_t ownerId = 0; // 0为无主，往上代表Snake id
    };

    bool setEntityCell(Point coord, Entity e,uint8_t ownerId = 0){
        if (coord.x() < 0 || coord.x() >= cols || coord.y() < 0 || coord.y() >= rows) {
            return false;
        }
        entity_[coord.y()][coord.x()] = {e,ownerId};
        return true;
    }
    std::optional<EntityCell> getEntityCell(Point coord) const{
        if (coord.x() < 0 || coord.x() >= cols || coord.y() < 0 || coord.y() >= rows) {
            return std::nullopt;
        }
        return entity_[coord.y()][coord.x()];
    }
    std::optional<Terrain> getTerrainCell(Point coord) const{
        if (coord.x() < 0 || coord.x() >= cols || coord.y() < 0 || coord.y() >= rows) {
            return std::nullopt;
        }
        return terrain_[coord.y()][coord.x()];
    }

    std::optional<Point> findConnectedPortal(Point portal) const;
    std::optional<Rect> findMaxEmptyRectangle() const;
    const std::array<std::array<EntityCell,cols>,rows>& entity() const{return entity_;}
    const std::array<std::array<Terrain,cols>,rows>& terrain() const{return terrain_;}
    const std::vector<std::pair<Point,Point>>& portalPairs() const{return portalPairs_;}
    bool setFood();
    bool isCellEmpty(Point coord) const;
    void makeMap(uint8_t mapId = 0);
    void init();
    void clearEntitys();

private:
    std::array<std::array<EntityCell,cols>,rows> entity_;
    std::array<std::array<Terrain,cols>,rows> terrain_;
    std::vector<std::pair<Point,Point>> portalPairs_;

    void setPortals(std::pair<Point,Point> portalPair);

    bool setTerrainCell(Point coord, Terrain t){
        if (coord.x() < 0 || coord.x() >= cols || coord.y() < 0 || coord.y() >= rows) {
            return false;
        }
        terrain_[coord.y()][coord.x()] = t;
        return true;
    }
};

#endif // GAMEBOARD_H
