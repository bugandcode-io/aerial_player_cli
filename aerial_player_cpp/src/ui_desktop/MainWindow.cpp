#include "MainWindow.h"

#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDirIterator>
#include <QDebug>
#include <QUrl>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_player(new QMediaPlayer(this)),
      m_audioOutput(new QAudioOutput(this)),
      m_playPauseButton(nullptr),
      m_currentIndex(-1),
      m_isPlaying(false)
{
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(0.1f); // 10% default

    setupUi();
    loadAllMusic();
    startCurrentTrack();
}

void MainWindow::setupUi()
{
    auto* central = new QWidget(this);
    auto* mainLayout = new QVBoxLayout(central);

    // Simple title
    auto* title = new QLabel("Aerial Player (C:\\all_music)", central);
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Play/Pause button
    m_playPauseButton = new QPushButton("Play", central);
    m_playPauseButton->setMinimumHeight(40);
    connect(m_playPauseButton, &QPushButton::clicked,
            this, &MainWindow::onPlayPause);

    mainLayout->addWidget(m_playPauseButton, 0, Qt::AlignCenter);

    // You can add next/prev/seek/volume UI later; for now we just stub slots.

    setCentralWidget(central);
}

void MainWindow::loadAllMusic()
{
    m_playlist.clear();
    m_currentIndex = -1;

    // Hard-coded folder: C:\all_music
    const QString rootPath = R"(C:\all_music)";

    QDir dir(rootPath);
    if (!dir.exists()) {
        qWarning() << "Music folder does not exist:" << rootPath;
        return;
    }

    // Look for basic audio files; expand this list when you want
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
    } else {
        m_currentIndex = 0;
        qDebug() << "Loaded" << m_playlist.size() << "tracks from" << rootPath;
    }
}

void MainWindow::startCurrentTrack()
{
    if (m_currentIndex < 0 || m_currentIndex >= m_playlist.size()) {
        qWarning() << "startCurrentTrack: no valid track index";
        return;
    }

    const QString path = m_playlist[m_currentIndex];
    qDebug() << "Starting track:" << path;

    m_player->setSource(QUrl::fromLocalFile(path));
    if (m_isPlaying) {
        m_player->play();
    }
}

void MainWindow::openFile(const QString& path)
{
    // Optional: if you call this from somewhere else
    qDebug() << "openFile called with:" << path;
    m_playlist.clear();
    m_playlist << path;
    m_currentIndex = 0;
    m_isPlaying = true;
    startCurrentTrack();
}

// ─────────────────────────────
// Slots wired to UI / future UI
// ─────────────────────────────

void MainWindow::onPlayPause()
{
    if (m_playlist.isEmpty()) {
        qWarning() << "PlayPause: playlist is empty";
        return;
    }

    if (!m_isPlaying) {
        // Start playback
        m_isPlaying = true;
        m_playPauseButton->setText("Pause");

        if (m_player->source().isEmpty()) {
            startCurrentTrack();
        }
        m_player->play();
    } else {
        // Pause playback
        m_isPlaying = false;
        m_playPauseButton->setText("Play");
        m_player->pause();
    }
}

void MainWindow::onNext()
{
    if (m_playlist.isEmpty()) return;

    m_currentIndex++;
    if (m_currentIndex >= m_playlist.size())
        m_currentIndex = 0;

    startCurrentTrack();
    if (m_isPlaying)
        m_player->play();
}

void MainWindow::onPrev()
{
    if (m_playlist.isEmpty()) return;

    m_currentIndex--;
    if (m_currentIndex < 0)
        m_currentIndex = m_playlist.size() - 1;

    startCurrentTrack();
    if (m_isPlaying)
        m_player->play();
}

void MainWindow::onSeek(int value)
{
    // value could be a percentage slider (0–100)
    if (m_player->duration() <= 0)
        return;

    qint64 pos = static_cast<qint64>(m_player->duration() * (value / 100.0));
    m_player->setPosition(pos);
}

void MainWindow::onVolumeChange(int value)
{
    // value assumed 0–100
    m_audioOutput->setVolume(value / 100.0);
}
