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
    playerDirX(1), playerDirY(0)
{
    frame = new MyLabel(this);
    frame->setFixedSize(cols * cellSize, rows * cellSize);
    frame->setStyleSheet("background:black");
    setCentralWidget(frame);

    initMaze();
    initFood();
    initEnemies();

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    timer->start(120); // slightly slower tick for overall game pace
}

MainWindow::~MainWindow() {}

void MainWindow::initMaze()
{
    maze.clear();
    maze.resize(rows);
    for (int y = 0; y < rows; ++y) {
        maze[y].resize(cols);
        for (int x = 0; x < cols; ++x) {
            if (x==0 || y==0 || x==cols-1 || y==rows-1 ||
                (y==3 && x>2 && x<cols-3) ||
                (x==3 && y>2 && y<rows-3) ||
                (y==6 && x>5 && x<cols-4) ||
                (x==6 && y>5 && y<rows-4) ||
                (y==16 && x>2 && x<cols-2) ||
                (x==15 && y>5 && y<rows-6) ||
                (x==17 && y>4 && y<rows-3))
                maze[y][x] = 1;
            else
                maze[y][x] = 0;
        }
    }
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
        1,               // moveInterval: slower than player (every 1 frames)
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
        1,       // even slower: every 1 frames
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

    // FOOD
    for (auto &f : food)
        drawBlock(img, f.first, f.second, cellSize, Qt::white);

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
