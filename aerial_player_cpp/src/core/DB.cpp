#include "DB.hpp"
#include <sqlite3.h>
#include <iostream>
#include <filesystem>

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

PlayDatabase::PlayDatabase(const std::string& path)
    : db_(nullptr)
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
    } else {
        std::cout << "[DB] Opened DB at " << path << "\n";
    }
}

PlayDatabase::~PlayDatabase() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

// NOTE: do NOT re-define ok() here – it's inline in DB.hpp
// bool PlayDatabase::ok() const { ... }  // ← leave this OUT

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
