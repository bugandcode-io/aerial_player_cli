#pragma once
#include <string>

class Playlist;

void printNowPlayingBox(const std::string& nowPath,
                        const std::string& nextPath);

std::string extractTitle(const std::string& fullPath);

// 🔹 add this:
std::string renderNowPlayingBoxPlain(const std::string& nowPath,
                                     const std::string& nextPath);

std::string renderProgressBar(double positionSeconds);


void updateNowPlayingUI(Playlist& playlist);

std::string renderProgressBarLine(double seconds);
