#include "mainwindow.h"
#include <QTimer>
#include <QRandomGenerator>
#include <QDebug>
#include <queue>
#include <vector>
#include <limits>
#include <QMap>
#include <QFontDatabase>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QSoundEffect>

// Hash for QPair<int,int> so we can use QSet
inline uint qHash(const QPair<int,int> &key, uint seed = 0) {
    return qHash(key.first, seed) ^ (qHash(key.second, seed) << 1);
}

void MainWindow::updateHUD()
{
    if (!hud) return;
    hud->setScore(score);
    hud->setLives(lives);
    hud->setLevel(currentLevel);
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    cellSize(25), rows(25), cols(25),
    playerX(1), playerY(1), mouthOpen(0),
    playerDirX(0), playerDirY(0), currentLevel(1),
    gameTimer(new QTimer(this)),
    lives(3), score(0),
    exitBtn(new QPushButton("Exit", this))
{
    // ✅ FIX: Setup all level data first
    setupLevels();

    // Initialize frame
    frame = new MyLabel(this);
    frame->setFixedSize(cols * cellSize, rows * cellSize);
    frame->setStyleSheet("background:black");

    // Initialize HUD and overlay
    hud = new GameHUD(this);
    crtOverlay = new CRTOverlay(frame);
    crtOverlay->resize(frame->size());
    crtOverlay->raise();

    // Stack layout
    auto layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(hud);
    layout->addWidget(frame);

    QWidget *container = new QWidget(this);
    container->setLayout(layout);
    setCentralWidget(container);

    // Timer setup
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::updateFrame);

    // Exit button setup
    exitBtn->setFocusPolicy(Qt::NoFocus);
    QFont f = exitBtn->font();
    f.setPointSize(8);
    exitBtn->setFont(f);
    exitBtn->setFixedSize(50, 22);
    exitBtn->move(width() - exitBtn->width() - 1, 1);
    exitBtn->raise();

    connect(exitBtn, &QPushButton::clicked, this, [this]() {
        stopGame();
        auto r = QMessageBox::question(this, "Exit", "Exit the game?",
                                       QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::Yes)
            close();
        else
            showLevelSelect();
    });

    // ✅ Initialize level data safely
    initMaze(currentLevel);
    initFood();
    initEnemies();
    // Initialize sound system
    initAudio();


    // Show level-select overlay after startup
    QTimer::singleShot(0, this, [this]() {
        ensureMenuOverlay();
        showLevelSelect();
    });
}



MainWindow::~MainWindow() {}

// ======== LEVELS (4 levels) ========

