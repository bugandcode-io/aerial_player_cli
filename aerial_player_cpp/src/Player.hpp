#pragma once

#include <string>
#include <memory>
#include "Playlist.hpp"

class Player {
public:
    Player();
    ~Player();

    bool init();
    void shutdown();

    void setPlaylist(std::shared_ptr<Playlist> playlist);

    bool playCurrent();
    bool playNext();
    bool playPrevious();
    void pause();
    void resume();
    void stop();

    bool isPlaying() const;
    bool isPaused() const;

    std::string nowPlaying() const;

private:
    std::shared_ptr<Playlist> playlist_;
    bool initialized_ = false;
    bool paused_ = false;
};
