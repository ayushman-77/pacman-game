#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QSet>
#include <QVector>
#include <QPoint>
#include <QTimer>
#include <QKeyEvent>
#include <QPushButton>
#include <QMessageBox>
#include <QColor>
#include <QRect>
#include <QImage>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include "my_label.h"  // your label header

// Enemy types
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
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    // ----- GAME / GRID -----
    MyLabel *frame;
    int cellSize;
    int rows, cols;

    QVector<QVector<int>> maze;
    QSet<QPair<int,int>> food;
    QVector<Enemy> enemies;

    int playerX, playerY;
    int playerDirX, playerDirY;
    bool mouthOpen;

    // ----- LEVELS -----
    QVector<QVector<QPoint>> levels; // 4 levels
    int currentLevel;

    // ----- TIMER -----
    QTimer *gameTimer;

    // ----- LIVES -----
    int lives;

    // ----- GLOBAL EXIT BUTTON (top-right, outside frame) -----
    QPushButton *exitBtn;

    // ----- LEVEL SELECT OVERLAY (inside window, above frame) -----
    QWidget *menuOverlay = nullptr;
    QPushButton *btnLvl1 = nullptr;
    QPushButton *btnLvl2 = nullptr;
    QPushButton *btnLvl3 = nullptr;
    QPushButton *btnLvl4 = nullptr;
    QPushButton *btnMenuExit = nullptr;

    // ----- INITIALIZATION -----
    void setupLevels();                // defines 4 levels
    void initMaze(int levelNumber);
    void initFood();
    void initEnemies();

    // ----- GAME MECHANICS -----
    bool isWalkable(int x, int y) const;
    bool aStarNextStep(int sx, int sy, int tx, int ty, int &nx, int &ny);
    void moveEnemies();
    void checkCollisions();
    void updateFrame();

    // ----- DRAW -----
    void drawLives(QImage &img);

    // ----- GAME FLOW -----
    void startGame(int level);
    void stopGame();
    void showLevelSelect();            // shows the overlay

    // ----- UI builders -----
    void ensureMenuOverlay();          // creates overlay if needed
    void positionOverlay();            // matches overlay to frame area
};

#endif // MAINWINDOW_H
