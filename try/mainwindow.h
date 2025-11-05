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

// Enemy behavior types
enum class EnemyType { Simple, Smart };

// Enemy structure (grid-based, no sprites)
struct Enemy {
    int x, y;           // grid position
    int dx, dy;         // movement direction (for Simple & Smart when idle)
    QColor color;       // render color
    EnemyType type;     // behavior

    // Regional A* activation (used only for Smart enemies)
    QRect habitat;      // grid-rect of habitat; A* only when Pac-Man is inside

    // Speed control: move every 'moveInterval' frames
    int moveInterval;   // e.g., 2 or 3 for slower enemies
    int cooldown;       // counts down each tick
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
    void updateFrame(); // raster repaint loop

private:
    // grid display
    MyLabel *frame;
    int cellSize;
    int rows, cols;

    // maze: 0=empty, 1=wall
    QVector<QVector<int>> maze;

    // pac-man player state
    int playerX, playerY;
    int mouthOpen;              // mouth animation toggle
    int playerDirX, playerDirY; // mouth direction

    // food pellets
    QSet<QPair<int,int>> food;

    // enemies (ghosts)
    QVector<Enemy> enemies;

    // functions
    void initMaze();
    void initFood();
    void initEnemies();
    void moveEnemies();
    void checkCollisions();

    // A* helpers (4-directional movement only)
    bool isWalkable(int x, int y) const;
    bool aStarNextStep(int sx, int sy, int tx, int ty, int &nx, int &ny);

    // raster helper
    friend void drawBlock(QImage &img, int gx, int gy, int cellSize, QColor color);
};

#endif // MAINWINDOW_H
