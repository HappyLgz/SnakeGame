#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include "gamelogic.h"

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    ~Widget();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameLoop();

private:
    void drawGrid(QPainter &painter);
    void drawTerrain(QPainter &painter);
    void drawEntities(QPainter &painter);
    void drawPortals(QPainter &painter);
    void drawSnakeHeads(QPainter &painter);

    QColor getSnakeColor(uint playerId) const;
    QColor getTerrainColor(GameBoard::Terrain terrain) const;
    QColor getEntityColor(GameBoard::Entity entity, uint ownerId) const;

    GameLogic *gameLogic;
    QTimer *timer;
    int cellSize;
    int margin;
};

#endif // WIDGET_H