void MainWindow::setupLevels() {
    // Level 1 – custom
    QVector<QPoint> lvl4 = {
        QPoint(1, 2), QPoint(2, 2), QPoint(3, 2), QPoint(3, 3), QPoint(3, 4), QPoint(2, 4),
        QPoint(6, 2), QPoint(6, 3), QPoint(6, 4), QPoint(7, 4), QPoint(8, 4), QPoint(8, 3),
        QPoint(10, 10), QPoint(11, 10), QPoint(12, 10), QPoint(13, 10), QPoint(15, 10),
        QPoint(14, 10), QPoint(15, 11), QPoint(15, 12), QPoint(13, 13), QPoint(14, 13),
        QPoint(15, 13), QPoint(10, 13), QPoint(9, 13), QPoint(8, 13), QPoint(8, 12),
        QPoint(8, 11), QPoint(8, 10), QPoint(9, 10), QPoint(10, 3), QPoint(11, 3),
        QPoint(11, 4), QPoint(11, 5), QPoint(11, 6), QPoint(10, 6), QPoint(9, 6),
        QPoint(16, 2), QPoint(15, 2), QPoint(15, 3), QPoint(15, 4), QPoint(15, 5),
        QPoint(16, 5), QPoint(17, 5), QPoint(18, 5), QPoint(18, 4), QPoint(18, 3),
        QPoint(19, 3), QPoint(20, 3), QPoint(20, 4), QPoint(13, 4), QPoint(13, 5),
        QPoint(13, 6), QPoint(13, 7), QPoint(14, 7), QPoint(15, 7), QPoint(16, 7),
        QPoint(17, 7), QPoint(18, 7), QPoint(20, 6), QPoint(20, 7), QPoint(22, 3),
        QPoint(23, 3), QPoint(22, 4), QPoint(22, 5), QPoint(1, 6), QPoint(2, 6),
        QPoint(3, 6), QPoint(3, 7), QPoint(3, 9), QPoint(3, 10), QPoint(2, 10),
        QPoint(1, 10), QPoint(5, 8), QPoint(5, 9), QPoint(5, 10), QPoint(5, 11),
        QPoint(6, 11), QPoint(6, 12), QPoint(6, 13), QPoint(5, 13), QPoint(4, 13),
        QPoint(2, 13), QPoint(3, 13), QPoint(2, 12), QPoint(7, 8), QPoint(7, 7),
        QPoint(7, 6), QPoint(6, 6), QPoint(5, 6), QPoint(17, 13), QPoint(18, 13),
        QPoint(19, 13), QPoint(20, 13), QPoint(20, 12), QPoint(20, 11), QPoint(23, 12),
        QPoint(20, 10), QPoint(21, 10), QPoint(22, 10), QPoint(9, 8), QPoint(10, 8),
        QPoint(11, 8), QPoint(1, 15), QPoint(2, 15), QPoint(3, 15), QPoint(4, 15),
        QPoint(4, 16), QPoint(4, 17), QPoint(4, 18), QPoint(5, 18), QPoint(6, 18),
        QPoint(2, 18), QPoint(2, 17), QPoint(2, 19), QPoint(2, 20), QPoint(3, 20),
        QPoint(4, 20), QPoint(4, 21), QPoint(7, 18), QPoint(7, 19), QPoint(6, 22),
        QPoint(6, 21), QPoint(7, 21), QPoint(8, 21), QPoint(9, 21), QPoint(9, 16),
        QPoint(9, 17), QPoint(9, 18), QPoint(9, 15), QPoint(7, 15), QPoint(8, 15),
        QPoint(4, 22), QPoint(14, 8), QPoint(11, 15), QPoint(12, 15), QPoint(13, 15),
        QPoint(14, 15), QPoint(14, 16), QPoint(14, 17), QPoint(13, 17), QPoint(12, 17),
        QPoint(12, 18), QPoint(12, 19), QPoint(11, 19), QPoint(17, 14), QPoint(17, 15),
        QPoint(17, 16), QPoint(17, 17), QPoint(16, 17), QPoint(16, 18), QPoint(16, 19),
        QPoint(15, 19), QPoint(15, 20), QPoint(17, 12), QPoint(17, 11), QPoint(17, 10),
        QPoint(15, 21), QPoint(14, 21), QPoint(13, 21), QPoint(19, 17), QPoint(19, 18),
        QPoint(19, 19), QPoint(18, 19), QPoint(19, 16), QPoint(20, 16), QPoint(21, 16),
        QPoint(22, 16), QPoint(22, 17), QPoint(20, 21), QPoint(21, 21), QPoint(19, 21),
        QPoint(21, 18), QPoint(22, 18), QPoint(21, 19), QPoint(21, 20), QPoint(18, 21),
        QPoint(18, 22), QPoint(22, 12), QPoint(22, 13), QPoint(22, 14)
    };

    // Level 2 – custom
    QVector<QPoint> lvl2 = {
        QPoint(5,2), QPoint(5,3), QPoint(5,4), QPoint(5,5), QPoint(4,5), QPoint(2,2), QPoint(2,3),
        QPoint(3,2), QPoint(3,3), QPoint(3,5), QPoint(3,6), QPoint(3,7), QPoint(2,9), QPoint(3,9),
        QPoint(3,10), QPoint(3,11), QPoint(3,12), QPoint(2,12), QPoint(16,10), QPoint(15,9),
        QPoint(14,8), QPoint(13,8), QPoint(12,9), QPoint(11,10), QPoint(16,14), QPoint(15,15),
        QPoint(11,14), QPoint(12,15), QPoint(13,16), QPoint(14,16), QPoint(16,11), QPoint(16,13),
        QPoint(11,11), QPoint(11,13), QPoint(17,11), QPoint(18,11), QPoint(17,13), QPoint(18,13),
        QPoint(5,11), QPoint(6,11), QPoint(7,11), QPoint(9,8), QPoint(9,9), QPoint(9,10),
        QPoint(9,11), QPoint(8,11), QPoint(5,9), QPoint(6,9), QPoint(7,9), QPoint(7,6),
        QPoint(7,7), QPoint(6,7), QPoint(6,5), QPoint(7,5), QPoint(8,3), QPoint(8,2),
        QPoint(10,2), QPoint(9,2), QPoint(11,2), QPoint(12,2), QPoint(13,2), QPoint(11,3),
        QPoint(11,4), QPoint(10,4), QPoint(10,5), QPoint(10,6), QPoint(14,5), QPoint(15,5),
        QPoint(15,4), QPoint(15,3), QPoint(16,3), QPoint(17,3), QPoint(17,2), QPoint(18,2),
        QPoint(19,2), QPoint(19,3), QPoint(19,4), QPoint(19,5), QPoint(18,5), QPoint(17,5),
        QPoint(19,11), QPoint(20,11), QPoint(19,13), QPoint(20,13), QPoint(22,2), QPoint(22,3),
        QPoint(21,3), QPoint(21,4), QPoint(21,5), QPoint(22,5), QPoint(22,6), QPoint(22,7),
        QPoint(21,7), QPoint(20,7), QPoint(18,7), QPoint(19,7), QPoint(22,9), QPoint(22,10),
        QPoint(20,9), QPoint(21,9), QPoint(19,9), QPoint(22,13), QPoint(22,14), QPoint(22,15),
        QPoint(22,16), QPoint(21,16), QPoint(20,16), QPoint(19,16), QPoint(18,16), QPoint(18,18),
        QPoint(18,15), QPoint(19,15), QPoint(6,14), QPoint(5,15), QPoint(4,16), QPoint(3,17),
        QPoint(2,18), QPoint(9,14), QPoint(8,15), QPoint(7,16), QPoint(6,17), QPoint(5,18),
        QPoint(4,19), QPoint(4,20), QPoint(4,21), QPoint(6,12), QPoint(1,18), QPoint(2,14),
        QPoint(2,13), QPoint(2,15), QPoint(14,12), QPoint(13,12), QPoint(13,11), QPoint(14,11),
        QPoint(14,13), QPoint(13,13), QPoint(8,19), QPoint(7,19), QPoint(7,20), QPoint(14,22),
        QPoint(15,22), QPoint(16,22), QPoint(16,20), QPoint(16,21), QPoint(16,19), QPoint(20,18),
        QPoint(20,19), QPoint(20,20), QPoint(19,20), QPoint(18,20), QPoint(18,21), QPoint(18,22),
        QPoint(19,22), QPoint(20,22), QPoint(21,22), QPoint(22,22), QPoint(22,18), QPoint(22,19),
        QPoint(22,20), QPoint(10,19), QPoint(9,19), QPoint(11,21), QPoint(11,19), QPoint(11,20),
        QPoint(11,18), QPoint(11,17), QPoint(10,17), QPoint(11,22), QPoint(12,20), QPoint(12,19),
        QPoint(12,21), QPoint(2,21), QPoint(3,21), QPoint(2,22), QPoint(3,22), QPoint(4,22),
        QPoint(7,21), QPoint(7,22), QPoint(8,22), QPoint(9,22), QPoint(9,21), QPoint(16,17),
        QPoint(17,17), QPoint(18,17), QPoint(16,18), QPoint(15,19), QPoint(14,19), QPoint(13,5),
        QPoint(13,6), QPoint(14,6)
    };

    // Level 3 – custom
    QVector<QPoint> lvl3 = {
        QPoint(5,2), QPoint(5,3), QPoint(5,4), QPoint(5,5), QPoint(5,6), QPoint(19,2), QPoint(19,3),
        QPoint(19,4), QPoint(19,5), QPoint(19,6), QPoint(7,2), QPoint(8,2), QPoint(9,2), QPoint(10,2),
        QPoint(17,2), QPoint(16,2), QPoint(15,2), QPoint(14,2), QPoint(2,2), QPoint(3,2), QPoint(3,3),
        QPoint(2,3), QPoint(2,5), QPoint(3,5), QPoint(3,6), QPoint(2,6), QPoint(21,2), QPoint(22,2),
        QPoint(22,3), QPoint(21,3), QPoint(21,5), QPoint(22,5), QPoint(22,6), QPoint(21,6), QPoint(2,21),
        QPoint(2,22), QPoint(3,22), QPoint(3,21), QPoint(5,22), QPoint(5,21), QPoint(5,20), QPoint(5,19),
        QPoint(5,18), QPoint(2,18), QPoint(3,18), QPoint(3,19), QPoint(2,19), QPoint(22,22), QPoint(21,22),
        QPoint(21,21), QPoint(22,21), QPoint(21,19), QPoint(21,18), QPoint(22,18), QPoint(22,19), QPoint(19,18),
        QPoint(19,19), QPoint(19,20), QPoint(19,21), QPoint(19,22), QPoint(7,22), QPoint(8,22), QPoint(9,22),
        QPoint(10,22), QPoint(14,22), QPoint(15,22), QPoint(16,22), QPoint(17,22), QPoint(7,4), QPoint(8,4),
        QPoint(7,5), QPoint(16,4), QPoint(17,4), QPoint(17,5), QPoint(7,19), QPoint(7,20), QPoint(8,20),
        QPoint(17,19), QPoint(17,20), QPoint(16,20), QPoint(4,8), QPoint(4,9), QPoint(4,10), QPoint(4,14),
        QPoint(4,15), QPoint(4,16), QPoint(2,12), QPoint(3,12), QPoint(4,12), QPoint(5,12), QPoint(6,12),
        QPoint(20,8), QPoint(20,9), QPoint(20,10), QPoint(20,12), QPoint(19,12), QPoint(21,12), QPoint(22,12),
        QPoint(18,12), QPoint(20,14), QPoint(20,15), QPoint(20,16), QPoint(12,2), QPoint(12,3), QPoint(12,4),
        QPoint(12,5), QPoint(12,6), QPoint(12,7), QPoint(12,8), QPoint(12,16), QPoint(12,17), QPoint(12,18),
        QPoint(12,19), QPoint(12,20), QPoint(12,21), QPoint(12,22), QPoint(2,9), QPoint(22,9), QPoint(22,15),
        QPoint(2,15), QPoint(8,11), QPoint(8,10), QPoint(8,9), QPoint(8,8), QPoint(8,13), QPoint(8,14),
        QPoint(8,15), QPoint(8,16), QPoint(9,6), QPoint(10,6), QPoint(10,7), QPoint(10,8), QPoint(9,18),
        QPoint(10,18), QPoint(10,17), QPoint(10,16), QPoint(10,10), QPoint(10,11), QPoint(10,12), QPoint(10,13),
        QPoint(10,14), QPoint(5,8), QPoint(6,9), QPoint(7,10), QPoint(5,16), QPoint(6,15), QPoint(7,14),
        QPoint(14,6), QPoint(15,6), QPoint(14,7), QPoint(14,8), QPoint(14,16), QPoint(14,17), QPoint(14,18),
        QPoint(15,18), QPoint(14,10), QPoint(14,11), QPoint(14,12), QPoint(14,13), QPoint(16,8), QPoint(16,9),
        QPoint(16,10), QPoint(16,11), QPoint(16,13), QPoint(16,14), QPoint(16,15), QPoint(16,16), QPoint(17,14),
        QPoint(18,15), QPoint(19,16), QPoint(17,10), QPoint(18,9), QPoint(19,8), QPoint(9,4), QPoint(15,4),
        QPoint(9,20), QPoint(15,20), QPoint(11,10), QPoint(13,10), QPoint(11,14), QPoint(14,14), QPoint(13,14),
        QPoint(12,12)
    };

    // Level 4 – custom classic-ish
    QVector<QPoint> lvl1 = {
        QPoint(2,2), QPoint(2,3), QPoint(2,4), QPoint(2,5), QPoint(2,6), QPoint(3,2), QPoint(4,2),
        QPoint(5,2), QPoint(6,2), QPoint(7,2), QPoint(6,3), QPoint(3,6), QPoint(2,7), QPoint(6,4),
        QPoint(4,6), QPoint(22,2), QPoint(21,2), QPoint(20,2), QPoint(19,2), QPoint(18,2), QPoint(17,2),
        QPoint(22,3), QPoint(22,4), QPoint(22,5), QPoint(22,6), QPoint(22,7), QPoint(18,3), QPoint(18,4),
        QPoint(20,6), QPoint(21,6), QPoint(2,22), QPoint(2,21), QPoint(2,20), QPoint(2,19), QPoint(2,18),
        QPoint(2,17), QPoint(3,22), QPoint(4,22), QPoint(5,22), QPoint(6,22), QPoint(7,22), QPoint(3,18),
        QPoint(4,18), QPoint(6,21), QPoint(6,20), QPoint(22,17), QPoint(22,18), QPoint(22,19), QPoint(22,20),
        QPoint(22,21), QPoint(22,22), QPoint(21,22), QPoint(20,22), QPoint(19,22), QPoint(18,22), QPoint(17,22),
        QPoint(18,21), QPoint(18,20), QPoint(21,18), QPoint(20,18), QPoint(4,20), QPoint(20,20), QPoint(20,4),
        QPoint(4,4), QPoint(6,6), QPoint(6,7), QPoint(6,8), QPoint(6,9), QPoint(6,10), QPoint(6,18),
        QPoint(6,17), QPoint(6,16), QPoint(6,15), QPoint(6,14), QPoint(18,18), QPoint(18,17), QPoint(18,16),
        QPoint(18,15), QPoint(18,14), QPoint(18,6), QPoint(18,7), QPoint(18,8), QPoint(18,9), QPoint(18,10),
        QPoint(9,2), QPoint(10,2), QPoint(11,2), QPoint(13,2), QPoint(14,2), QPoint(15,2), QPoint(9,22),
        QPoint(10,22), QPoint(11,22), QPoint(13,22), QPoint(14,22), QPoint(15,22), QPoint(4,8), QPoint(4,9),
        QPoint(4,10), QPoint(4,14), QPoint(4,15), QPoint(4,16), QPoint(20,8), QPoint(20,9), QPoint(20,10),
        QPoint(20,14), QPoint(20,15), QPoint(20,16), QPoint(21,12), QPoint(22,12), QPoint(2,12), QPoint(3,12),
        QPoint(22,9), QPoint(22,10), QPoint(22,11), QPoint(2,9), QPoint(2,10), QPoint(2,11), QPoint(22,13),
        QPoint(22,14), QPoint(22,15), QPoint(2,13), QPoint(2,14), QPoint(2,15), QPoint(9,4), QPoint(10,5),
        QPoint(11,6), QPoint(12,7), QPoint(13,8), QPoint(14,9), QPoint(15,10), QPoint(15,4), QPoint(14,5),
        QPoint(13,6), QPoint(11,8), QPoint(10,9), QPoint(9,10), QPoint(9,20), QPoint(10,19), QPoint(11,18),
        QPoint(12,17), QPoint(13,16), QPoint(14,15), QPoint(15,14), QPoint(11,16), QPoint(10,15), QPoint(9,14),
        QPoint(13,18), QPoint(14,19), QPoint(15,20), QPoint(16,6), QPoint(16,7), QPoint(16,8), QPoint(8,6),
        QPoint(8,7), QPoint(8,8), QPoint(8,16), QPoint(8,17), QPoint(8,18), QPoint(16,16), QPoint(16,17),
        QPoint(16,18), QPoint(12,10), QPoint(12,11), QPoint(12,12), QPoint(12,13), QPoint(12,14), QPoint(5,12),
        QPoint(6,12), QPoint(8,12), QPoint(9,12), QPoint(15,12), QPoint(16,12), QPoint(18,12), QPoint(19,12)
    };

    levels = { lvl1, lvl2, lvl3, lvl4 };
}

