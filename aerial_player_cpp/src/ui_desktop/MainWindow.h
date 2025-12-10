// MainWindow.h
#pragma once

#include <QMainWindow>
#include <QStringList>

class QMediaPlayer;
class QAudioOutput;
class QPushButton;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    void openFile(const QString& path);

private slots:
    void onPlayPause();
    void onNext();
    void onPrev();
    void onSeek(int value);
    void onVolumeChange(int value);
    void onModeClicked();          // <-- NEW: cycle NORMAL / REPEAT_ONE

private:
    // Playback mode:
    //
    //  CURRENT BEHAVIOR:
    //   - NORMAL     → advance to next track in Playlist.
    //   - REPEAT_ONE → stay on the same track and restart playback.
    //   - RANDOM     → currently behaves like NORMAL (TODO: random jump).
    //   - SHUFFLE    → currently behaves like NORMAL (TODO: smart shuffle).
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
    void updateModeButtonLabel();  // <-- NEW
    void handleEndOfTrack();       // <-- NEW: called when QMediaPlayer hits EndOfMedia

    QMediaPlayer*  m_player;
    QAudioOutput*  m_audioOutput;

    QPushButton*   m_playPauseButton;
    QPushButton*   m_prevButton;
    QPushButton*   m_nextButton;
    QPushButton*   m_modeButton;   // <-- NEW
    QLabel*        m_nowPlaying;

    QStringList    m_playlist;
    int            m_currentIndex;
    bool           m_isPlaying;
    PlaybackMode   m_mode;         // <-- NEW
};
