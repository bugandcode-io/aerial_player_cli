#include "MainWindow.h"
#include "DB.hpp"   // uses PlayDatabase (SQLite or JSON)

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDirIterator>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_player(new QMediaPlayer(this)),
      m_audioOutput(new QAudioOutput(this)),
      m_playPauseButton(nullptr),
      m_prevButton(nullptr),
      m_nextButton(nullptr),
      m_modeButton(nullptr),
      m_nowPlaying(nullptr),
      m_currentIndex(-1),
      m_isPlaying(false),
      m_mode(PlaybackMode::Normal),
      m_db(new PlayDatabase("aerial_stats.json"))  // JSON fallback if no SQLite
{
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.1f); // 10% default

    setupUi();
    loadAllMusic();
    startCurrentTrack();

    // When a track finishes, decide what to do based on mode
    connect(m_player, &QMediaPlayer::mediaStatusChanged,
            this, [this](QMediaPlayer::MediaStatus status) {
                if (status == QMediaPlayer::EndOfMedia) {
                    handleEndOfTrack();
                }
            });
}

MainWindow::~MainWindow()
{
    delete m_db;
    m_db = nullptr;
}

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);

    // Simple title
    auto* title = new QLabel("Aerial Player (C:\\all_music)", central);
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // "Now Playing" label
    m_nowPlaying = new QLabel("Now Playing: (nothing loaded)", central);
    m_nowPlaying->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_nowPlaying);

    // Mode button (NORMAL / REPEAT_ONE for now)
    m_modeButton = new QPushButton(central);
    m_modeButton->setMinimumHeight(28);
    connect(m_modeButton, &QPushButton::clicked,
            this, &MainWindow::onModeClicked);
    updateModeButtonLabel();
    mainLayout->addWidget(m_modeButton, 0, Qt::AlignCenter);

    // Transport controls: Prev | Play/Pause | Next
    auto* transportLayout = new QHBoxLayout;

    m_prevButton = new QPushButton("⏮ Prev", central);
    m_prevButton->setMinimumHeight(40);

    m_playPauseButton = new QPushButton("Play", central);
    m_playPauseButton->setMinimumHeight(40);

    m_nextButton = new QPushButton("Next ⏭", central);
    m_nextButton->setMinimumHeight(40);

    transportLayout->addStretch();
    transportLayout->addWidget(m_prevButton);
    transportLayout->addWidget(m_playPauseButton);
    transportLayout->addWidget(m_nextButton);
    transportLayout->addStretch();

    mainLayout->addLayout(transportLayout);

    // Wire up buttons
    connect(m_playPauseButton, &QPushButton::clicked,
            this, &MainWindow::onPlayPause);

    connect(m_prevButton, &QPushButton::clicked,
            this, &MainWindow::onPrev);

    connect(m_nextButton, &QPushButton::clicked,
            this, &MainWindow::onNext);

    setCentralWidget(central);
}

void MainWindow::loadAllMusic()
{
    m_playlist.clear();
    m_currentIndex = -1;

    const QString rootPath = R"(C:\all_music)";

    QDir dir(rootPath);
    if (!dir.exists()) {
        qWarning() << "Music folder does not exist:" << rootPath;
        m_nowPlaying->setText("Now Playing: (folder not found)");
        return;
    }

    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.flac" << "*.ogg";

    QDirIterator it(rootPath,
                    filters,
                    QDir::Files,
                    QDirIterator::Subdirectories);

    while (it.hasNext()) {
        QString filePath = it.next();
        m_playlist << filePath;
    }

    if (m_playlist.isEmpty()) {
        qWarning() << "No audio files found in" << rootPath;
        m_nowPlaying->setText("Now Playing: (no audio files)");
    } else {
        m_currentIndex = 0;
        qDebug() << "Loaded" << m_playlist.size() << "tracks from" << rootPath;
        updateNowPlayingLabel();
    }
}

QString MainWindow::currentTrackPath() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size())
        return {};
    return m_playlist[m_currentIndex];
}

void MainWindow::startCurrentTrack()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size()) {
        qWarning() << "startCurrentTrack: no valid track index";
        m_nowPlaying->setText("Now Playing: (invalid track index)");
        return;
    }

    const QString path = m_playlist[m_currentIndex];
    qDebug() << "Starting track:" << path;

    m_player->setSource(QUrl::fromLocalFile(path));
    updateNowPlayingLabel();

    if (m_isPlaying) {
        // Log a play when we actually start playback
        if (m_db) {
            m_db->logPlay(path.toStdString());
        }
        m_player->play();
    }
}