void MainWindow::initMaze(int levelNumber)
{
    maze.clear();
    maze.resize(rows);
    for (int y = 0; y < rows; ++y) {
        maze[y].resize(cols, 0);
    }

    // boundary walls
    for (int x = 0; x < cols; x++) {
        maze[0][x] = 1;
        maze[rows - 1][x] = 1;
    }
    for (int y = 0; y < rows; y++) {
        maze[y][0] = 1;
        maze[y][cols - 1] = 1;
    }

    // Load level walls
    if (levelNumber <= 0 || levelNumber > levels.size())
        levelNumber = 1;

    const QVector<QPoint> &currentWalls = levels[levelNumber - 1];
    for (const QPoint &p : currentWalls) {
        if (p.x() >= 0 && p.x() < cols && p.y() >= 0 && p.y() < rows)
            maze[p.y()][p.x()] = 1;
    }

    playerX = 1;
    playerY = 1;
}

void MainWindow::initFood()
{
    food.clear();
    for (int y=1; y<rows-1; y++)
        for (int x=1; x<cols-1; x++)
            if (!maze[y][x])
                food.insert(qMakePair(x,y));

    food.remove(qMakePair(playerX, playerY));
}

void MainWindow::initEnemies()
{
    enemies.clear();

    QRect topLeft(0, 0, cols/2, rows/2);
    QRect topRight(cols/2, 0, cols - cols/2, rows/2);
    QRect bottomLeft(0, rows/2, cols/2, rows - rows/2);
    QRect bottomRight(cols/2, rows/2, cols - cols/2, rows - rows/2);

    // ---------- LEVEL 1 ----------
    if (currentLevel == 1)
    {
        enemies.push_back(Enemy{ 1, rows-2,  1, 0, Qt::green,  EnemyType::Simple, QRect(), 1, 0 });
        enemies.push_back(Enemy{ cols-2, rows-2, 0,-1, Qt::blue,   EnemyType::Simple, QRect(), 1, 1 });
        enemies.push_back(Enemy{ cols/2, 1, 1, 0, Qt::red, EnemyType::Simple, QRect(), 1, 0 });
        return;
    }

    // ---------- LEVEL 2 ----------
    if (currentLevel == 2)
    {
        // Smart enemy in top-left quadrant
        enemies.push_back(Enemy{ 10, 2, 1, 0, Qt::red, EnemyType::Smart, topLeft, 1, 0 });

        // Simple enemies
        enemies.push_back(Enemy{ cols-2, rows-2, 0,-1, Qt::blue, EnemyType::Simple, QRect(), 1, 0 });
        enemies.push_back(Enemy{ 1, rows-2, 1, 0, Qt::green, EnemyType::Simple, QRect(), 1, 0 });
        return;
    }

    // ---------- LEVEL 3 ----------
    if (currentLevel == 3)
    {
        enemies.push_back(Enemy{ 10, 2, 1, 0, Qt::red,      EnemyType::Smart,  topLeft,   1, 0 });
        enemies.push_back(Enemy{ cols-2, 1, -1,0, Qt::magenta, EnemyType::Smart, topRight,  1, 1 });
        enemies.push_back(Enemy{ 1, rows-2, 1, 0, Qt::green,  EnemyType::Simple, QRect(),   1, 0 });
        enemies.push_back(Enemy{ cols-2, rows-2, 0,-1, Qt::blue,  EnemyType::Simple, QRect(),1, 1 });
        return;
    }

    // ---------- LEVEL 4 ----------
    if (currentLevel == 4)
    {
        enemies.push_back(Enemy{ 10, 2, 1, 0, Qt::cyan,   EnemyType::Smart,  topLeft, 1, 0 });
        enemies.push_back(Enemy{ 22, 22, -1,0, Qt::green, EnemyType::Smart, bottomRight, 1, 0 });
        enemies.push_back(Enemy{ 2, 22, 1, 0, Qt::white,  EnemyType::Smart, bottomLeft, 1, 0 });
        return;
    }
}

