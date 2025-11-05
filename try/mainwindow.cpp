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
    playerDirX(1), playerDirY(0), currentLevel(3)
{
    frame = new MyLabel(this);
    frame->setFixedSize(cols * cellSize, rows * cellSize);
    frame->setStyleSheet("background:black");
    setCentralWidget(frame);

    setupLevels();          // <-- define all mazes here
    initMaze(currentLevel); // <-- load initial maze layout
    initFood();
    initEnemies();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(120); // slightly slower tick for overall game pace
}

MainWindow::~MainWindow() {}

void MainWindow::setupLevels() {
    QVector<QPoint> lvl1 = {
        QPoint(1, 2),
        QPoint(2, 2),
        QPoint(3, 2),
        QPoint(3, 3),
        QPoint(3, 4),
        QPoint(2, 4),
        QPoint(6, 2),
        QPoint(6, 3),
        QPoint(6, 4),
        QPoint(7, 4),
        QPoint(8, 4),
        QPoint(8, 3),
        QPoint(10, 10),
        QPoint(11, 10),
        QPoint(12, 10),
        QPoint(13, 10),
        QPoint(15, 10),
        QPoint(14, 10),
        QPoint(15, 11),
        QPoint(15, 12),
        QPoint(13, 13),
        QPoint(14, 13),
        QPoint(15, 13),
        QPoint(10, 13),
        QPoint(9, 13),
        QPoint(8, 13),
        QPoint(8, 12),
        QPoint(8, 11),
        QPoint(8, 10),
        QPoint(9, 10),
        QPoint(10, 3),
        QPoint(11, 3),
        QPoint(11, 4),
        QPoint(11, 5),
        QPoint(11, 6),
        QPoint(10, 6),
        QPoint(9, 6),
        QPoint(16, 2),
        QPoint(15, 2),
        QPoint(15, 3),
        QPoint(15, 4),
        QPoint(15, 5),
        QPoint(16, 5),
        QPoint(17, 5),
        QPoint(18, 5),
        QPoint(18, 4),
        QPoint(18, 3),
        QPoint(19, 3),
        QPoint(20, 3),
        QPoint(20, 4),
        QPoint(13, 4),
        QPoint(13, 5),
        QPoint(13, 6),
        QPoint(13, 7),
        QPoint(14, 7),
        QPoint(15, 7),
        QPoint(16, 7),
        QPoint(17, 7),
        QPoint(18, 7),
        QPoint(20, 6),
        QPoint(20, 7),
        QPoint(22, 3),
        QPoint(23, 3),
        QPoint(22, 4),
        QPoint(22, 5),
        QPoint(1, 6),
        QPoint(2, 6),
        QPoint(3, 6),
        QPoint(3, 7),
        QPoint(3, 9),
        QPoint(3, 10),
        QPoint(2, 10),
        QPoint(1, 10),
        QPoint(5, 8),
        QPoint(5, 9),
        QPoint(5, 10),
        QPoint(5, 11),
        QPoint(6, 11),
        QPoint(6, 12),
        QPoint(6, 13),
        QPoint(5, 13),
        QPoint(4, 13),
        QPoint(2, 13),
        QPoint(3, 13),
        QPoint(2, 12),
        QPoint(7, 8),
        QPoint(7, 7),
        QPoint(7, 6),
        QPoint(6, 6),
        QPoint(5, 6),
        QPoint(17, 13),
        QPoint(18, 13),
        QPoint(19, 13),
        QPoint(20, 13),
        QPoint(20, 12),
        QPoint(20, 11),
        QPoint(23, 12),
        QPoint(20, 10),
        QPoint(21, 10),
        QPoint(22, 10),
        QPoint(9, 8),
        QPoint(10, 8),
        QPoint(11, 8),
        QPoint(1, 15),
        QPoint(2, 15),
        QPoint(3, 15),
        QPoint(4, 15),
        QPoint(4, 16),
        QPoint(4, 17),
        QPoint(4, 18),
        QPoint(5, 18),
        QPoint(6, 18),
        QPoint(2, 18),
        QPoint(2, 17),
        QPoint(2, 19),
        QPoint(2, 20),
        QPoint(3, 20),
        QPoint(4, 20),
        QPoint(4, 21),
        QPoint(7, 18),
        QPoint(7, 19),
        QPoint(6, 22),
        QPoint(6, 21),
        QPoint(7, 21),
        QPoint(8, 21),
        QPoint(9, 21),
        QPoint(9, 16),
        QPoint(9, 17),
        QPoint(9, 18),
        QPoint(9, 15),
        QPoint(7, 15),
        QPoint(8, 15),
        QPoint(4, 22),
        QPoint(14, 8),
        QPoint(11, 15),
        QPoint(12, 15),
        QPoint(13, 15),
        QPoint(14, 15),
        QPoint(14, 16),
        QPoint(14, 17),
        QPoint(13, 17),
        QPoint(12, 17),
        QPoint(12, 18),
        QPoint(12, 19),
        QPoint(11, 19),
        QPoint(17, 14),
        QPoint(17, 15),
        QPoint(17, 16),
        QPoint(17, 17),
        QPoint(16, 17),
        QPoint(16, 18),
        QPoint(16, 19),
        QPoint(15, 19),
        QPoint(15, 20),
        QPoint(17, 12),
        QPoint(17, 11),
        QPoint(17, 10),
        QPoint(15, 21),
        QPoint(14, 21),
        QPoint(13, 21),
        QPoint(19, 17),
        QPoint(19, 18),
        QPoint(19, 19),
        QPoint(18, 19),
        QPoint(19, 16),
        QPoint(20, 16),
        QPoint(21, 16),
        QPoint(22, 16),
        QPoint(22, 17),
        QPoint(20, 21),
        QPoint(21, 21),
        QPoint(19, 21),
        QPoint(21, 18),
        QPoint(22, 18),
        QPoint(21, 19),
        QPoint(21, 20),
        QPoint(18, 21),
        QPoint(18, 22),
        QPoint(22, 12),
        QPoint(22, 13),
        QPoint(22, 14)
    };

    QVector<QPoint> lvl2 = {
        QPoint(5, 2),
        QPoint(5, 3),
        QPoint(5, 4),
        QPoint(5, 5),
        QPoint(4, 5),
        QPoint(2, 2),
        QPoint(2, 3),
        QPoint(3, 2),
        QPoint(3, 3),
        QPoint(3, 5),
        QPoint(3, 6),
        QPoint(3, 7),
        QPoint(2, 9),
        QPoint(3, 9),
        QPoint(3, 10),
        QPoint(3, 11),
        QPoint(3, 12),
        QPoint(2, 12),
        QPoint(16, 10),
        QPoint(15, 9),
        QPoint(14, 8),
        QPoint(13, 8),
        QPoint(12, 9),
        QPoint(11, 10),
        QPoint(16, 14),
        QPoint(15, 15),
        QPoint(11, 14),
        QPoint(12, 15),
        QPoint(13, 16),
        QPoint(14, 16),
        QPoint(16, 11),
        QPoint(16, 13),
        QPoint(11, 11),
        QPoint(11, 13),
        QPoint(17, 11),
        QPoint(18, 11),
        QPoint(17, 13),
        QPoint(18, 13),
        QPoint(5, 11),
        QPoint(6, 11),
        QPoint(7, 11),
        QPoint(9, 8),
        QPoint(9, 9),
        QPoint(9, 10),
        QPoint(9, 11),
        QPoint(8, 11),
        QPoint(5, 9),
        QPoint(6, 9),
        QPoint(7, 9),
        QPoint(7, 6),
        QPoint(7, 7),
        QPoint(6, 7),
        QPoint(6, 5),
        QPoint(7, 5),
        QPoint(8, 3),
        QPoint(8, 2),
        QPoint(10, 2),
        QPoint(9, 2),
        QPoint(11, 2),
        QPoint(12, 2),
        QPoint(13, 2),
        QPoint(11, 3),
        QPoint(11, 4),
        QPoint(10, 4),
        QPoint(10, 5),
        QPoint(10, 6),
        QPoint(14, 5),
        QPoint(15, 5),
        QPoint(15, 4),
        QPoint(15, 3),
        QPoint(16, 3),
        QPoint(17, 3),
        QPoint(17, 2),
        QPoint(18, 2),
        QPoint(19, 2),
        QPoint(19, 3),
        QPoint(19, 4),
        QPoint(19, 5),
        QPoint(18, 5),
        QPoint(17, 5),
        QPoint(19, 11),
        QPoint(20, 11),
        QPoint(19, 13),
        QPoint(20, 13),
        QPoint(22, 2),
        QPoint(22, 3),
        QPoint(21, 3),
        QPoint(21, 4),
        QPoint(21, 5),
        QPoint(22, 5),
        QPoint(22, 6),
        QPoint(22, 7),
        QPoint(21, 7),
        QPoint(20, 7),
        QPoint(18, 7),
        QPoint(19, 7),
        QPoint(22, 9),
        QPoint(22, 10),
        QPoint(20, 9),
        QPoint(21, 9),
        QPoint(19, 9),
        QPoint(22, 13),
        QPoint(22, 14),
        QPoint(22, 15),
        QPoint(22, 16),
        QPoint(21, 16),
        QPoint(20, 16),
        QPoint(19, 16),
        QPoint(18, 16),
        QPoint(18, 18),
        QPoint(18, 15),
        QPoint(19, 15),
        QPoint(6, 14),
        QPoint(5, 15),
        QPoint(4, 16),
        QPoint(3, 17),
        QPoint(2, 18),
        QPoint(9, 14),
        QPoint(8, 15),
        QPoint(7, 16),
        QPoint(6, 17),
        QPoint(5, 18),
        QPoint(4, 19),
        QPoint(4, 20),
        QPoint(4, 21),
        QPoint(6, 12),
        QPoint(1, 18),
        QPoint(2, 14),
        QPoint(2, 13),
        QPoint(2, 15),
        QPoint(14, 12),
        QPoint(13, 12),
        QPoint(13, 11),
        QPoint(14, 11),
        QPoint(14, 13),
        QPoint(13, 13),
        QPoint(8, 19),
        QPoint(7, 19),
        QPoint(7, 20),
        QPoint(14, 22),
        QPoint(15, 22),
        QPoint(16, 22),
        QPoint(16, 20),
        QPoint(16, 21),
        QPoint(16, 19),
        QPoint(20, 18),
        QPoint(20, 19),
        QPoint(20, 20),
        QPoint(19, 20),
        QPoint(18, 20),
        QPoint(18, 21),
        QPoint(18, 22),
        QPoint(19, 22),
        QPoint(20, 22),
        QPoint(21, 22),
        QPoint(22, 22),
        QPoint(22, 18),
        QPoint(22, 19),
        QPoint(22, 20),
        QPoint(10, 19),
        QPoint(9, 19),
        QPoint(11, 21),
        QPoint(11, 19),
        QPoint(11, 20),
        QPoint(11, 18),
        QPoint(11, 17),
        QPoint(10, 17),
        QPoint(11, 22),
        QPoint(12, 20),
        QPoint(12, 19),
        QPoint(12, 21),
        QPoint(2, 21),
        QPoint(3, 21),
        QPoint(2, 22),
        QPoint(3, 22),
        QPoint(4, 22),
        QPoint(7, 21),
        QPoint(7, 22),
        QPoint(8, 22),
        QPoint(9, 22),
        QPoint(9, 21),
        QPoint(16, 17),
        QPoint(17, 17),
        QPoint(18, 17),
        QPoint(16, 18),
        QPoint(15, 19),
        QPoint(14, 19),
        QPoint(13, 5),
        QPoint(13, 6),
        QPoint(14, 6)
    };

    QVector<QPoint> lvl3 = {
        QPoint(4, 4), QPoint(5, 4), QPoint(6, 4),
        QPoint(4, 5), QPoint(6, 5),
        QPoint(4, 6), QPoint(5, 6), QPoint(6, 6),
        QPoint(10, 10), QPoint(11, 10), QPoint(12, 10),
        QPoint(10, 11), QPoint(12, 11),
        QPoint(10, 12), QPoint(11, 12), QPoint(12, 12),
        QPoint(15, 15), QPoint(16, 15), QPoint(17, 15)
    };

    levels = { lvl1, lvl2, lvl3 };
}

