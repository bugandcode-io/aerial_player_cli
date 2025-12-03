#pragma once
#include <string>

void printNowPlayingBox(const std::string& nowPath,
                        const std::string& nextPath);
std::string extractTitle(const std::string& fullPath);