void drawBlock(QImage &img, int gx, int gy, int cellSize, QColor color)
{
    for(int py = gy * cellSize; py < (gy+1)*cellSize; py++)
        for(int px = gx * cellSize; px < (gx+1)*cellSize; px++)
            img.setPixel(px, py, color.rgb());
}

// --- A* helpers ---
bool MainWindow::isWalkable(int x, int y) const {
    if (x < 0 || y < 0 || x >= cols || y >= rows) return false;
    return maze[y][x] == 0;
}

// Compute next step towards (tx,ty) from (sx,sy) using A* with 4-neighbour moves.
bool MainWindow::aStarNextStep(int sx, int sy, int tx, int ty, int &nx, int &ny) {
    if (sx == tx && sy == ty) return false;

    auto start = qMakePair(sx, sy);
    auto goal  = qMakePair(tx, ty);

    struct Node { QPair<int,int> p; int f; int g; };

    auto heuristic = [&](const QPair<int,int> &a, const QPair<int,int> &b)->int {
        return std::abs(a.first - b.first) + std::abs(a.second - b.second);
    };

    auto cmp = [](const Node &a, const Node &b){
        if (a.f != b.f) return a.f > b.f;
        return a.g > b.g;
    };
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> open(cmp);

    QSet<QPair<int,int>> openSet;
    QSet<QPair<int,int>> closedSet;
    QMap<QPair<int,int>, QPair<int,int>> cameFrom;
    QMap<QPair<int,int>, int> gScore;

    gScore[start] = 0;
    open.push({start, heuristic(start, goal), 0});
    openSet.insert(start);

    auto pushNeighbor = [&](const QPair<int,int> &current, int nx_, int ny_) {
        QPair<int,int> np(nx_, ny_);
        if (!isWalkable(nx_, ny_) || closedSet.contains(np)) return;

        int tentative_g = gScore[current] + 1;
        if (!gScore.contains(np)) gScore[np] = std::numeric_limits<int>::max();
        if (tentative_g < gScore[np]) {
            cameFrom[np] = current;
            gScore[np] = tentative_g;
            int f = tentative_g + heuristic(np, goal);
            open.push({np, f, tentative_g});
            openSet.insert(np);
        }
    };

    while(!open.empty()) {
        Node cur = open.top(); open.pop();
        QPair<int,int> current = cur.p;
        if (closedSet.contains(current)) continue;
        closedSet.insert(current);
        openSet.remove(current);

        if (current == goal) {
            QPair<int,int> step = current;
            while (cameFrom.contains(step) && cameFrom[step] != start) {
                step = cameFrom[step];
            }
            if (cameFrom.contains(step) || step == goal) {
                nx = step.first; ny = step.second;
                return true;
            } else return false;
        }

        int cx = current.first, cy = current.second;
        pushNeighbor(current, cx+1, cy);
        pushNeighbor(current, cx-1, cy);
        pushNeighbor(current, cx, cy+1);
        pushNeighbor(current, cx, cy-1);
    }

    return false;
}