void MainWindow::initMaze(int levelNumber)
{
    maze.clear();
    maze.resize(rows);
    for (int y = 0; y < rows; ++y) {
        maze[y].resize(cols, 0);
    }

    // Add boundary walls automatically
    for (int x = 0; x < cols; x++) {
        maze[0][x] = 1;
        maze[rows - 1][x] = 1;
    }
    for (int y = 0; y < rows; y++) {
        maze[y][0] = 1;
        maze[y][cols - 1] = 1;
    }

    // Load current level walls
    if (levelNumber <= 0 || levelNumber > levels.size())
        levelNumber = 1;

    const QVector<QPoint> &currentWalls = levels[levelNumber - 1];
    for (const QPoint &p : currentWalls) {
        if (p.x() >= 0 && p.x() < cols && p.y() >= 0 && p.y() < rows)
            maze[p.y()][p.x()] = 1;
    }

    // Reset player position
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

    // Define habitat quarters (grid coordinates)
    QRect topLeft(0, 0, cols/2, rows/2);
    QRect topRight(cols/2, 0, cols - cols/2, rows/2);
    QRect bottomLeft(0, rows/2, cols/2, rows - rows/2);
    QRect bottomRight(cols/2, rows/2, cols - cols/2, rows - rows/2);

    // Two SMART enemies (A* when Pac-Man is in their quarter), two SIMPLE enemies
    // Smart #1: top-left
    enemies.push_back(Enemy{
        10, 2,            // x, y
        1, 0,            // dx, dy (idle/simple direction)
        Qt::red,         // color
        EnemyType::Smart,// type
        topLeft,         // habitat
        1,               // moveInterval: slower than player (every 2 frames)
        0                // cooldown
    });

    // Smart #2: top-right
    enemies.push_back(Enemy{
        cols-2, 1,
        -1, 0,
        Qt::magenta,
        EnemyType::Smart,
        topRight,
        1,
        1  // staggered cooldown so they don't move same frame
    });

    // Simple #1: bottom-left (horizontal patrol)
    enemies.push_back(Enemy{
        1, rows-2,
        1, 0,
        Qt::green,
        EnemyType::Simple,
        QRect(), // unused
        1,       // even slower: every 3 frames
        0
    });

    // Simple #2: bottom-right (vertical patrol)
    enemies.push_back(Enemy{
        cols-2, rows-2,
        0, -1,
        Qt::blue,
        EnemyType::Simple,
        QRect(), // unused
        1,
        1
    });
}

