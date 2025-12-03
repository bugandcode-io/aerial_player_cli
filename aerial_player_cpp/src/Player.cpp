#include "Player.hpp"

#include <SDL.h>
#include <SDL_mixer.h>
#include <iostream>

#include "UI.hpp"  // our separate UI layer

Player::Player() = default;

Player::~Player() {
    shutdown();
}

bool Player::init() {
    if (initialized_) return true;

    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "[SDL] SDL_Init failed: " << SDL_GetError() << "\n";
        return false;
    }

    // Initialize codecs (MP3, OGG, FLAC, etc.)
    int flags = MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC;
    int initted = Mix_Init(flags);
    if ((initted & flags) != flags) {
        std::cerr << "[SDL_mixer] Mix_Init failed: " << Mix_GetError()
                  << " (some formats may not be supported)\n";
        // Not necessarily fatal, we can still try to play what is supported
    }

    // 44.1kHz, default format, stereo, 1024 buffer size
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        std::cerr << "[SDL_mixer] Mix_OpenAudio failed: " << Mix_GetError() << "\n";
        Mix_Quit();
        SDL_Quit();
        return false;
    }

    Mix_AllocateChannels(16);
    initialized_ = true;
    std::cout << "[DEBUG] Audio initialized.\n";
    return true;
}

void Player::shutdown() {
    if (!initialized_) return;

    std::cout << "[DEBUG] Shutting down audio...\n";
    Mix_HaltChannel(-1);
    Mix_HaltMusic();
    Mix_CloseAudio();
    Mix_Quit();
    SDL_Quit();
    initialized_ = false;
}

void Player::setPlaylist(std::shared_ptr<Playlist> playlist) {
    playlist_ = std::move(playlist);
}

bool Player::playCurrent() {
    if (!initialized_ || !playlist_ || playlist_->empty()) {
        std::cerr << "[ERROR] Cannot play: player not initialized or playlist empty.\n";
        return false;
    }

    const std::string path = playlist_->current();
    std::cout << "[DEBUG] Attempting to play: " << path << "\n";

    Mix_HaltChannel(-1);
    Mix_HaltMusic();

    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (!music) {
        std::cerr << "[SDL_mixer] Failed to load: " << path
                  << " | " << Mix_GetError() << "\n";
        return false;
    }

    if (Mix_PlayMusic(music, 1) < 0) {
        std::cerr << "[SDL_mixer] Failed to play: " << path
                  << " | " << Mix_GetError() << "\n";
        Mix_FreeMusic(music);
        return false;
    }

    paused_ = false;

    std::string nextTrack =
        (playlist_->size() > 1) ? playlist_->peekNext() : "(end of playlist)";

    // UI layer handles formatting + colors
    printNowPlayingBox(path, nextTrack);

    return true;
}

bool Player::playNext() {
    if (!playlist_) return false;
    playlist_->next();
    return playCurrent();
}

bool Player::playPrevious() {
    if (!playlist_) return false;
    playlist_->previous();
    return playCurrent();
}

void Player::pause() {
    if (!initialized_) return;
    Mix_PauseMusic();
    paused_ = true;
}

void Player::resume() {
    if (!initialized_) return;
    Mix_ResumeMusic();
    paused_ = false;
}

void Player::stop() {
    if (!initialized_) return;
    Mix_HaltMusic();
    paused_ = false;
}

bool Player::isPlaying() const {
    if (!initialized_) return false;
    return Mix_PlayingMusic() != 0;
}

bool Player::isPaused() const {
    return paused_;
}

std::string Player::nowPlaying() const {
    if (!playlist_ || playlist_->empty()) return {};
    return playlist_->current();
}