void MainWindow::updateNowPlayingLabel()
{
    if (!m_nowPlaying) return;

    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size()) {
        m_nowPlaying->setText("Now Playing: (nothing)");
        return;
    }

    const QString path = m_playlist[m_currentIndex];
    QFileInfo info(path);
    const QString fileName = info.fileName();

    m_nowPlaying->setText(
        QString("Now Playing: %1 (%2/%3)")
            .arg(fileName)
            .arg(m_currentIndex + 1)
            .arg(m_playlist.size()));
}

void MainWindow::updateModeButtonLabel()
{
    if (!m_modeButton) return;

    QString modeText;
    switch (m_mode) {
    case PlaybackMode::Normal:
        modeText = "Mode: NORMAL";
        break;
    case PlaybackMode::RepeatOne:
        modeText = "Mode: REPEAT ONE";
        break;
    case PlaybackMode::Random:
        modeText = "Mode: RANDOM (TODO)";
        break;
    case PlaybackMode::Shuffle:
        modeText = "Mode: SHUFFLE (TODO)";
        break;
    }

    m_modeButton->setText(modeText);
}

void MainWindow::handleEndOfTrack()
{
    if (m_playlist.isEmpty())
        return;

    // Natural end of track → finished
    const QString finishedPath = currentTrackPath();
    if (m_db && !finishedPath.isEmpty()) {
        m_db->logFinished(finishedPath.toStdString());
    }

    switch (m_mode) {
    case PlaybackMode::Normal:
        // NORMAL → go to next track in playlist
        onNext();
        break;

    case PlaybackMode::RepeatOne:
        // REPEAT_ONE → stay on same track, restart playback
        m_player->setPosition(0);
        if (m_isPlaying) {
            m_player->play();
        }
        break;

    case PlaybackMode::Random:
        onNext();
        break;

    case PlaybackMode::Shuffle:
        onNext();
        break;
    }
}

void MainWindow::openFile(const QString& path)
{
    qDebug() << "openFile called with:" << path;
    m_playlist.clear();
    m_playlist << path;
    m_currentIndex = 0;
    m_isPlaying = true;
    startCurrentTrack();
}

// ─────────────────────────────
// Slots
// ─────────────────────────────

void MainWindow::onPlayPause()
{
    if (m_playlist.isEmpty()) {
        qWarning() << "PlayPause: playlist is empty";
        return;
    }

    if (!m_isPlaying) {
        m_isPlaying = true;
        m_playPauseButton->setText("Pause");

        if (m_player->source().isEmpty()) {
            startCurrentTrack();
        } else {
            // Resuming an already-loaded track: log play once
            const QString path = currentTrackPath();
            if (m_db && !path.isEmpty()) {
                m_db->logPlay(path.toStdString());
            }
            m_player->play();
        }
    } else {
        m_isPlaying = false;
        m_playPauseButton->setText("Play");
        m_player->pause();
    }
}

void MainWindow::onNext()
{
    if (m_playlist.isEmpty()) return;

    // User-initiated skip of current track
    if (m_isPlaying && m_db) {
        const QString path = currentTrackPath();
        if (!path.isEmpty()) {
            m_db->logSkip(path.toStdString());
        }
    }

    m_currentIndex++;
    if (m_currentIndex >= m_playlist.size())
        m_currentIndex = 0;

    startCurrentTrack();
}

void MainWindow::onPrev()
{
    if (m_playlist.isEmpty()) return;

    // Treat prev as a skip of the current track if playing
    if (m_isPlaying && m_db) {
        const QString path = currentTrackPath();
        if (!path.isEmpty()) {
            m_db->logSkip(path.toStdString());
        }
    }

    m_currentIndex--;
    if (m_currentIndex < 0)
        m_currentIndex = m_playlist.size() - 1;

    startCurrentTrack();
}

void MainWindow::onSeek(int value)
{
    // Not wired yet (no slider)
    Q_UNUSED(value);
}

void MainWindow::onVolumeChange(int value)
{
    m_audioOutput->setVolume(value / 100.0);
}

void MainWindow::onModeClicked()
{
    // For now just toggle between NORMAL and REPEAT_ONE.
    if (m_mode == PlaybackMode::Normal) {
        m_mode = PlaybackMode::RepeatOne;
    } else if (m_mode == PlaybackMode::RepeatOne) {
        m_mode = PlaybackMode::Normal;
    } else {
        // If we ever land in RANDOM/SHUFFLE now, snap back to NORMAL.
        m_mode = PlaybackMode::Normal;
    }

    updateModeButtonLabel();
}