void MainWindow::moveEnemies()
{
    QPoint playerPt(playerX, playerY);

    for (auto &e : enemies) {
        if (e.cooldown > 0) {
            e.cooldown--;
            continue;
        }
        e.cooldown = e.moveInterval;

        if (e.type == EnemyType::Smart) {
            bool playerInHabitat = e.habitat.contains(playerPt);
            if (playerInHabitat) {
                int nx = e.x, ny = e.y;
                bool hasStep = aStarNextStep(e.x, e.y, playerX, playerY, nx, ny);
                if (hasStep) {
                    e.dx = nx - e.x;
                    e.dy = ny - e.y;
                    e.x = nx;
                    e.y = ny;
                    continue;
                }
            }
            int tryx = e.x + e.dx;
            int tryy = e.y + e.dy;
            if (isWalkable(tryx, tryy)) {
                e.x = tryx; e.y = tryy;
            } else {
                if (e.dx != 0) e.dx = -e.dx;
                else if (e.dy != 0) e.dy = -e.dy;
            }
        } else {
            int tryx = e.x + e.dx;
            int tryy = e.y + e.dy;
            if (isWalkable(tryx, tryy)) {
                e.x = tryx; e.y = tryy;
            } else {
                if (e.dx != 0) e.dx = -e.dx;
                if (e.dy != 0) e.dy = -e.dy;
            }
        }
    }
}

