#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    gameLogic = new GameLogic(1,0);
    timer = new QTimer(this);
    // 初始化游戏
    gameLogic->startGame();

    // 设置定时器
    connect(timer, &QTimer::timeout, this, &Widget::gameLoop);
    timer->start(15);

    // 设置窗口大小
    cellSize = 25;
    margin = 10;
    int width = GameBoard::cols * cellSize + 2 * margin;
    int height = GameBoard::rows * cellSize + 2 * margin;
    setFixedSize(width, height);

    setFocusPolicy(Qt::StrongFocus);
    setWindowTitle("Snake Game");
}

Widget::~Widget()
{
    delete gameLogic;
}

void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制背景
    painter.fillRect(rect(), Qt::black);

    // 绘制网格
    drawGrid(painter);

    // 绘制地形
    drawTerrain(painter);

    // 绘制传送门
    drawPortals(painter);

    // 绘制实体（食物、蛇身）
    drawEntities(painter);

    // 绘制蛇头（最上层）
    drawSnakeHeads(painter);
}

void Widget::drawGrid(QPainter &painter)
{
    painter.setPen(QPen(Qt::darkGray, 1));

    // 绘制垂直线
    for (int x = 0; x <= GameBoard::cols; ++x) {
        int xPos = margin + x * cellSize;
        painter.drawLine(xPos, margin, xPos, margin + GameBoard::rows * cellSize);
    }

    // 绘制水平线
    for (int y = 0; y <= GameBoard::rows; ++y) {
        int yPos = margin + y * cellSize;
        painter.drawLine(margin, yPos, margin + GameBoard::cols * cellSize, yPos);
    }
}

void Widget::drawTerrain(QPainter &painter)
{
    const auto &terrain = gameLogic->boardData().terrain;

    for (int y = 0; y < GameBoard::rows; ++y) {
        for (int x = 0; x < GameBoard::cols; ++x) {
            GameBoard::Terrain t = terrain[y][x];
            if (t == GameBoard::Terrain::Empty) continue;

            QColor color = getTerrainColor(t);
            painter.setBrush(color);
            painter.setPen(Qt::NoPen);

            int xPos = margin + x * cellSize + 1;
            int yPos = margin + y * cellSize + 1;
            painter.drawRect(xPos, yPos, cellSize - 2, cellSize - 2);
        }
    }
}

void Widget::drawPortals(QPainter &painter)
{
    const auto &portalPairs = gameLogic->boardData().portalPairs;

    painter.setPen(QPen(Qt::magenta, 2,Qt::DashLine));
    painter.setBrush(Qt::NoBrush);

    for (const auto &pair : portalPairs) {
        int x1 = margin + pair.first.x() * cellSize + cellSize / 2;
        int y1 = margin + pair.first.y() * cellSize + cellSize / 2;
        int x2 = margin + pair.second.x() * cellSize + cellSize / 2;
        int y2 = margin + pair.second.y() * cellSize + cellSize / 2;

        // 绘制传送门连接线
        painter.drawLine(x1, y1, x2, y2);
    }
}

void Widget::drawEntities(QPainter &painter)
{
    const auto &entity = gameLogic->boardData().entity;

    for (int y = 0; y < GameBoard::rows; ++y) {
        for (int x = 0; x < GameBoard::cols; ++x) {
            const auto &cell = entity[y][x];
            if (cell.type == GameBoard::Entity::None) continue;

            QColor color = getEntityColor(cell.type, cell.ownerId);
            painter.setBrush(color);
            painter.setPen(Qt::NoPen);

            int xPos = margin + x * cellSize + 2;
            int yPos = margin + y * cellSize + 2;

            if (cell.type == GameBoard::Entity::Food) {
                // 食物画成圆形
                painter.drawEllipse(xPos, yPos, cellSize - 4, cellSize - 4);
            } else {
                // 蛇身画成矩形
                painter.drawRect(xPos, yPos, cellSize - 4, cellSize - 4);
            }
        }
    }
}

