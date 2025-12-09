#pragma once

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QStringList>

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    // still here if you want to open a specific file later
    void openFile(const QString& path);

private slots:
    void onPlayPause();
    void onNext();
    void onPrev();
    void onSeek(int value);
    void onVolumeChange(int value);

private:
    void setupUi();
    void loadAllMusic();
    void startCurrentTrack();

    QMediaPlayer*  m_player;
    QAudioOutput*  m_audioOutput;
    QPushButton*   m_playPauseButton;

    QStringList    m_playlist;
    int            m_currentIndex;
    bool           m_isPlaying;
};
