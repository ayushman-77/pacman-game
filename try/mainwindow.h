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
#include <QFile>
#include <QTextStream>
#include <QInputDialog>

// ---- Qt 6 Multimedia ----
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QSoundEffect>

#include "my_label.h"

// ==============================
// 🎨 MINECRAFT-STYLE UI CLASSES
// ==============================

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

class GameHUD : public QWidget
{
    Q_OBJECT
public:
    explicit GameHUD(QWidget *parent = nullptr);

    void setScore(int value);
    void setLives(int value);
    void setLevel(int value);

private:
    QLabel *scoreLabel;
    QLabel *livesLabel;
    QLabel *levelLabel;

    void updateStyle(QLabel *label, const QString &colorName);
};

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
// 🕹 GAME LOGIC STRUCTURES
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
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;

private:

    // ---------- GRID / PLAYER ----------
    MyLabel *frame;
    int cellSize;
    int rows, cols;
    QVector<QVector<int>> maze;
    QSet<QPair<int,int>> food;
    QVector<Enemy> enemies;

    int playerX, playerY;
    int playerDirX, playerDirY;
    bool mouthOpen;

    QSet<int> pressedKeys;

    // ---------- LEVELS ----------
    QVector<QVector<QPoint>> levels;
    int currentLevel;

    // ---------- TIMER ----------
    QTimer *gameTimer;

    // ---------- STATS ----------
    int lives;
    int score;
    QString currentPlayerName;


    // ---------- UI ----------
    GameHUD *hud;
    CRTOverlay *crtOverlay;
    QPushButton *exitBtn;

    QWidget *menuOverlay = nullptr;
    QPushButton *btnLvl1 = nullptr;
    QPushButton *btnLvl2 = nullptr;
    QPushButton *btnLvl3 = nullptr;
    QPushButton *btnLvl4 = nullptr;
    QPushButton *btnMenuExit = nullptr;

    // ============================
    // 🎵 AUDIO (Qt6 Multimedia)
    // ============================
    QMediaPlayer *bgMusic;
    QAudioOutput *bgOutput;

    QSoundEffect *sfxEat;
    QSoundEffect *sfxDeath;
    QSoundEffect *sfxWin;

    // ============================
    // 🏆 LEADERBOARD SYSTEM
    // ============================
    void saveScore(const QString &name, int score);
    QVector<QPair<QString,int>> loadLeaderboard();
    void showLeaderboard();

    // ---------- INIT ----------
    void setupLevels();
    void initMaze(int levelNumber);
    void initFood();
    void initEnemies();
    void initAudio();

    // ---------- GAMEPLAY ----------
    bool isWalkable(int x, int y) const;
    bool aStarNextStep(int sx, int sy, int tx, int ty, int &nx, int &ny);
    void moveEnemies();
    void checkCollisions();
    void updateFrame();
    void drawLives(QImage &img);

    // ---------- FLOW ----------
    void startGame(int level);
    void stopGame();
    void showLevelSelect();

    void ensureMenuOverlay();
    void positionOverlay();
    void updateHUD();

    // ---------- END STATES ----------
    void handleWin();
    void handleGameOver();
};

#endif // MAINWINDOW_H