void Widget::drawSnakeHeads(QPainter &painter)
{
    const auto &snakeHeads = gameLogic->boardData().snakeHeads;
    const auto &entity = gameLogic->boardData().entity;

    painter.setPen(Qt::NoPen);

    for (size_t i = 0; i < snakeHeads.size(); ++i) {
        const Point &head = snakeHeads[i];
        if (head.x() < 0 || head.x() >= GameBoard::cols ||
            head.y() < 0 || head.y() >= GameBoard::rows) continue;

        const auto &cell = entity[head.y()][head.x()];
        if (cell.type != GameBoard::Entity::Snake) continue;

        QColor color = getSnakeColor(cell.ownerId);
        painter.setBrush(color.darker(150)); // 头部颜色更深

        int xPos = margin + head.x() * cellSize + 1;
        int yPos = margin + head.y() * cellSize + 1;

        // 蛇头画成稍大的圆角矩形
        QRect rect(xPos, yPos, cellSize - 2, cellSize - 2);
        painter.drawRoundedRect(rect, 4, 4);
    }
}

QColor Widget::getSnakeColor(uint playerId) const
{
    // 为不同玩家分配不同颜色
    switch (playerId) {
    case 1: return QColor(0, 200, 0);      // 玩家1: 绿色
    case 2: return QColor(0, 100, 255);    // 玩家2: 蓝色
    case 3: return QColor(255, 100, 0);    // 玩家3: 橙色
    case 4: return QColor(200, 0, 200);    // 玩家4: 紫色
    default: return Qt::white;
    }
}

QColor Widget::getTerrainColor(GameBoard::Terrain terrain) const
{
    switch (terrain) {
    case GameBoard::Terrain::Wall:   return QColor(100, 100, 100);    // 灰色
    case GameBoard::Terrain::Portal: return QColor(200, 0, 200, 100); // 半透明紫色
    case GameBoard::Terrain::Track:  return QColor(100, 100, 255, 80); // 半透明蓝色
    case GameBoard::Terrain::Empty:
    default: return Qt::transparent;
    }
}

QColor Widget::getEntityColor(GameBoard::Entity entity, uint ownerId) const
{
    if (entity == GameBoard::Entity::Food) {
        return QColor(255, 50, 50); // 红色食物
    } else if (entity == GameBoard::Entity::Snake) {
        return getSnakeColor(ownerId);
    }
    return Qt::transparent;
}

void Widget::gameLoop()
{
    // 更新游戏逻辑
    gameLogic->update();

    // 重绘
    update();
}

void Widget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_W:
        gameLogic->pushCmd(Snake::Direction::Up, 0);
        break;
    case Qt::Key_S:
        gameLogic->pushCmd(Snake::Direction::Down, 0);
        break;
    case Qt::Key_A:
        gameLogic->pushCmd(Snake::Direction::Left, 0);
        break;
    case Qt::Key_D:
        gameLogic->pushCmd(Snake::Direction::Right, 0);
        break;

    case Qt::Key_Up:
        gameLogic->pushCmd(Snake::Direction::Up, 1);
        break;
    case Qt::Key_Down:
        gameLogic->pushCmd(Snake::Direction::Down, 1);
        break;
    case Qt::Key_Left:
        gameLogic->pushCmd(Snake::Direction::Left, 1);
        break;
    case Qt::Key_Right:
        gameLogic->pushCmd(Snake::Direction::Right, 1);
        break;

    case Qt::Key_P:
        // 切换1人/2人模式
//        if (gameLogic->state() == GameLogic::GameState::Idle ||
//            gameLogic->state() == GameLogic::GameState::Done) {
        {
            uint8_t currentPlayers = gameLogic->boardData().snakeHeads.size();
            uint8_t newPlayers = (currentPlayers == 1) ? 2 : 1;
            gameLogic->setPlayersNum(newPlayers);
        }
//            gameLogic->restartGame();
//        }
        break;

    case Qt::Key_Space:
        // 暂停/继续
        gameLogic->togglePause();
        break;

    case Qt::Key_1:
        gameLogic->setMap(0);
        gameLogic->restartGame();
        break;
    case Qt::Key_2:
        gameLogic->setMap(1);
        gameLogic->restartGame();
        break;
    case Qt::Key_3:
        gameLogic->setMap(2);
        gameLogic->restartGame();
        break;
    case Qt::Key_4:
        gameLogic->setMap(3);
        gameLogic->restartGame();
        break;

    default:
        QWidget::keyPressEvent(event);
    }
}
