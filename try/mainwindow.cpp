#include "mainwindow.h"
#include <QTimer>
#include <QRandomGenerator>
#include <QDebug>
#include <queue>
#include <vector>
#include <limits>
#include <QMap>

// Hash for QPair<int,int> so we can use QSet
inline uint qHash(const QPair<int,int> &key, uint seed = 0) {
    return qHash(key.first, seed) ^ (qHash(key.second, seed) << 1);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), cellSize(25), rows(25), cols(25),
    playerX(1), playerY(1), mouthOpen(0),
    playerDirX(1), playerDirY(0), currentLevel(1),
    gameTimer(new QTimer(this)),
    lives(3),
    exitBtn(new QPushButton("Exit", this))
{
    frame = new MyLabel(this);
    frame->setFixedSize(cols * cellSize, rows * cellSize);
    frame->setStyleSheet("background:black");
    setCentralWidget(frame);

    // Timer wiring
    connect(gameTimer, &QTimer::timeout, this, &MainWindow::updateFrame);

    // Global Exit button (outside frame, top-right)
    exitBtn->setFocusPolicy(Qt::NoFocus); // do not steal key focus
    exitBtn->raise();
    // Optional: make a bit smaller & tuck in corner
    QFont f = exitBtn->font();
    f.setPointSize(8);
    exitBtn->setFont(f);
    exitBtn->setFixedSize(50, 22);
    exitBtn->move(width() - exitBtn->width() - 1, 1);

    connect(exitBtn, &QPushButton::clicked, this, [this](){
        stopGame();
        auto r = QMessageBox::question(this, "Exit",
                                       "Exit the game?",
                                       QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::Yes) {
            close();
        } else {
            showLevelSelect();
        }
    });

    // Show the level-select overlay after window shows
    QTimer::singleShot(0, this, [this](){
        ensureMenuOverlay();
        showLevelSelect();
    });
}

MainWindow::~MainWindow() {}

// ======== LEVELS (4 levels) ========

void MainWindow::setupLevels() {
    // Level 1 – custom
    QVector<QPoint> lvl1 = {
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
    QVector<QPoint> lvl4 = {
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

    enemies.push_back(Enemy{ 10, 2, 1, 0, Qt::red,      EnemyType::Smart,  topLeft,   1, 0 });
    enemies.push_back(Enemy{ cols-2, 1, -1,0, Qt::magenta, EnemyType::Smart, topRight,  1, 1 });
    enemies.push_back(Enemy{ 1, rows-2, 1, 0, Qt::green,  EnemyType::Simple, QRect(),   1, 0 });
    enemies.push_back(Enemy{ cols-2, rows-2, 0,-1, Qt::blue,  EnemyType::Simple, QRect(),1, 1 });
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
    auto cur = qMakePair(playerX, playerY);
    if (food.contains(cur)) food.remove(cur);

    for (auto &e : enemies)
        if (e.x == playerX && e.y == playerY) {
            // Reset on collision
            playerX = 1; playerY = 1;
            initEnemies();

            // Life handling (no audio)
            lives -= 1;

            if (lives <= 0) {
                // stop movement, small delay (300ms) then go to level select
                if (gameTimer->isActive()) gameTimer->stop();
                QTimer::singleShot(300, this, [this](){
                    showLevelSelect();
                });
            }
            break;
        }
}

void MainWindow::updateFrame()
{
    mouthOpen = !mouthOpen;
    moveEnemies();
    checkCollisions();

    // win condition (no pellets)
    if (food.isEmpty()) {
        if (gameTimer->isActive()) {
            gameTimer->stop();
            showLevelSelect(); // back to menu on success
            return;
        }
    }

    QImage img(frame->width(), frame->height(), QImage::Format_RGB32);
    img.fill(Qt::black);

    // MAZE
    for(int y=0; y<rows; y++)
        for(int x=0; x<cols; x++)
            if (maze[y][x] == 1)
                drawBlock(img, x, y, cellSize, Qt::darkBlue);

    // FOOD dots (smaller centered)
    for (auto &f : food) {
        int gx = f.first, gy = f.second;
        int dot = cellSize / 4;
        int sx = gx * cellSize + (cellSize - dot) / 2;
        int sy = gy * cellSize + (cellSize - dot) / 2;
        for (int py = sy; py < sy + dot; ++py)
            for (int px = sx; px < sx + dot; ++px)
                img.setPixel(px, py, QColor(Qt::white).rgb());
    }

    // ENEMIES
    for(auto &e : enemies) {
        drawBlock(img, e.x, e.y, cellSize, e.color);
    }

    // PAC-MAN
    drawBlock(img, playerX, playerY, cellSize, Qt::yellow);

    // Wedge mouth raster cut
    if (mouthOpen)
    {
        int px = playerX * cellSize;
        int py = playerY * cellSize;

        for(int y=0; y<cellSize; y++)
            for(int x=0; x<cellSize; x++)
            {
                int rx = px + x, ry = py + y;
                float dx = x - cellSize/2;
                float dy = y - cellSize/2;
                float angle = atan2(dy, dx) * 180.0 / M_PI;

                bool cut = false;

                if (playerDirX == 1 && angle > -30 && angle < 30) cut = true;
                if (playerDirX == -1 && (angle > 150 || angle < -150)) cut = true;
                if (playerDirY == 1 && angle > 60 && angle < 120) cut = true;
                if (playerDirY == -1 && angle > -120 && angle < -60) cut = true;

                if (cut)
                    img.setPixel(rx, ry, QColor(Qt::black).rgb());
            }
    }

    // Draw lives at bottom-left
    drawLives(img);

    frame->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if (menuOverlay && menuOverlay->isVisible()) {
        // ignore key movement while menu is open
        e->ignore();
        return;
    }

    int nx = playerX, ny = playerY;

    if(e->key()==Qt::Key_Left)  { nx--; playerDirX=-1; playerDirY=0; }
    if(e->key()==Qt::Key_Right) { nx++; playerDirX=1;  playerDirY=0; }
    if(e->key()==Qt::Key_Up)    { ny--; playerDirX=0;  playerDirY=-1; }
    if(e->key()==Qt::Key_Down)  { ny++; playerDirX=0;  playerDirY=1; }

    // move only if not wall
    if (!maze[ny][nx]) {
        playerX = nx;
        playerY = ny;

        // eat food immediately (no SFX)
        auto cur = qMakePair(playerX, playerY);
        if (food.contains(cur))
            food.remove(cur);
    }
}

// ----- LIVES RENDER -----
void MainWindow::drawLives(QImage &img) {
    // Bottom-left, 3 red squares (grey if lost)
    int size = cellSize / 2;
    int margin = 6;
    int y = img.height() - size - margin;

    for (int i = 0; i < 3; ++i) {
        int x = margin + i * (size + 6);
        QColor c = (i < lives) ? QColor(255,60,60) : QColor(80,80,80);
        for (int py = y; py < y + size; ++py)
            for (int px = x; px < x + size; ++px)
                img.setPixel(px, py, c.rgb());
    }
}

// ----- GAME FLOW -----
void MainWindow::startGame(int level) {
    if (levels.isEmpty()) setupLevels();

    currentLevel = level;
    initMaze(currentLevel);
    initFood();
    initEnemies();

    lives = 3;
    mouthOpen = 0;

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
