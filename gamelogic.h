#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <vector>
#include <array>
#include "snake.h"
#include "gameboard.h"

class GameLogic
{
public:
    explicit GameLogic(uint8_t playersNum = 1,uint8_t mapId = 0);
    GameLogic(const GameLogic&) = delete;
    GameLogic& operator=(const GameLogic&) = delete;
    ~GameLogic();

    bool setPlayersNum(uint8_t num);
    void setMap(uint8_t id);
    void pushCmd(Snake::Direction dir,uint8_t playerId);

    enum class GameState {Idle,Playing,Paused,Done};
    GameState state() const { return currentState; }

    void startGame();
    void pauseGame();
    void resumeGame();
    void restartGame();
    void togglePause();
    void update();

    struct BoardData{
        const int rows = GameBoard::rows;
        const int cols = GameBoard::cols;
        const std::array<std::array<GameBoard::EntityCell,GameBoard::cols>,GameBoard::rows> &entity;
        const std::array<std::array<GameBoard::Terrain,GameBoard::cols>,GameBoard::rows> &terrain;
        const std::vector<std::pair<Point,Point>> &portalPairs;
        std::vector<Point> snakeHeads;
        explicit BoardData(const std::array<std::array<GameBoard::EntityCell,GameBoard::cols>,GameBoard::rows> &entity,
                           const std::array<std::array<GameBoard::Terrain,GameBoard::cols>,GameBoard::rows> &terrain,
                           const std::vector<std::pair<Point,Point>> &portalPairs)
            : entity(entity),terrain(terrain),portalPairs(portalPairs){}
    };
    const BoardData& boardData() const;

private:
    struct Spawn { Point headPos; Snake::Direction dir; int length; };
    static constexpr std::array<Spawn,4> spawns = {{
        { Point(0,3), Snake::Direction::Down, 4 },           // 玩家1
        { Point(GameBoard::cols-1, GameBoard::rows-4), Snake::Direction::Up, 4 }, // 玩家2
        { Point(0, GameBoard::rows-4), Snake::Direction::Up, 4 },     // 玩家3
        { Point(GameBoard::cols-1, 3), Snake::Direction::Down, 4 }       // 玩家4
    }};

    uint8_t playersNum;
    uint8_t mapId;
    std::vector<Snake*> players;
    GameBoard gameBoard;
    GameState currentState = GameState::Idle;

    mutable BoardData boardData_;

    void setState(GameState newState){currentState = newState;}
    void gameInit();
    void gameOver();
    void clean();

    void creatSnake(Snake *snake,Point head,int length,Snake::Direction dir);
    void cutSnakeHead(Snake *snake,int length);
    void destroySnake(Snake *snake);
    void moveSnake(Snake *snake);
    void processSnakeDeath(Snake *snake);
    void rebirth(Snake *snake);
};

#endif // GAMELOGIC_H