void MainWindow::checkCollisions()
{
    // inside MainWindow::checkCollisions()
    auto cur = qMakePair(playerX, playerY);
    if (food.contains(cur)) {
        food.remove(cur);
        score += 10;
        updateHUD();
        sfxEat->play();    // 🔊 PLAY EAT SOUND
    }




    for (auto &e : enemies)
        if (e.x == playerX && e.y == playerY) {
            // Reset on collision
            playerX = 1; playerY = 1;
            initEnemies();

            // Life handling (no audio)
            lives -= 1;
            updateHUD();

            if (lives <= 0) {
                if (gameTimer->isActive()) gameTimer->stop();
                bgMusic->stop();    // ⛔ Stop background music
                sfxDeath->play();   // 🔊 Play death sound

                QTimer::singleShot(300, this, [this](){
                    handleGameOver();
                });
            }


            break;
        }
}

void MainWindow::updateFrame()
{
    mouthOpen = !mouthOpen;

    // If a direction is set, attempt to move the player this tick.
    if (!(playerDirX == 0 && playerDirY == 0)) {
        int nx = playerX + playerDirX;
        int ny = playerY + playerDirY;
        if (isWalkable(nx, ny)) {
            playerX = nx;
            playerY = ny;

            // Eat food immediately and update score/HUD
            auto cur = qMakePair(playerX, playerY);
            if (food.contains(cur)) {
                food.remove(cur);
                score += 10;
                updateHUD();
                sfxEat->play();    // 🔊 PLAY EAT SOUND
            }

        }
    }

    // Move enemies and check collisions every tick regardless of player movement
    moveEnemies();
    checkCollisions();


    if (food.isEmpty()) {
        if (gameTimer->isActive()) gameTimer->stop();
        bgMusic->stop();
        sfxWin->play(); // win sound

        QTimer::singleShot(400, this, [this](){
            handleWin();
        });
        return;
    }


    // --- draw the whole scene (maze, food, enemies, player, etc.) ---
    QImage img(frame->width(), frame->height(), QImage::Format_RGB32);
    img.fill(Qt::black);

    // MAZE
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < cols; ++x)
            if (maze[y][x] == 1)
                drawBlock(img, x, y, cellSize, Qt::darkBlue);

    // FOOD dots
    for (const auto &f : food) {
        int gx = f.first, gy = f.second;
        int dot = cellSize / 4;
        int sx = gx * cellSize + (cellSize - dot) / 2;
        int sy = gy * cellSize + (cellSize - dot) / 2;
        for (int py = sy; py < sy + dot; ++py)
            for (int px = sx; px < sx + dot; ++px)
                img.setPixel(px, py, QColor(Qt::white).rgb());
    }

    // ENEMIES
    for (const auto &e : enemies)
        drawBlock(img, e.x, e.y, cellSize, e.color);

    // PAC-MAN
    drawBlock(img, playerX, playerY, cellSize, Qt::yellow);

    // mouth wedge...
    if (mouthOpen) {
        int px = playerX * cellSize;
        int py = playerY * cellSize;
        for (int y = 0; y < cellSize; ++y)
            for (int x = 0; x < cellSize; ++x) {
                int rx = px + x, ry = py + y;
                float dx = x - cellSize / 2.0f;
                float dy = y - cellSize / 2.0f;
                float angle = atan2(dy, dx) * 180.0f / M_PI;
                bool cut = false;
                if (playerDirX == 1 && angle > -30 && angle < 30) cut = true;
                if (playerDirX == -1 && (angle > 150 || angle < -150)) cut = true;
                if (playerDirY == 1 && angle > 60 && angle < 120) cut = true;
                if (playerDirY == -1 && angle > -120 && angle < -60) cut = true;
                if (cut) img.setPixel(rx, ry, QColor(Qt::black).rgb());
            }
    }

    frame->setPixmap(QPixmap::fromImage(img));
}

// =========================
// 📜 LEADERBOARD SYSTEM
// =========================

void MainWindow::saveScore(const QString &name, int score)
{
    QFile file("leaderboard.txt");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << name << "," << score << "\n";
        file.close();
    }
}

QVector<QPair<QString,int>> MainWindow::loadLeaderboard()
{
    QVector<QPair<QString,int>> list;
    QFile file("leaderboard.txt");

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            auto parts = line.split(',');
            if (parts.size() == 2) {
                list.append({ parts[0], parts[1].toInt() });
            }
        }
        file.close();
    }

    // sort descending
    std::sort(list.begin(), list.end(),
              [](auto &a, auto &b) { return a.second > b.second; });

    return list;
}

void MainWindow::showLeaderboard()
{
    QVector<QPair<QString,int>> list = loadLeaderboard();

    QString msg = "<b>🏆 Leaderboard</b><br><br>";
    int count = 0;

    for (auto &p : list) {
        msg += QString("%1. %2 — %3<br>")
                   .arg(++count)
                   .arg(p.first)
                   .arg(p.second);
        if (count == 10) break; // top 10
    }

    if (count == 0)
        msg += "No scores yet!";

    QMessageBox::information(this, "Leaderboard", msg);
}



void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (menuOverlay && menuOverlay->isVisible()) { e->ignore(); return; }

    // set direction when key pressed (do NOT move immediately here)
    if (e->key() == Qt::Key_Left)  { playerDirX = -1; playerDirY = 0; }
    if (e->key() == Qt::Key_Right) { playerDirX = 1;  playerDirY = 0; }
    if (e->key() == Qt::Key_Up)    { playerDirX = 0;  playerDirY = -1; }
    if (e->key() == Qt::Key_Down)  { playerDirX = 0;  playerDirY = 1; }
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    if (menuOverlay && menuOverlay->isVisible()) { e->ignore(); return; }

    if (e->key() == Qt::Key_Left ||
        e->key() == Qt::Key_Right ||
        e->key() == Qt::Key_Up ||
        e->key() == Qt::Key_Down)
    {
        // stop immediately when the arrow key is released
        playerDirX = 0;
        playerDirY = 0;
    }
}


