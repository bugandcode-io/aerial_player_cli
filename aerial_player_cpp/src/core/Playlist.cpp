#include "Playlist.hpp"
#include <stdexcept>
#include <algorithm>

void Playlist::addTrack(const std::string& path) {
    tracks_.push_back(path);
}

const std::string& Playlist::current() const {
    if (tracks_.empty()) {
        throw std::runtime_error("Playlist is empty");
    }
    return tracks_[currentIndex_];
}

const std::string& Playlist::next() {
    if (tracks_.empty()) {
        throw std::runtime_error("Playlist is empty");
    }
    currentIndex_ = (currentIndex_ + 1) % tracks_.size();
    return tracks_[currentIndex_];
}

const std::string& Playlist::previous() {
    if (tracks_.empty()) {
        throw std::runtime_error("Playlist is empty");
    }
    if (currentIndex_ == 0) {
        currentIndex_ = tracks_.size() - 1;
    } else {
        --currentIndex_;
    }
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

// ───────── NEW STUFF ─────────

const std::string& Playlist::trackAt(size_t i) const {
    if (i >= tracks_.size()) {
        throw std::out_of_range("trackAt index out of range");
    }
    return tracks_[i];
}

std::vector<size_t> Playlist::search(const std::string& query) const {
    std::vector<size_t> result;
    if (query.empty()) return result;

    std::string qLower = query;
    std::transform(qLower.begin(), qLower.end(), qLower.begin(), ::tolower);

    for (size_t i = 0; i < tracks_.size(); ++i) {
        std::string name = tracks_[i];
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.find(qLower) != std::string::npos) {
            result.push_back(i);
        }
    }
    return result;
}

void Playlist::jumpTo(size_t i) {
    if (tracks_.empty()) return;
    if (i >= tracks_.size()) {
        throw std::out_of_range("jumpTo index out of range");
    }
    currentIndex_ = i;
}
