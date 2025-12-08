#include "UI.hpp"
#include "Playlist.hpp"
#include <iostream>
#include <filesystem>
#include <cctype>  
#include <sstream>


#ifdef _WIN32
#include <windows.h>
#endif

// ==========================================
//  Extract clean title (no path, no extension)
// ==========================================
std::string extractTitle(const std::string& fullPath) {
    if (fullPath.empty()) return "(none)";

    std::filesystem::path p = std::filesystem::u8path(fullPath);
    std::string name = p.filename().u8string();
    

    // Remove extension
    size_t dot = name.rfind('.');
    if (dot != std::string::npos)
        name = name.substr(0, dot);

    // Strip numeric prefixes like "01 - ", "07. ", "03 "
    if (name.size() > 2 &&
        std::isdigit((unsigned char)name[0]) &&
        std::isdigit((unsigned char)name[1]))
    {
        size_t pos = 2;

        // Optional punctuation
        if (pos < name.size() && (name[pos] == '-' || name[pos] == '.' || name[pos] == '_'))
            pos++;

        // Optional space
        if (pos < name.size() && name[pos] == ' ')
            pos++;

        name = name.substr(pos);
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



void updateNowPlayingUI(Playlist& playlist) {
    std::string nowPath;
    std::string nextPath;

    if (!playlist.empty()) {
        nowPath = playlist.current();     // get current track safely
        nextPath = playlist.peekNext();   // automatically wraps around
    }

    printNowPlayingBox(nowPath, nextPath);
}


std::string renderNowPlayingBox(const std::string& nowPath,
                                const std::string& nextPath)
{
    const std::string line(69, '*');

    std::string now  = extractTitle(nowPath);
    std::string next = nextPath.empty()
        ? "(end of playlist)"
        : extractTitle(nextPath);

    std::ostringstream out;
    out << line << "\n";

#ifdef _WIN32
    // For sockets, don't bother with Windows color codes;
    // a telnet client might not understand them anyway.
    out << "****    Now Playing: " << now  << "\n";
    out << "****\n";
    out << "****    Up Next:     " << next << "\n";
#else
    // If you want ANSI colors over telnet you can keep these:
    out << "****    Now Playing: \033[32m" << now  << "\033[0m\n";
    out << "****\n";
    out << "****    Up Next:     \033[33m" << next << "\033[0m\n";
#endif

    out << line << "\n";
    return out.str();
}



std::string renderNowPlayingBoxPlain(const std::string& nowPath,
                                     const std::string& nextPath)
{
    const std::string line(69, '*');

    std::string now  = nowPath.empty()  ? "(none)"            : extractTitle(nowPath);
    std::string next = nextPath.empty() ? "(end of playlist)" : extractTitle(nextPath);

    std::ostringstream out;

    // out << "\r\n=== TELNET UI TEST ===\r\n";   // 👈 BIG TEST MARKER

    out << line << "\r\n";
    out << "****    Now Playing: " << now  << "\r\n";
    out << "****\r\n";
    out << "****    Up Next:     " << next << "\r\n";
    out << line << "\r\n";

    return out.str();
}

std::string renderProgressBar(double positionSeconds)
{
    const int barWidth = 40;

    int posInt = static_cast<int>(positionSeconds);
    if (posInt < 0) posInt = 0;

    int filled = posInt % (barWidth + 1);
    if (filled > barWidth) filled = barWidth;

    std::ostringstream out;

    out << "[";
    out << std::string(filled, '=');
    if (filled < barWidth) {
        out << ">";
        out << std::string(barWidth - filled - 1, ' ');
    } else {
        out << std::string(barWidth - filled, ' ');
    }
    out << "]";
    out << " " << posInt << "s\r\n";

    return out.str();
}


std::string renderProgressBarLine(double seconds)
{
    const int barWidth = 40;

    int posInt = static_cast<int>(seconds);
    int filled = posInt % (barWidth + 1);
    if (filled > barWidth) filled = barWidth;

    std::string bar = "[";
    bar += std::string(filled, '=');
    if (filled < barWidth) {
        bar += ">";
        bar += std::string(barWidth - filled - 1, ' ');
    } else {
        bar += std::string(barWidth - filled, ' ');
    }
    bar += "]";

    std::ostringstream oss;
    oss << bar << " " << posInt << "s\r\n";
    return oss.str();
}