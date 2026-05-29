#include "gamelogic.h"

GameLogic::GameLogic(uint8_t playersNum, uint8_t mapId) : playersNum(playersNum) , mapId(mapId) ,
    gameBoard_() , boardData_(gameBoard_.entity(),gameBoard_.terrain(),gameBoard_.portalPairs()) , ai(gameBoard_)
{
    playerIdOfAi = 0xFF;
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

void GameLogic::setAi(uint8_t playerId)
{
    playerIdOfAi = playerId;
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
    gameBoard_.setFood();
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
            if(isAi(snake)){
                snake->pushCmd(ai.getAiCmd(*(snake->getHead())));
            }
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

bool GameLogic::isAi(Snake *snake) const
{
    if(playerIdOfAi >= players.size()) return false;
    if(players[playerIdOfAi] != snake) return false;
    return true;
}

void GameLogic::gameInit()
{
    gameBoard_.init();
    gameBoard_.makeMap(mapId);

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
        auto cellTerrainOpt = gameBoard_.getTerrainCell(newHead);
        auto cellEntityOpt = gameBoard_.getEntityCell(newHead);

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
            auto connected = gameBoard_.findConnectedPortal(newHead);
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

    auto newTerrainOpt = gameBoard_.getTerrainCell(newHead);

    bool newOnTrack = (newTerrainOpt && *newTerrainOpt == GameBoard::Terrain::Track);

    if (newOnTrack) {
        snake->setSpeed(GameConstants::trackSpeed);
    } else {
        snake->setSpeed(GameConstants::normalSpeed);
    }

    snake->setHead(newHead, popTail);

    if (popTail) {
        gameBoard_.setEntityCell(oldTail, GameBoard::Entity::None);
    }
    gameBoard_.setEntityCell(newHead, GameBoard::Entity::Snake,snake->id);

    if (!popTail) {
        if(!gameBoard_.setFood()){
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
        gameBoard_.setEntityCell(*oldHead,GameBoard::Entity::None);
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
        if(!gameBoard_.isCellEmpty(start)){
            rebirth(snake);
            return;
        }

        snake->setHead(start,false);
        gameBoard_.setEntityCell(start,GameBoard::Entity::Snake,snake->id);
        start += snake->getStep();
    }
}

void GameLogic::processSnakeDeath(Snake *snake)
{
    if(!snake || players.size() == 1){
        gameOver();
        return;
    }

    if(snake->getLength() > initialLength + cutLength - 1){
        cutSnakeHead(snake,cutLength);
        return;
    }
    rebirth(snake);
}

void GameLogic::rebirth(Snake *snake)
{
    if(!snake) return;
    destroySnake(snake);

    auto rect = gameBoard_.findMaxEmptyRectangle();
    if(!rect){
        gameOver();
        return;
    }
    if(std::max((*rect).width(),(*rect).height()) < 6){
        gameOver();
        return;
    }
    if((*rect).width() > (*rect).height()){
        creatSnake(snake,Point((*rect).x() + initialLength - 1,(*rect).y()),initialLength,Snake::Direction::Right);
    }else{
        creatSnake(snake,Point((*rect).x(),(*rect).y() + initialLength - 1),initialLength,Snake::Direction::Down);
    }
}

