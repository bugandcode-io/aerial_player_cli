#pragma once

#include <string>
#include <vector>

class Playlist {
public:
    Playlist() = default;

    void addTrack(const std::string& path);
    const std::string& current() const;
    const std::string& next();
    const std::string& previous();
    bool empty() const;
    size_t size() const;
    size_t index() const;
    std::string peekNext() const;


private:
    std::vector<std::string> tracks_;
    size_t currentIndex_ = 0;
};
