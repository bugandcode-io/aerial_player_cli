#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>
#include <stdexcept>
#include <system_error> // for std::error_code
#include <algorithm>
#include <atomic>
#include <thread>
#include <chrono>

#include "Player.hpp"
#include "Playlist.hpp"
#include "Server.hpp"


namespace fs = std::filesystem;

// ───────────────────────────────
// Helpers
// ───────────────────────────────

static bool isAudioFile(const fs::path& path) {
    // Use wide string to avoid ANSI codepage issues
    auto ext = path.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    return ext == L".mp3" ||
           ext == L".wav" ||
           ext == L".ogg" ||
           ext == L".flac" ||
           ext == L".m4a";
}

std::shared_ptr<Playlist> buildPlaylistFromFolder(const std::string& folderPath) {
    auto playlist = std::make_shared<Playlist>();

    std::cout << "[DEBUG] Scanning folder: " << folderPath << "\n";

    // Treat input as UTF-8 and build a filesystem path from it
    fs::path root = fs::u8path(folderPath);

    std::error_code ec;

    if (!fs::exists(root, ec) || ec) {
        throw std::runtime_error(
            "Folder does not exist or cannot be accessed: " + folderPath +
            " (" + ec.message() + ")"
        );
    }

    if (!fs::is_directory(root, ec) || ec) {
        throw std::runtime_error(
            "Path is not a directory: " + folderPath +
            " (" + ec.message() + ")"
        );
    }

    // Use recursive iterator and skip entries that error instead of throwing
    fs::directory_options opts = fs::directory_options::skip_permission_denied;

    fs::recursive_directory_iterator it(root, opts, ec), end;
    if (ec) {
        throw std::runtime_error(
            std::string("Error creating directory iterator: ") + ec.message()
        );
    }

    for (; it != end; it.increment(ec)) {
        if (ec) {
            std::cerr << "[WARN] Skipping entry: " << ec.message() << "\n";
            ec.clear();
            continue;
        }

        const fs::directory_entry& entry = *it;
        if (!entry.is_regular_file()) continue;

        const fs::path& path = entry.path();
        if (!isAudioFile(path)) continue;

        // Log and store UTF-8 paths; avoids codepage issues on Windows
        std::string utf8Path = path.u8string();
        std::cout << "[DEBUG] Found audio file: " << utf8Path << "\n";
        playlist->addTrack(utf8Path);
    }

    std::cout << "[DEBUG] Playlist size: " << playlist->size() << "\n";
    return playlist;
}

// ───────────────────────────────
// main
// ───────────────────────────────

int main(int argc, char* argv[]) {

    AerialConfig cfg = load_config();

    std::cout << "[CONFIG] DB Path: " << cfg.db_path << "\n";
    std::cout << "[CONFIG] Server Port: " << cfg.port << "\n";

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
        
        // 🔥 Start TCP control server in background
        start_control_server(player, playlist);
        start_http_server(player, playlist, 8080);   

        constexpr const char* AERIAL_VERSION = "0.1.3-dev (CLI)";
        std::cout << "Aerial Player " << AERIAL_VERSION << "\n\n";

        std::cout << "Aerial Player (C++ CLI)\n";
        std::cout << "Controls:\n";
        std::cout << "  play        - play current track\n";
        std::cout << "  next        - next track\n";
        std::cout << "  prev        - previous track\n";
        std::cout << "  ff          - fast forward 10s\n";
        std::cout << "  rew         - rewind 10s\n";
        std::cout << "  search      - search and play a track\n";   // <── NEW
        std::cout << "  pause       - pause\n";
        std::cout << "  resume      - resume\n";
        std::cout << "  stop        - stop\n";
        std::cout << "  quit/exit   - quit\n\n";

        // ===== Progress bar thread =====
        std::atomic<bool> progressRunning{true};
        std::atomic<bool> inputActive{false};

                std::thread progressThread([&]() {
            const int barWidth = 40;

            while (progressRunning.load()) {
                // Don't draw while the user is typing a command
                if (!inputActive.load()) {
                    double pos = player.getPositionSeconds();
                    int posInt = static_cast<int>(pos);

                    // Keep the bar moving based on time
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

                    // Print bar at start of line
                    std::cout << "\r" << bar << " " << posInt << "s";
                    std::cout.flush();
                }

                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            // Clear line when exiting
            std::cout << "\r" << std::string(80, ' ') << "\r";
            std::cout.flush();
        });


        // ===== Command loop =====
        std::string cmd;
        bool running = true;

        while (running) {
            inputActive.store(true);
            std::cout << "\n> ";
            if (!(std::cin >> cmd)) {
                inputActive.store(false);
                break;
            }
            inputActive.store(false);

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
            else if (cmd == "ff") {
                player.seekBy(10.0);
            }
            else if (cmd == "rew") {
                player.seekBy(-10.0);
            }
              else if (cmd == "search") {
                // ---- SEARCH COMMAND ----
                std::string term;
                std::cout << "Search term: ";
                std::getline(std::cin >> std::ws, term);

                if (term.empty()) {
                    std::cout << "Search cancelled.\n";
                    continue;
                }

                auto matches = playlist->search(term);
                if (matches.empty()) {
                    std::cout << "No matches for \"" << term << "\"\n";
                    continue;
                }

                std::cout << "Found " << matches.size() << " match(es):\n";
                for (size_t i = 0; i < matches.size(); ++i) {
                    size_t idx = matches[i];
                    const std::string& fullPath = playlist->trackAt(idx);

                    // Show just filename
                    fs::path p = fs::u8path(fullPath);
                    std::string name = p.filename().u8string();

                    std::cout << "  [" << i << "] " << name << "\n";
                }

                std::cout << "Enter number to play (blank = cancel): ";
                std::string choice;
                std::getline(std::cin, choice);

                if (choice.empty()) {
                    std::cout << "Selection cancelled.\n";
                    continue;
                }

                try {
                    int sel = std::stoi(choice);
                    if (sel < 0 || static_cast<size_t>(sel) >= matches.size()) {
                        std::cout << "Invalid selection.\n";
                        continue;
                    }

                    size_t realIndex = matches[sel];
                    playlist->jumpTo(realIndex);
                    player.playCurrent();
                } catch (...) {
                    std::cout << "Invalid input.\n";
                }
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
                std::cout << "Commands: play, resume, pause, next, prev, ff, rew, stop, quit\n";
            }
        }

        // Stop progress thread
        progressRunning.store(false);
        if (progressThread.joinable()) {
            progressThread.join();
        }

        player.shutdown();
        std::cout << "[DEBUG] Shutdown complete.\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] Unhandled exception: " << e.what() << "\n";
        return 1;
    }
}