void drawBlock(QImage &img, int gx, int gy, int cellSize, QColor color)
{
    for(int py = gy * cellSize; py < (gy+1)*cellSize; py++)
        for(int px = gx * cellSize; px < (gx+1)*cellSize; px++)
            img.setPixel(px, py, color.rgb());
}

void drawFood(QImage &img, int gx, int gy, int cellSize, QColor color)
{
    int dotSize = cellSize / 4;  // small dot (1/4th of cell)
    int startX = gx * cellSize + (cellSize - dotSize) / 2;
    int startY = gy * cellSize + (cellSize - dotSize) / 2;

    for (int y = startY; y < startY + dotSize; ++y)
        for (int x = startX; x < startX + dotSize; ++x)
            img.setPixel(x, y, color.rgb());
}

// --- A* helpers ---
bool MainWindow::isWalkable(int x, int y) const {
    if (x < 0 || y < 0 || x >= cols || y >= rows) return false;
    return maze[y][x] == 0;
}

// Compute next step towards (tx,ty) from (sx,sy) using A* with 4-neighbour moves.
// Returns true and sets (nx,ny) if a step exists; otherwise returns false.
bool MainWindow::aStarNextStep(int sx, int sy, int tx, int ty, int &nx, int &ny) {
    if (sx == tx && sy == ty) return false; // already there

    auto start = qMakePair(sx, sy);
    auto goal  = qMakePair(tx, ty);

    struct Node {
        QPair<int,int> p;
        int f;
        int g;
    };

    auto heuristic = [&](const QPair<int,int> &a, const QPair<int,int> &b)->int {
        return std::abs(a.first - b.first) + std::abs(a.second - b.second); // Manhattan
    };

    // Min-heap by f then g
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

        int tentative_g = gScore[current] + 1; // cost grid 1
        bool isBetter = false;
        if (!gScore.contains(np)) {
            gScore[np] = std::numeric_limits<int>::max();
        }
        if (tentative_g < gScore[np]) {
            isBetter = true;
            cameFrom[np] = current;
            gScore[np] = tentative_g;
            int f = tentative_g + heuristic(np, goal);
            if (!openSet.contains(np)) {
                open.push({np, f, tentative_g});
                openSet.insert(np);
            } else {
                // Reinsert to update priority
                open.push({np, f, tentative_g});
            }
        }
    };

    while(!open.empty()) {
        Node cur = open.top(); open.pop();
        QPair<int,int> current = cur.p;

        if (closedSet.contains(current)) continue; // skip outdated
        closedSet.insert(current);
        openSet.remove(current);

        if (current == goal) {
            // reconstruct to find immediate next step
            QPair<int,int> step = current;
            // backtrack until predecessor is start
            while (cameFrom.contains(step) && cameFrom[step] != start) {
                step = cameFrom[step];
            }
            if (cameFrom.contains(step) || step == goal) {
                nx = step.first;
                ny = step.second;
                return true;
            } else {
                return false;
            }
        }

        int cx = current.first, cy = current.second;
        // 4-directional neighbours (no diagonals)
        pushNeighbor(current, cx+1, cy);
        pushNeighbor(current, cx-1, cy);
        pushNeighbor(current, cx, cy+1);
        pushNeighbor(current, cx, cy-1);
    }

    return false; // no path
}

