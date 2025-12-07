#pragma once

#include <memory>
#include <string>

class Playlist;

class Player {
public:
    Player();
    ~Player();

    bool init();
    void shutdown();

    // Attach a playlist to this player
    void setPlaylist(std::shared_ptr<Playlist> playlist);

    // Playback controls
    bool playCurrent();
    bool playNext();
    bool playPrevious();
    void pause();
    void resume();
    void stop();
    bool isPlaying() const;
    bool isPaused() const;

    // Currently playing track (full path as string)
    std::string nowPlaying() const;

    // Position / seeking (in seconds)
    double getPositionSeconds() const;
    bool   seekTo(double seconds);
    bool   seekBy(double deltaSeconds);

    // Volume controls (0–100%)
    void setVolumePercent(int percent);
    void changeVolumePercent(int delta);  // e.g. +5, -10
    int  getVolumePercent() const;

private:
    bool initialized_ = false;
    bool paused_      = false;

    std::shared_ptr<Playlist> playlist_;

    int volumePercent_ = 100;  // default volume
};