void MainWindow::initAudio()
{
    // ----- Background Music -----
    bgMusic = new QMediaPlayer(this);
    bgOutput = new QAudioOutput(this);
    bgMusic->setAudioOutput(bgOutput);

    // Set your music file (MP3/WAV/OGG)
    bgMusic->setSource(QUrl("qrc:/sounds/bgm.mp3"));
    bgOutput->setVolume(0.4);

    // Loop forever
    connect(bgMusic, &QMediaPlayer::mediaStatusChanged, this, [&](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia)
            bgMusic->play();
    });

    bgMusic->play();


    // ----- Eat Pellet -----
    sfxEat = new QSoundEffect(this);
    sfxEat->setSource(QUrl("qrc:/sounds/eat.wav"));
    sfxEat->setVolume(0.65);

    // ----- Death -----
    sfxDeath = new QSoundEffect(this);
    sfxDeath->setSource(QUrl("qrc:/sounds/death.wav"));
    sfxDeath->setVolume(0.9);

    // ----- Win -----
    sfxWin = new QSoundEffect(this);
    sfxWin->setSource(QUrl("qrc:/sounds/win.wav"));
    sfxWin->setVolume(0.9);
}



// // ----- LIVES RENDER -----
// void MainWindow::drawLives(QImage &img) {
//     // Bottom-left, 3 red squares (grey if lost)
//     int size = cellSize / 2;
//     int margin = 6;
//     int y = img.height() - size - margin;

//     for (int i = 0; i < 3; ++i) {
//         int x = margin + i * (size + 6);
//         QColor c = (i < lives) ? QColor(255,60,60) : QColor(80,80,80);
//         for (int py = y; py < y + size; ++py)
//             for (int px = x; px < x + size; ++px)
//                 img.setPixel(px, py, c.rgb());
//     }
// }

// ----- GAME FLOW -----
void MainWindow::startGame(int level) {
    if (levels.isEmpty()) setupLevels();

    currentLevel = level;
    initMaze(currentLevel);
    initFood();
    initEnemies();
    bgMusic->play();


    lives = 3;

    // Ask player name at game start (only once per new session)
    static bool asked = false;
    static QString playerName;

    if (!asked) {
        asked = true;
        playerName = QInputDialog::getText(this, "Enter Name", "Your Name:");
        if (playerName.trimmed().isEmpty())
            playerName = "Player";
    }
    currentPlayerName = playerName;


    mouthOpen = 0;
    updateHUD();  // ✅ show correct Level/Lives/Score immediately

    if (!gameTimer->isActive()) gameTimer->start(120);

    // hide overlay if visible
    if (menuOverlay) menuOverlay->hide();

    // make sure game receives key events
    this->setFocus();
    frame->setFocus();

    // position exit button
    exitBtn->show();
    exitBtn->raise();
}

void MainWindow::stopGame() {
    if (gameTimer->isActive()) gameTimer->stop();
}

void MainWindow::ensureMenuOverlay() {
    if (menuOverlay) return;

    menuOverlay = new QWidget(this);
    menuOverlay->setStyleSheet("background: rgba(0,0,0,0.65);");

    auto container = new QWidget(menuOverlay);
    container->setStyleSheet("background: #111; border: 2px solid #444; border-radius: 10px;");

    auto v = new QVBoxLayout(container);
    auto title = new QLabel("Select Level", container);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: white; font-size: 22px; margin: 8px 0;");
    v->addWidget(title);

    auto grid = new QGridLayout();
    btnLvl1 = new QPushButton("Level 1", container);
    btnLvl2 = new QPushButton("Level 2", container);
    btnLvl3 = new QPushButton("Level 3", container);
    btnLvl4 = new QPushButton("Level 4", container);
    btnLvl1->setMinimumHeight(40);
    btnLvl2->setMinimumHeight(40);
    btnLvl3->setMinimumHeight(40);
    btnLvl4->setMinimumHeight(40);
    grid->addWidget(btnLvl1, 0, 0);
    grid->addWidget(btnLvl2, 0, 1);
    grid->addWidget(btnLvl3, 1, 0);
    grid->addWidget(btnLvl4, 1, 1);
    v->addLayout(grid);

    btnMenuExit = new QPushButton("Exit", container);
    btnMenuExit->setMinimumHeight(36);
    v->addWidget(btnMenuExit);

    QPushButton *btnLeaderboard = new QPushButton("Leaderboard", container);
    btnLeaderboard->setMinimumHeight(36);
    v->addWidget(btnLeaderboard);

    connect(btnLeaderboard, &QPushButton::clicked, this, [this]() {
        if (menuOverlay) menuOverlay->hide();   // hide menu so dialog is visible
        showLeaderboard();
        if (menuOverlay) menuOverlay->show();   // show it back
    });



    auto outer = new QVBoxLayout(menuOverlay);
    outer->addStretch();
    outer->addWidget(container, 0, Qt::AlignHCenter);
    outer->addStretch();

    // connections
    connect(btnLvl1, &QPushButton::clicked, this, [this](){ startGame(1); });
    connect(btnLvl2, &QPushButton::clicked, this, [this](){ startGame(2); });
    connect(btnLvl3, &QPushButton::clicked, this, [this](){ startGame(3); });
    connect(btnLvl4, &QPushButton::clicked, this, [this](){ startGame(4); });
    connect(btnMenuExit, &QPushButton::clicked, this, [this](){ close(); });

    // overlay shouldn’t take focus away forever
    btnLvl1->setFocusPolicy(Qt::StrongFocus);
    btnLvl2->setFocusPolicy(Qt::StrongFocus);
    btnLvl3->setFocusPolicy(Qt::StrongFocus);
    btnLvl4->setFocusPolicy(Qt::StrongFocus);

    positionOverlay();
    menuOverlay->hide();
}

