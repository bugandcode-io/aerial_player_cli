// src/ui_desktop/MainWindow.cpp
#include "MainWindow.hpp"
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QSlider>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Build UI here
    playlistWidget = new QListWidget;
    playPauseBtn = new QPushButton("Play/Pause");
    nextBtn = new QPushButton("Next");
    prevBtn = new QPushButton("Prev");
    positionSlider = new QSlider(Qt::Horizontal);
    volumeSlider = new QSlider(Qt::Horizontal);

    QWidget *central = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    layout->addWidget(playlistWidget);
    layout->addWidget(positionSlider);
    layout->addWidget(volumeSlider);
    layout->addWidget(playPauseBtn);
    layout->addWidget(prevBtn);
    layout->addWidget(nextBtn);

    central->setLayout(layout);
    setCentralWidget(central);

    // Connect signals/slots here
}

void MainWindow::openFile(const QString &path)
{
    // Example pseudocode
    // player.clear();
    // player.add(path.toStdString());
    // player.play();

    playlistWidget->clear();
    playlistWidget->addItem(path);
}
