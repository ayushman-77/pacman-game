#include "mainwindow.h"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Optional: smoother rendering if using animations or pixmaps
    QSurfaceFormat fmt;
    fmt.setSamples(4);  // anti-aliasing
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow window;
    window.setWindowTitle("Qt Maze Game - Pacman Style");
    window.show();

    return app.exec();
}