void MainWindow::moveEnemies()
{
    QPoint playerPt(playerX, playerY);

    for (auto &e : enemies) {
        // handle speed cooldown
        if (e.cooldown > 0) {
            e.cooldown--;
            continue;
        }
        e.cooldown = e.moveInterval; // reset for next moves

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
                // fall-through to simple if no path
            }
            // Smart enemy idle/simple movement when Pac-Man not in habitat
            int tryx = e.x + e.dx;
            int tryy = e.y + e.dy;
            if (isWalkable(tryx, tryy)) {
                e.x = tryx; e.y = tryy;
            } else {
                // bounce: try reversing axis; if axis blocked, try turning
                if (e.dx != 0) {
                    e.dx = -e.dx;
                    tryx = e.x + e.dx;
                    if (isWalkable(tryx, e.y)) e.x = tryx;
                    else {
                        // attempt vertical detour
                        if (isWalkable(e.x, e.y+1)) { e.dy = 1; e.dx = 0; e.y += 1; }
                        else if (isWalkable(e.x, e.y-1)) { e.dy = -1; e.dx = 0; e.y -= 1; }
                    }
                } else if (e.dy != 0) {
                    e.dy = -e.dy;
                    tryy = e.y + e.dy;
                    if (isWalkable(e.x, tryy)) e.y = tryy;
                    else {
                        // attempt horizontal detour
                        if (isWalkable(e.x+1, e.y)) { e.dx = 1; e.dy = 0; e.x += 1; }
                        else if (isWalkable(e.x-1, e.y)) { e.dx = -1; e.dy = 0; e.x -= 1; }
                    }
                }
            }
        } else {
            // Simple movement (horizontal/vertical bounce)
            int tryx = e.x + e.dx;
            int tryy = e.y + e.dy;
            if (isWalkable(tryx, tryy)) {
                e.x = tryx; e.y = tryy;
            } else {
                // bounce
                if (e.dx != 0) {
                    e.dx = -e.dx;
                    tryx = e.x + e.dx;
                    if (isWalkable(tryx, e.y)) e.x = tryx;
                } else if (e.dy != 0) {
                    e.dy = -e.dy;
                    tryy = e.y + e.dy;
                    if (isWalkable(e.x, tryy)) e.y = tryy;
                }
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
            break;
        }
}

