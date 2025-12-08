// src/ui_desktop/MainWindow.hpp
#pragma once

#include <QMainWindow>

class QListWidget;
class QPushButton;
class QSlider;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void openFile(const QString &path);

private slots:
    void onPlayPause();
    void onNext();
    void onPrev();
    void onSeek(int value);
    void onVolumeChange(int value);

private:
    QListWidget *playlistWidget;
    QPushButton *playPauseBtn;
    QPushButton *nextBtn;
    QPushButton *prevBtn;
    QSlider *positionSlider;
    QSlider *volumeSlider;

    // Your existing audio engine:
    // Player player;
};