void MainWindow::positionOverlay() {
    if (!menuOverlay) return;
    // Cover the frame area (center widget)
    const QPoint topLeft = frame->mapTo(this, QPoint(0,0));
    menuOverlay->setGeometry(QRect(topLeft, frame->size()));
    menuOverlay->raise();
}

void MainWindow::showLevelSelect() {
    stopGame();           // stop timer when showing menu
    ensureMenuOverlay();
    positionOverlay();
    menuOverlay->show();
    menuOverlay->raise();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    // keep exit button pinned to top-right of MainWindow
    exitBtn->move(width() - exitBtn->width() - 1, 1);
    positionOverlay();
    QMainWindow::resizeEvent(event);
}

// =====================
// 🎨 RETRO UI IMPLEMENTATION
// =====================

// ----- RetroLabel -----
RetroLabel::RetroLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent), bright(true)
{
    setAlignment(Qt::AlignCenter);
    setStyleSheet(R"(
        color: #FFFFFF;
        font-family: 'Press Start 2P', monospace;
        font-size: 18px;
        font-weight: bold;
        letter-spacing: 1px;
        background-color: #222222;
        border: 3px solid #555555;
        border-radius: 6px;
        padding: 8px 16px;
        box-shadow: 2px 2px 0 #000000;
    )");

    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, &RetroLabel::toggleGlow);
    blinkTimer->start(600);
}

void RetroLabel::toggleGlow()
{
    bright = !bright;
    if (bright)
        setStyleSheet(R"(
            color: #FFFFFF;
            font-family: 'Press Start 2P', monospace;
            font-size: 18px;
            font-weight: bold;
            letter-spacing: 1px;
            background-color: #222222;
            border: 3px solid #777777;
            border-radius: 6px;
            padding: 8px 16px;
            box-shadow: 2px 2px 0 #000000;
        )");
    else
        setStyleSheet(R"(
            color: #CCCCCC;
            font-family: 'Press Start 2P', monospace;
            font-size: 18px;
            font-weight: bold;
            letter-spacing: 1px;
            background-color: #1C1C1C;
            border: 3px solid #555555;
            border-radius: 6px;
            padding: 8px 16px;
            box-shadow: 2px 2px 0 #000000;
        )");
}



// ----- GameHUD -----
// ----- GameHUD -----
GameHUD::GameHUD(QWidget *parent) : QWidget(parent)
{
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 10, 20, 10);
    layout->setSpacing(30);

    // Create labels
    scoreLabel = new QLabel("SCORE: 0000", this);
    livesLabel = new QLabel("LIVES: ❤❤❤", this);  // ❤️ default lives displayed
    levelLabel = new QLabel("LEVEL: 1", this);

    // Apply consistent Minecraft-like styling
    updateStyle(scoreLabel, "yellow");
    updateStyle(livesLabel, "red");
    updateStyle(levelLabel, "dodgerblue");

    layout->addWidget(scoreLabel);
    layout->addWidget(livesLabel);
    layout->addWidget(levelLabel);
    layout->addStretch(1);

    setLayout(layout);
    setStyleSheet(R"(
        background-color: #2E2E2E;
        border-bottom: 4px solid #555555;
    )");
}

// Apply style individually for color customization
void GameHUD::updateStyle(QLabel *lbl, const QString &colorName)
{
    lbl->setStyleSheet(QString(R"(
        color: %1;
        font-family: 'Press Start 2P', monospace;
        font-size: 18px;
        font-weight: bold;
        background-color: #3A3A3A;
        border: 2px solid #5A5A5A;
        border-radius: 6px;
        padding: 10px 16px;
        margin: 4px;
    )").arg(colorName));
}

// Update values dynamically
void GameHUD::setScore(int value)
{
    scoreLabel->setText(QString("SCORE: %1").arg(value, 4, 10, QLatin1Char('0')));
}

void GameHUD::setLives(int value)
{
    // Build hearts string dynamically
    QString hearts;
    for (int i = 0; i < value; ++i)
        hearts += "❤ ";
    if (value <= 0) hearts = "💀";

    livesLabel->setText(QString("LIVES: %1").arg(hearts.trimmed()));
}

void GameHUD::setLevel(int value)
{
    levelLabel->setText(QString("LEVEL: %1").arg(value));
}

void MainWindow::handleWin()
{
    stopGame();
    saveScore(currentPlayerName, score);


    QMessageBox msg(this);
    msg.setWindowTitle("Level Cleared!");
    msg.setText(QString("🎉 You cleared Level %1!\nYour score: %2")
                    .arg(currentLevel)
                    .arg(score));
    msg.setIcon(QMessageBox::Information);
    msg.addButton("Next Level", QMessageBox::AcceptRole);
    msg.addButton("Quit", QMessageBox::RejectRole);

    int ret = msg.exec();

    if (ret == 0) {
        currentLevel++;
        if (currentLevel > levels.size()) {
            QMessageBox::information(this, "Victory!",
                                     "🏆 You cleared all levels! Game Complete!");
            showLevelSelect();
        } else {
            startGame(currentLevel);
        }
    }
    else {
        showLevelSelect();
    }
}

void MainWindow::handleGameOver()
{
    stopGame();
    saveScore(currentPlayerName, score);

    QMessageBox msg(this);
    msg.setWindowTitle("Game Over");
    msg.setText(QString("💀 You lost all lives!\nFinal score: %1").arg(score));
    msg.setIcon(QMessageBox::Critical);
    msg.addButton("Retry Level", QMessageBox::AcceptRole);
    msg.addButton("Quit to Menu", QMessageBox::RejectRole);

    int ret = msg.exec();

    if (ret == 0) {
        startGame(currentLevel);
    } else {
        showLevelSelect();
    }
}





