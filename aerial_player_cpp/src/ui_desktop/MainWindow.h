#pragma once

#ifdef string
#  undef string
#endif

#include <QMainWindow>
#include <QStringList>

class QMediaPlayer;
class QAudioOutput;
class QPushButton;
class QLabel;
class PlayDatabase;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void openFile(const QString& path);

private slots:
    void onPlayPause();
    void onNext();
    void onPrev();
    void onSeek(int value);          // placeholder for future slider
    void onVolumeChange(int value);  // placeholder
    void onModeClicked();

private:
    enum class PlaybackMode {
        Normal,
        RepeatOne,
        Random,
        Shuffle
    };

    void setupUi();
    void loadAllMusic();
    void startCurrentTrack();
    void updateNowPlayingLabel();
    void updateModeButtonLabel();
    void handleEndOfTrack();

    QString currentTrackPath() const;

    QMediaPlayer*  m_player;
    QAudioOutput*  m_audioOutput;

    QPushButton*   m_playPauseButton;
    QPushButton*   m_prevButton;
    QPushButton*   m_nextButton;
    QPushButton*   m_modeButton;
    QLabel*        m_nowPlaying;

    QStringList    m_playlist;
    int            m_currentIndex;
    bool           m_isPlaying;
    PlaybackMode   m_mode;

    PlayDatabase*  m_db;
};
