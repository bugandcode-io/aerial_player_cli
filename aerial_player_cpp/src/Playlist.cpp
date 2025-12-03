#include "Playlist.hpp"
#include <stdexcept>

void Playlist::addTrack(const std::string& path) {
    tracks_.push_back(path);
}

const std::string& Playlist::current() const {
    if (tracks_.empty()) throw std::runtime_error("Playlist is empty");
    return tracks_[currentIndex_];
}

const std::string& Playlist::next() {
    if (tracks_.empty()) throw std::runtime_error("Playlist is empty");
    currentIndex_ = (currentIndex_ + 1) % tracks_.size();
    return tracks_[currentIndex_];
}

const std::string& Playlist::previous() {
    if (tracks_.empty()) throw std::runtime_error("Playlist is empty");
    if (currentIndex_ == 0) currentIndex_ = tracks_.size() - 1;
    else --currentIndex_;
    return tracks_[currentIndex_];
}

bool Playlist::empty() const {
    return tracks_.empty();
}

size_t Playlist::size() const {
    return tracks_.size();
}

size_t Playlist::index() const {
    return currentIndex_;
}

std::string Playlist::peekNext() const {
    if (tracks_.empty()) return "";
    size_t nextIndex = (currentIndex_ + 1) % tracks_.size();
    return tracks_[nextIndex];
}
