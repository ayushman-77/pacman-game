#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QPoint>
#include <QTimer>
#include <QVector>
#include <QSet>
#include <QPair>
#include <QColor>
#include <QRect>
#include "my_label.h"

enum class EnemyType { Simple, Smart };

struct Enemy {
    int x, y;
    int dx, dy;
    QColor color;
    EnemyType type;
    QRect habitat;
    int moveInterval;
    int cooldown;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateFrame();

private:
    MyLabel *frame;
    int cellSize;
    int rows, cols;

    QVector<QVector<int>> maze;
    QVector<QVector<QPoint>> levels; // <-- stores all mazes

    int playerX, playerY;
    int mouthOpen;
    int playerDirX, playerDirY;
    int currentLevel; // <-- which maze is active

    QSet<QPair<int,int>> food;
    QVector<Enemy> enemies;

    void setupLevels();        // <-- defines wall arrays
    void initMaze(int levelNumber); // <-- builds maze from wall arrays

    void initFood();
    void initEnemies();
    void moveEnemies();
    void checkCollisions();

    bool isWalkable(int x, int y) const;
    bool aStarNextStep(int sx, int sy, int tx, int ty, int &nx, int &ny);

    friend void drawBlock(QImage &img, int gx, int gy, int cellSize, QColor color);
};

#endif // MAINWINDOW_H
