#include "DB.hpp"
#include <iostream>
#include <filesystem>


#include "json.hpp"
#include <fstream>
using json = nlohmann::json;

namespace fs = std::filesystem;

// Helper: get a nice title from the full path (filename only)
static std::string extractTitleFromPath(const std::string& path) {
    try {
        fs::path p = fs::u8path(path);
        return p.filename().u8string();   // just the file name
    } catch (...) {
        return path; // fallback
    }
}

#ifdef AERIAL_USE_SQLITE
// ──────────────────────────────────────────────
//  SQLITE BACKEND
// ──────────────────────────────────────────────
#include <sqlite3.h>

PlayDatabase::PlayDatabase(const std::string& path)
    : dbPath_(path),
      db_(nullptr),
      ok_(false)
{
    int rc = sqlite3_open(path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Failed to open DB at " << path
                  << " : " << sqlite3_errmsg(db_) << "\n";
        sqlite3_close(db_);
        db_ = nullptr;
        return;
    }

    if (!initSchema()) {
        std::cerr << "[DB] Failed to initialize schema.\n";
        sqlite3_close(db_);
        db_ = nullptr;
        return;
    }

    ok_ = true;
    std::cout << "[DB] Opened SQLite DB at " << path << "\n";
}

PlayDatabase::~PlayDatabase() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool PlayDatabase::initSchema() {
    if (!db_) return false;

    const char* sql =
        "CREATE TABLE IF NOT EXISTS plays ("
        "  id           INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  track_path   TEXT NOT NULL,"
        "  track_title  TEXT NOT NULL,"
        "  event_type   TEXT NOT NULL,"  // 'play' | 'skip' | 'finished'
        "  created_at   DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] Schema error: " << (errMsg ? errMsg : "") << "\n";
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

void PlayDatabase::logEvent(const std::string& trackPath,
                            const std::string& eventType)
{
    if (!db_) return;

    std::string title = extractTitleFromPath(trackPath);

    const char* sql =
        "INSERT INTO plays (track_path, track_title, event_type) "
        "VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB] prepare failed: " << sqlite3_errmsg(db_) << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, trackPath.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, title.c_str(),      -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, eventType.c_str(),  -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "[DB] insert failed: " << sqlite3_errmsg(db_) << "\n";
    } else {
        auto rowid = sqlite3_last_insert_rowid(db_);
        std::cout << "[DB] logged event=" << eventType
                  << " rowid=" << rowid << "\n";
    }

    sqlite3_finalize(stmt);
}

void PlayDatabase::logPlay(const std::string& trackPath) {
    logEvent(trackPath, "play");
}

void PlayDatabase::logSkip(const std::string& trackPath) {
    logEvent(trackPath, "skip");
}

void PlayDatabase::logFinished(const std::string& trackPath) {
    logEvent(trackPath, "finished");
}

#else  // ─────────── JSON BACKEND (no SQLite) ───────────



// JSON schema:
//
// {
//   "tracks": {
//      "<path>": {
//          "title": "...",
//          "plays": 12,
//          "skips": 3,
//          "finished": 5
//      },
//      ...
//   }
// }

// Load JSON stats file (or create an empty object if missing/broken)
static nlohmann::json loadStatsJson(const std::string& path) {
    nlohmann::json j;

    std::ifstream in(path);
    if (!in.is_open()) {
        // No file yet, start with empty structure
        j["tracks"] = nlohmann::json::object();
        return j;
    }

    try {
        in >> j;
    } catch (...) {
        std::cerr << "[DB] Failed to parse JSON stats, starting fresh.\n";
        j["tracks"] = nlohmann::json::object();
    }

    if (!j.contains("tracks") || !j["tracks"].is_object()) {
        j["tracks"] = nlohmann::json::object();
    }
    return j;
}

// Save JSON stats back to disk
static void saveStatsJson(const std::string& path, const nlohmann::json& j) {
    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "[DB] Failed to write JSON stats to: " << path << "\n";
        return;
    }
    out << j.dump(2);
}

// JSON-mode constructor
PlayDatabase::PlayDatabase(const std::string& path)
    : dbPath_(path),
      db_(nullptr),  // unused in JSON mode
      ok_(true)
{
    std::cout << "[DB] JSON stats mode at " << path << "\n";
}

PlayDatabase::~PlayDatabase() {
    // Nothing special in JSON mode
}

bool PlayDatabase::initSchema() {
    // Nothing to do for JSON
    return true;
}

void PlayDatabase::logEvent(const std::string& trackPath,
                            const std::string& eventType)
{
    if (!ok_) return;

    nlohmann::json j = loadStatsJson(dbPath_);

    auto& tracks = j["tracks"];
    std::string title = extractTitleFromPath(trackPath);

    nlohmann::json& entry = tracks[trackPath];
    if (!entry.contains("title"))    entry["title"]    = title;
    if (!entry.contains("plays"))    entry["plays"]    = 0;
    if (!entry.contains("skips"))    entry["skips"]    = 0;
    if (!entry.contains("finished")) entry["finished"] = 0;

    if (eventType == "play") {
        entry["plays"] = entry["plays"].get<int>() + 1;
    } else if (eventType == "skip") {
        entry["skips"] = entry["skips"].get<int>() + 1;
    } else if (eventType == "finished") {
        entry["finished"] = entry["finished"].get<int>() + 1;
    }

    saveStatsJson(dbPath_, j);

    std::cout << "[DB] JSON logged event=" << eventType
              << " for " << trackPath << "\n";
}

void PlayDatabase::logPlay(const std::string& trackPath) {
    logEvent(trackPath, "play");
}

void PlayDatabase::logSkip(const std::string& trackPath) {
    logEvent(trackPath, "skip");
}

void PlayDatabase::logFinished(const std::string& trackPath) {
    logEvent(trackPath, "finished");
}

#endif // AERIAL_USE_SQLITE
