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
#include <cctype> // for std::isspace
#include <sstream> // for parsing "vol 50"


#include "Config.hpp"
#include "Player.hpp"
#include "Playlist.hpp"
#include "Server.hpp"
#include "UI.hpp"
#include "DB.hpp"

namespace fs = std::filesystem;

// ───────────────────────────────
// Helpers
// ───────────────────────────────

static bool isAudioFile(const fs::path &path)
{
    // Use wide string to avoid ANSI codepage issues
    auto ext = path.extension().wstring();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    return ext == L".mp3" ||
           ext == L".wav" ||
           ext == L".ogg" ||
           ext == L".flac" ||
           ext == L".m4a";
}

std::shared_ptr<Playlist> buildPlaylistFromFolder(const std::string &folderPath)
{
    auto playlist = std::make_shared<Playlist>();

    std::cout << "[DEBUG] Scanning folder: " << folderPath << "\n";

    // Treat input as UTF-8 and build a filesystem path from it
    fs::path root = fs::u8path(folderPath);

    std::error_code ec;

    if (!fs::exists(root, ec) || ec)
    {
        throw std::runtime_error(
            "Folder does not exist or cannot be accessed: " + folderPath +
            " (" + ec.message() + ")");
    }

    if (!fs::is_directory(root, ec) || ec)
    {
        throw std::runtime_error(
            "Path is not a directory: " + folderPath +
            " (" + ec.message() + ")");
    }

    // Use recursive iterator and skip entries that error instead of throwing
    fs::directory_options opts = fs::directory_options::skip_permission_denied;

    fs::recursive_directory_iterator it(root, opts, ec), end;
    if (ec)
    {
        throw std::runtime_error(
            std::string("Error creating directory iterator: ") + ec.message());
    }

    for (; it != end; it.increment(ec))
    {
        if (ec)
        {
            std::cerr << "[WARN] Skipping entry: " << ec.message() << "\n";
            ec.clear();
            continue;
        }

        const fs::directory_entry &entry = *it;
        if (!entry.is_regular_file())
            continue;

        const fs::path &path = entry.path();
        if (!isAudioFile(path))
            continue;

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

int main(int argc, char *argv[])
{

    AerialConfig cfg = load_config();

    std::cout << "[CONFIG] DB Path: " << cfg.db_path << "\n";
    std::cout << "[CONFIG] Server Port: " << cfg.port << "\n";

    // 🔹 Init DB (may be disabled if path invalid)
    PlayDatabase db(cfg.db_path);
    if (!db.ok())
    {
        std::cerr << "[DB] WARNING: DB not available; continuing without logging.\n";
    }

    try
    {
        if (argc < 2)
        {
            std::cout << "Usage: aerial <music_folder>\n";
            return 1;
        }

        std::string folder = argv[1];
        std::cout << "[DEBUG] Aerial starting with folder: " << folder << "\n";

        auto playlist = buildPlaylistFromFolder(folder);
        if (playlist->empty())
        {
            std::cout << "No supported audio files found in folder: " << folder << "\n";
            return 1;
        }

        Player player;
        std::cout << "[DEBUG] Initializing audio...\n";
        if (!player.init())
        {
            std::cerr << "Failed to initialize audio.\n";
            return 1;
        }

        player.setPlaylist(playlist);

        std::cout << "[DEBUG] Calling playCurrent()...\n";
        if (!player.playCurrent())
        {
            std::cerr << "Failed to start playback.\n";
            return 1;
        }

        // Initial DB log + UI
        if (db.ok())
        {
            db.logPlay(playlist->current());
        }
        updateNowPlayingUI(*playlist);

        // 🔥 Start TCP control server in background
        start_control_server(player, playlist, db.ok() ? &db : nullptr);
        start_http_server(player, playlist, 8080);

        constexpr const char *AERIAL_VERSION = "0.1.3-dev (CLI)";
        std::cout << "Aerial Player " << AERIAL_VERSION << "\n\n";

        std::cout << "Aerial Player (C++ CLI)\n";
        std::cout << "Controls:\n";
        std::cout << "  play        - play current track\n";
        std::cout << "  next        - next track\n";
        std::cout << "  prev        - previous track\n";
        std::cout << "  ff          - fast forward 10s\n";
        std::cout << "  rew         - rewind 10s\n";
        std::cout << "  search      - search and play a track\n";
        std::cout << "  volup       - volume +5%\n";
        std::cout << "  voldown     - volume -5%\n";
        std::cout << "  mute        - volume 0%\n";
        std::cout << "  vol         - show current volume\n";
        std::cout << "  pause       - pause\n";
        std::cout << "  resume      - resume\n";
        std::cout << "  stop        - stop\n";
        std::cout << "  quit/exit   - quit\n\n";

        // ===== Command loop =====
        std::string cmd;
        bool running = true;

        while (running)
        {
            std::cout << "\n> ";
            if (!std::getline(std::cin, cmd))
            {
                break; // EOF / ctrl-D / ctrl-Z
            }

            // Trim both ends
            while (!cmd.empty() && std::isspace(static_cast<unsigned char>(cmd.front())))
            {
                cmd.erase(cmd.begin());
            }
            while (!cmd.empty() && std::isspace(static_cast<unsigned char>(cmd.back())))
            {
                cmd.pop_back();
            }

            if (cmd.empty())
            {
                continue; // just hit Enter, don't do anything
            }

            if (cmd == "play")
            {
                player.playCurrent();
                updateNowPlayingUI(*playlist);
                if (db.ok())
                {
                    db.logPlay(playlist->current());
                }
            }
            else if (cmd == "resume")
            {
                player.resume();
            }
            else if (cmd == "pause")
            {
                player.pause();
            }
            else if (cmd == "next")
            {
                // 🔹 Capture what was playing before skipping
                std::string prevTrack;
                if (!playlist->empty())
                {
                    prevTrack = playlist->current();
                }

                player.playNext();
                updateNowPlayingUI(*playlist);

                if (db.ok())
                {
                    if (!prevTrack.empty())
                    {
                        db.logSkip(prevTrack);
                    }
                    db.logPlay(playlist->current());
                }
            }
            else if (cmd == "prev" || cmd == "previous")
            {
                player.playPrevious();
                updateNowPlayingUI(*playlist);
                if (db.ok())
                {
                    db.logPlay(playlist->current());
                }
            }
            else if (cmd == "ff")
            {
                player.seekBy(10.0);
            }
            else if (cmd == "rew")
            {
                player.seekBy(-10.0);
            }
            else if (cmd == "search")
            {
                // ---- SEARCH COMMAND ----
                std::string term;
                std::cout << "Search term: ";
                std::getline(std::cin >> std::ws, term);

                if (term.empty())
                {
                    std::cout << "Search cancelled.\n";
                    continue;
                }

                auto matches = playlist->search(term);
                if (matches.empty())
                {
                    std::cout << "No matches for \"" << term << "\"\n";
                    continue;
                }

                std::cout << "Found " << matches.size() << " match(es):\n";
                for (size_t i = 0; i < matches.size(); ++i)
                {
                    size_t idx = matches[i];
                    const std::string &fullPath = playlist->trackAt(idx);

                    // Show just filename
                    fs::path p = fs::u8path(fullPath);
                    std::string name = p.filename().u8string();

                    std::cout << "  [" << i << "] " << name << "\n";
                }

                std::cout << "Enter number to play (blank = cancel): ";
                std::string choice;
                std::getline(std::cin, choice);

                if (choice.empty())
                {
                    std::cout << "Selection cancelled.\n";
                    continue;
                }

                try
                {
                    int sel = std::stoi(choice);
                    if (sel < 0 || static_cast<size_t>(sel) >= matches.size())
                    {
                        std::cout << "Invalid selection.\n";
                        continue;
                    }

                    size_t realIndex = matches[sel];
                    playlist->jumpTo(realIndex);
                    player.playCurrent();
                    updateNowPlayingUI(*playlist);
                    if (db.ok())
                    {
                        db.logPlay(playlist->current());
                    }
                }
                catch (...)
                {
                    std::cout << "Invalid input.\n";
                }
            }
            else if (cmd == "stop")
            {
                player.stop();
            }


             else if (cmd == "volup")
            {
                player.changeVolumePercent(+5);
                std::cout << "Volume: " << player.getVolumePercent() << "%\n";
            }
            else if (cmd == "voldown")
            {
                player.changeVolumePercent(-5);
                std::cout << "Volume: " << player.getVolumePercent() << "%\n";
            }
            else if (cmd == "mute")
            {
                player.setVolumePercent(0);
                std::cout << "Volume: " << player.getVolumePercent() << "% (muted)\n";
            }
            else if (cmd.rfind("vol", 0) == 0) // starts with "vol"
            {
                // Allow:
                //   vol        -> show volume
                //   vol 60     -> set to 60%
                if (cmd == "vol")
                {
                    std::cout << "Volume: " << player.getVolumePercent() << "%\n";
                }
                else
                {
                    // parse number after "vol"
                    std::istringstream iss(cmd.substr(3)); // skip "vol"
                    int percent = 0;
                    if (iss >> percent)
                    {
                        player.setVolumePercent(percent);
                        std::cout << "Volume set to: " << player.getVolumePercent() << "%\n";
                    }
                    else
                    {
                        std::cout << "Usage: vol [0-100]\n";
                    }
                }
            }

            
            else if (cmd == "quit" || cmd == "exit")
            {
                std::cout << "[DEBUG] Quit command received.\n";
                running = false;
            }
            else
            {
                std::cout << "Unknown command: " << cmd << "\n";
                std::cout << "Commands: play, resume, pause, next, prev, ff, rew, stop, quit\n";
            }
        }

        player.shutdown();
        std::cout << "[DEBUG] Shutdown complete.\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[FATAL] Unhandled exception: " << e.what() << "\n";
        return 1;
    }
}
