#include "Config.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>     // for std::getenv
#include "json.hpp"

using json = mini_json::json;
namespace fs = std::filesystem;

/*
 * Returns ~/.config/aerial/config.json on Linux/macOS
 * or %USERPROFILE%/.config/aerial/config.json on Windows
 */
static fs::path get_default_config_path() {
#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
#else
    const char* home = std::getenv("HOME");
#endif
    if (!home) return {};

    fs::path base = fs::path(home) / ".config" / "aerial";
    return base / "config.json";
}

AerialConfig load_config() {
    AerialConfig cfg;  // defaults from Config.hpp

    fs::path path = get_default_config_path();

    if (!fs::exists(path)) {
        std::cerr << "[WARN] Config file not found: "
                  << path.string() << "\n";
        std::cerr << "[WARN] Using built-in defaults.\n";
        return cfg;
    }

    try {
        json j = json::parseFile(path.string());

        if (j.contains("db_path")) {
            cfg.db_path = j["db_path"].get<std::string>();
        }
        if (j.contains("port")) {
            cfg.port = j["port"].get<int>();
        }
        if (j.contains("scan_recursive")) {
            cfg.scan_recursive = j["scan_recursive"].get<bool>();
        }

    } catch (const std::exception& e) {
        std::cerr << "[WARN] Failed to parse config.json: " << e.what() << "\n";
        std::cerr << "[WARN] Using built-in defaults.\n";
    }

    return cfg;
}
