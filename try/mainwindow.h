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
#include <QPainter>
#include <QFontDatabase>

#include "my_label.h"  // your label header

// ==============================
// 🎨 MINECRAFT-STYLE UI CLASSES
// ==============================

// ----- Blocky HUD Label with subtle blink -----
class RetroLabel : public QLabel
{
    Q_OBJECT
public:
    explicit RetroLabel(const QString &text, QWidget *parent = nullptr);
private slots:
    void toggleGlow();
private:
    QTimer *blinkTimer;
    bool bright;
};

// ----- Game HUD (Score / Lives / Level) -----
class GameHUD : public QWidget
{
    Q_OBJECT
public:
    explicit GameHUD(QWidget *parent = nullptr);

    // Update HUD values
    void setScore(int value);
    void setLives(int value);
    void setLevel(int value);

private:
    QLabel *scoreLabel;
    QLabel *livesLabel;
    QLabel *levelLabel;

    // helper to apply consistent blocky style
    void updateStyle(QLabel *label, const QString &colorName);
};

// ----- CRT Overlay (subtle scanline layer) -----
class CRTOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit CRTOverlay(QWidget *parent = nullptr)
    {
        setAttribute(Qt::WA_TransparentForMouseEvents);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setOpacity(0.08);
        p.setPen(QPen(Qt::black, 1));
        for (int y = 0; y < height(); y += 3)
            p.drawLine(0, y, width(), y);
    }
};

// ==============================
// 🕹️ GAME LOGIC STRUCTURES
// ==============================

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

// ==============================
// 🧠 MAIN WINDOW
// ==============================

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    // handle keyboard input
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;   // ✅ NEW: stop moving when released
    void resizeEvent(QResizeEvent *event) override;

private:
    // ----- GRID AND PLAYER -----
    MyLabel *frame;
    int cellSize;
    int rows, cols;
    QVector<QVector<int>> maze;
    QSet<QPair<int,int>> food;
    QVector<Enemy> enemies;

    int playerX, playerY;
    int playerDirX, playerDirY;
    bool mouthOpen;

    // ✅ Track held keys for more responsive control
    QSet<int> pressedKeys;

    // ----- LEVEL CONTROL -----
    QVector<QVector<QPoint>> levels; // 4 levels
    int currentLevel;

    // ----- GAME TIMER -----
    QTimer *gameTimer;

    // ----- PLAYER STATS -----
    int lives;
    int score;

    // ----- UI ELEMENTS -----
    GameHUD *hud;           // HUD bar
    CRTOverlay *crtOverlay; // scanline overlay
    QPushButton *exitBtn;   // top-right exit button

    // ----- MENU OVERLAY -----
    QWidget *menuOverlay = nullptr;
    QPushButton *btnLvl1 = nullptr;
    QPushButton *btnLvl2 = nullptr;
    QPushButton *btnLvl3 = nullptr;
    QPushButton *btnLvl4 = nullptr;
    QPushButton *btnMenuExit = nullptr;

    // ----- INITIALIZATION -----
    void setupLevels();              // prepare 4 maze levels
    void initMaze(int levelNumber);  // load selected maze
    void initFood();
    void initEnemies();

    // ----- GAMEPLAY -----
    bool isWalkable(int x, int y) const;
    bool aStarNextStep(int sx, int sy, int tx, int ty, int &nx, int &ny);
    void moveEnemies();
    void checkCollisions();
    void updateFrame();

    // ----- DRAW HELPERS -----
    void drawLives(QImage &img);

    // ----- GAME FLOW -----
    void startGame(int level);
    void stopGame();
    void showLevelSelect();

    // ----- MENU BUILDERS -----
    void ensureMenuOverlay();
    void positionOverlay();

    // ----- UI SYNC -----
    void updateHUD(); // synchronize HUD (score/lives/level)
};

#endif // MAINWINDOW_H
