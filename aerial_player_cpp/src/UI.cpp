#include "UI.hpp"
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

// ==========================================
//  Extract clean title (no path, no extension)
// ==========================================
std::string extractTitle(const std::string& fullPath) {
    if (fullPath.empty()) return "(none)";

    std::filesystem::path p(fullPath);
    std::string name = p.filename().string();

    // Remove extension
    size_t dot = name.rfind('.');
    if (dot != std::string::npos)
        name = name.substr(0, dot);

    // Strip "01 - " or "01 " prefixes
    if (name.size() > 4 &&
        std::isdigit(name[0]) &&
        std::isdigit(name[1]) &&
        (name[2] == '-' || name[2] == '.') &&
        name[3] == ' ') {
        name = name.substr(4);
    }
    else if (name.size() > 3 &&
             std::isdigit(name[0]) &&
             std::isdigit(name[1]) &&
             name[2] == ' ') {
        name = name.substr(3);
    }

    return name;
}

// ==========================================
//  Pretty UI Box (green + yellow)
// ==========================================
void printNowPlayingBox(const std::string& nowPath,
                        const std::string& nextPath)
{
    const std::string line(69, '*');

    std::string now  = extractTitle(nowPath);
    std::string next = extractTitle(nextPath);

    std::cout << line << "\n";

#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    WORD original;
    GetConsoleScreenBufferInfo(hConsole, &info);
    original = info.wAttributes;

    // NOW PLAYING (GREEN)
    std::cout << "****    Now Playing: ";
    SetConsoleTextAttribute(
        hConsole,
        (original & 0xF0) | FOREGROUND_GREEN | FOREGROUND_INTENSITY
    );
    std::cout << now << "\n";
    SetConsoleTextAttribute(hConsole, original);

#else
    std::cout << "****    Now Playing: \033[32m" << now << "\033[0m\n";
#endif

    std::cout << "****\n";

#ifdef _WIN32
    // UP NEXT (YELLOW — RED + GREEN = YELLOW)
    std::cout << "****    Up Next:     ";
    SetConsoleTextAttribute(
        hConsole,
        (original & 0xF0) |
        FOREGROUND_RED |
        FOREGROUND_GREEN |
        FOREGROUND_INTENSITY
    );
    std::cout << next << "\n";
    SetConsoleTextAttribute(hConsole, original);
#else
    std::cout << "****    Up Next:     \033[33m" << next << "\033[0m\n";
#endif

    std::cout << line << "\n";
}