void MainWindow::updateFrame()
{
    mouthOpen = !mouthOpen;
    moveEnemies();
    checkCollisions();

    QImage img(frame->width(), frame->height(), QImage::Format_RGB32);
    img.fill(Qt::black);

    // MAZE
    for(int y=0; y<rows; y++)
        for(int x=0; x<cols; x++)
            if (maze[y][x] == 1)
                drawBlock(img, x, y, cellSize, Qt::darkBlue);

    // FOOD (draw smaller dots)
    for (auto &f : food)
        drawFood(img, f.first, f.second, cellSize, Qt::white);

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
        int cx = px + cellSize/2;
        int cy = py + cellSize/2;

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

    frame->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    int nx = playerX, ny = playerY;

    if(e->key()==Qt::Key_Left)  { nx--; playerDirX=-1; playerDirY=0; }
    if(e->key()==Qt::Key_Right) { nx++; playerDirX=1;  playerDirY=0; }
    if(e->key()==Qt::Key_Up)    { ny--; playerDirX=0;  playerDirY=-1; }
    if(e->key()==Qt::Key_Down)  { ny++; playerDirX=0;  playerDirY=1; }

    // move only if not wall
    if (!maze[ny][nx]) {
        playerX = nx;
        playerY = ny;

        // eat food immediately
        auto cur = qMakePair(playerX, playerY);
        if (food.contains(cur))
            food.remove(cur);
    }
    // timer drives update
}
