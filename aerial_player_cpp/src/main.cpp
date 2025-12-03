#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>
#include <stdexcept>

#include "Player.hpp"
#include "Playlist.hpp"

namespace fs = std::filesystem;

std::shared_ptr<Playlist> buildPlaylistFromFolder(const std::string& folderPath) {
    auto playlist = std::make_shared<Playlist>();

    std::cout << "[DEBUG] Scanning folder: " << folderPath << "\n";

    if (!fs::exists(folderPath)) {
        throw std::runtime_error("Folder does not exist: " + folderPath);
    }
    if (!fs::is_directory(folderPath)) {
        throw std::runtime_error("Path is not a directory: " + folderPath);
    }

    try {
        for (const auto& entry : fs::directory_iterator(folderPath)) {
            if (!entry.is_regular_file()) continue;

            const auto path = entry.path();
            const auto ext = path.extension().string();

            // Basic filter: only load common audio formats
            if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || ext == ".flac") {
                std::cout << "[DEBUG] Found audio file: " << path.string() << "\n";
                playlist->addTrack(path.string());
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Error reading folder: ") + e.what());
    }

    std::cout << "[DEBUG] Playlist size: " << playlist->size() << "\n";
    return playlist;
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            std::cout << "Usage: aerial <music_folder>\n";
            return 1;
        }

        std::string folder = argv[1];
        std::cout << "[DEBUG] Aerial starting with folder: " << folder << "\n";

        auto playlist = buildPlaylistFromFolder(folder);
        if (playlist->empty()) {
            std::cout << "No supported audio files found in folder: " << folder << "\n";
            return 1;
        }

        Player player;
        std::cout << "[DEBUG] Initializing audio...\n";
        if (!player.init()) {
            std::cerr << "Failed to initialize audio.\n";
            return 1;
        }

        player.setPlaylist(playlist);

        std::cout << "[DEBUG] Calling playCurrent()...\n";
        if (!player.playCurrent()) {
            std::cerr << "Failed to start playback.\n";
            return 1;
        }

        std::cout << "Aerial Player (C++ CLI)\n";
        std::cout << "Controls:\n";
        std::cout << "  next - next\n";
        std::cout << "  pre- previous\n";
        std::cout << "  pause/resume - pause/resume (type space then Enter)\n";
        std::cout << "  stop - stop\n";
        std::cout << "  quit - quit\n\n";

        std::string cmd;
        bool running = true;

        while (running) {
    std::cout << "> ";
    if (!(std::cin >> cmd)) break;

    if (cmd == "play") {
        player.playCurrent();
    }
    else if (cmd == "resume") {
        player.resume();
    }
    else if (cmd == "pause") {
        player.pause();
    }
    else if (cmd == "next") {
        player.playNext();
    }
    else if (cmd == "prev" || cmd == "previous") {
        player.playPrevious();
    }
    else if (cmd == "stop") {
        player.stop();
    }
    else if (cmd == "quit" || cmd == "exit") {
        std::cout << "[DEBUG] Quit command received.\n";
        running = false;
    }
    else {
        std::cout << "Unknown command: " << cmd << "\n";
        std::cout << "Commands: play, resume, pause, next, prev, stop, quit\n";
    }
}

        player.shutdown();
        std::cout << "[DEBUG] Shutdown complete.\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] Unhandled exception: " << e.what() << "\n";
        return 1;
    }
}
