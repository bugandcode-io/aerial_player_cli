#include "Config.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>     // for std::getenv
#include "json.hpp"

using json = nlohmann::json;
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
    if (!home) {
        std::cerr << "[WARN] HOME/USERPROFILE not set; no config path.\n";
        return {};
    }

    fs::path base = fs::path(home) / ".config" / "aerial";
    return base / "config.json";
}

AerialConfig load_config() {
    AerialConfig cfg;  // defaults from Config.hpp

    fs::path path = get_default_config_path();
    if (path.empty()) {
        std::cerr << "[WARN] No config path resolved. Using built-in defaults.\n";
        return cfg;
    }

    if (!fs::exists(path)) {
        std::cerr << "[WARN] Config file not found: "
                  << path.string() << "\n";
        std::cerr << "[WARN] Using built-in defaults.\n";
        return cfg;
    }

    try {
        std::ifstream in(path);
        if (!in.is_open()) {
            std::cerr << "[WARN] Could not open config file: "
                      << path.string() << "\n";
            std::cerr << "[WARN] Using built-in defaults.\n";
            return cfg;
        }

        json j;
        in >> j;   // or: j = json::parse(in);

        if (j.contains("db_path") && j["db_path"].is_string()) {
            cfg.db_path = j["db_path"].get<std::string>();
        }
        if (j.contains("port") && j["port"].is_number_integer()) {
            cfg.port = j["port"].get<int>();
        }
        if (j.contains("scan_recursive") && j["scan_recursive"].is_boolean()) {
            cfg.scan_recursive = j["scan_recursive"].get<bool>();
        }

        if (j.contains("user") && j["user"].is_object()) {
             auto& u = j["user"];
            if (u.contains("default_volume") && u["default_volume"].is_number_integer()) {
                cfg.default_volume = u["default_volume"].get<int>();
            }
        }

        std::cout << "[CONFIG] Loaded config from: "
                  << path.string() << "\n";

    } catch (const std::exception& e) {
        std::cerr << "[WARN] Failed to parse config.json: " << e.what() << "\n";
        std::cerr << "[WARN] Using built-in defaults.\n";
    }

    return cfg;
}
