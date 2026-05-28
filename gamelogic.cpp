#include "gamelogic.h"

GameLogic::GameLogic(uint8_t playersNum, uint8_t mapId) : playersNum(playersNum) , mapId(mapId) ,
    gameBoard() , boardData_(gameBoard.entity(),gameBoard.terrain(),gameBoard.portalPairs())
{

}

GameLogic::~GameLogic()
{
    clean();
}

bool GameLogic::setPlayersNum(uint8_t num)
{
    if(num> spawns.size()){
        return false;
    }
    playersNum = num;
    restartGame();
    return true;
}

void GameLogic::setMap(uint8_t id)
{
    mapId = id;
    restartGame();
}

void GameLogic::pushCmd(Snake::Direction dir, uint8_t playerId)
{
    if(playerId >= players.size()) return;
    players[playerId]->pushCmd(dir);
}

const GameLogic::BoardData& GameLogic::boardData() const
{
    boardData_.snakeHeads.clear();
    for(Snake *snake : players){
        auto head = snake->getHead();
        if(!head) continue;
        boardData_.snakeHeads.push_back(*head);
    }
    return boardData_;
}

void GameLogic::startGame()
{
    if (currentState != GameState::Idle) return;

    gameInit();
    gameBoard.setFood();
    setState(GameState::Playing);
}

void GameLogic::pauseGame()
{
    if (currentState != GameState::Playing) return;
    setState(GameState::Paused);
}

void GameLogic::resumeGame()
{
    if (currentState != GameState::Paused) return;
    setState(GameState::Playing);
}

void GameLogic::togglePause()
{
    if (currentState == GameState::Playing) {
        pauseGame();
    } else if (currentState == GameState::Paused) {
        resumeGame();
    }
}

void GameLogic::restartGame()
{
    clean();
    setState(GameState::Idle);
    startGame();
}

void GameLogic::clean()
{
    for (Snake* snake : players) {
        delete snake;
    }
    players.clear();
}

void GameLogic::gameOver()
{
    setState(GameState::Done);
}

void GameLogic::update()
{
    if (currentState != GameState::Playing) return;

    std::vector<Snake*> readySnakes;
    for (Snake* snake : players) {
        snake->tick();
        if (snake->isReady()) {
            readySnakes.push_back(snake);
        }
    }

    if (readySnakes.size() > 1) {
        std::sort(readySnakes.begin(), readySnakes.end(), [](const Snake* a, const Snake* b) {
            return a->getLength() > b->getLength();
        });
    }

    for (Snake* snake : readySnakes) {
        snake->readyMove();
        moveSnake(snake);
    }
}

void GameLogic::gameInit()
{
    gameBoard.init();
    gameBoard.makeMap(mapId);

    uint8_t nextId = 0;
    for(size_t i = 0; i < playersNum && i < spawns.size();i++){
        Snake *snake = new Snake(++nextId);
        creatSnake(snake,spawns[i].headPos,spawns[i].length,spawns[i].dir);
        players.push_back(snake);
    }
}

void GameLogic::moveSnake(Snake *snake) {
    if (!snake) return;

    auto oldHeadOpt = snake->getHead();
    auto oldTailOpt = snake->getTail();
    if (!oldHeadOpt || !oldTailOpt) {
        processSnakeDeath(snake);
        return;
    }
    Point oldHead = *oldHeadOpt;
    Point oldTail = *oldTailOpt;

    Point newHead = oldHead + snake->getStep();
    bool popTail = true;
    bool dead = false;
    int maxLoop = 100;

    while (!dead && maxLoop-- > 0) {
        auto cellTerrainOpt = gameBoard.getTerrainCell(newHead);
        auto cellEntityOpt = gameBoard.getEntityCell(newHead);

        if (!cellTerrainOpt || !cellEntityOpt) {
            dead = true;
            break;
        }

        GameBoard::Terrain cellTerrain = *cellTerrainOpt;
        GameBoard::Entity cellEntity = (*cellEntityOpt).type;

        if (cellEntity == GameBoard::Entity::Food) {
            popTail = false;
            break;
        }
        if (cellEntity == GameBoard::Entity::Snake) {
            bool isOwnTail = (newHead == oldTail);
            if (!(popTail && isOwnTail)) {
                dead = true;
            }
            break;
        }

        if (cellTerrain == GameBoard::Terrain::Wall) {
            dead = true;
            break;
        }
        if (cellTerrain == GameBoard::Terrain::Portal) {
            auto connected = gameBoard.findConnectedPortal(newHead);
            if (!connected) {
                dead = true;
                break;
            }
            newHead = *connected + snake->getStep();
            continue;
        }
        if (cellTerrain == GameBoard::Terrain::Empty || cellTerrain == GameBoard::Terrain::Track) {
            break;
        }

        dead = true;
        break;
    }

    if (dead) {
        processSnakeDeath(snake);
        return;
    }

    auto newTerrainOpt = gameBoard.getTerrainCell(newHead);

    bool newOnTrack = (newTerrainOpt && *newTerrainOpt == GameBoard::Terrain::Track);

    if (newOnTrack) {
        snake->setSpeed(20);
    } else {
        snake->setSpeed(10);
    }

    snake->setHead(newHead, popTail);

    if (popTail) {
        gameBoard.setEntityCell(oldTail, GameBoard::Entity::None);
    }
    gameBoard.setEntityCell(newHead, GameBoard::Entity::Snake,snake->id);

    if (!popTail) {
        if(!gameBoard.setFood()){
            gameOver();
        }
    }
}

void GameLogic::cutSnakeHead(Snake *snake, int length)
{
    if(!snake) return;

    for(int i = 0;i < length;i++){
        auto oldHead = snake->shrink();
        if(!oldHead){
            processSnakeDeath(snake);
            return;
        }
        gameBoard.setEntityCell(*oldHead,GameBoard::Entity::None);
    }
}

void GameLogic::destroySnake(Snake *snake){
    if(!snake) return;

    cutSnakeHead(snake,snake->getLength());
}

void GameLogic::creatSnake(Snake *snake, Point head, int length, Snake::Direction dir)
{
    if(!snake) return;

    destroySnake(snake);
    snake->init(dir);
    Point start = head - snake->getStep() * (length - 1);

    for(int i = 0;i < length;i++){
        if(!gameBoard.isCellEmpty(start)){
            rebirth(snake);
            return;
        }

        snake->setHead(start,false);
        gameBoard.setEntityCell(start,GameBoard::Entity::Snake,snake->id);
        start += snake->getStep();
    }
}

void GameLogic::processSnakeDeath(Snake *snake)
{
    if(!snake || players.size() == 1){
        gameOver();
        return;
    }

    if(snake->getLength() > 7){
        cutSnakeHead(snake,4);
        return;
    }
    rebirth(snake);
}

void GameLogic::rebirth(Snake *snake)
{
    if(!snake) return;
    destroySnake(snake);

    auto rect = gameBoard.findMaxEmptyRectangle();
    if(!rect){
        gameOver();
        return;
    }
    if(std::max((*rect).width(),(*rect).height()) < 6){
        gameOver();
        return;
    }
    if((*rect).width() > (*rect).height()){
        creatSnake(snake,Point((*rect).x() + 3,(*rect).y()),4,Snake::Direction::Right);
    }else{
        creatSnake(snake,Point((*rect).x(),(*rect).y() + 3),4,Snake::Direction::Down);
    }
}

