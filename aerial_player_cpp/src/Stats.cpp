#include <sqlite3.h>
#include <string>
#include <iostream>

bool init_db(sqlite3** db) {
    int rc = sqlite3_open("aerial.db", db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open DB: " << sqlite3_errmsg(*db) << "\n";
        return false;
    }

    const char* sql =
        "CREATE TABLE IF NOT EXISTS song_stats ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  path TEXT UNIQUE,"
        "  plays INTEGER NOT NULL DEFAULT 0,"
        "  skips INTEGER NOT NULL DEFAULT 0,"
        "  searches INTEGER NOT NULL DEFAULT 0,"
        "  score REAL NOT NULL DEFAULT 0"
        ");";

    char* errMsg = nullptr;
    rc = sqlite3_exec(*db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "DB init error: " << errMsg << "\n";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

void increment_play(sqlite3* db, const std::string& path) {
    const char* sql =
        "INSERT INTO song_stats(path, plays, score) "
        "VALUES(?1, 1, 3.0) "
        "ON CONFLICT(path) DO UPDATE SET "
        "  plays  = plays + 1,"
        "  score  = score + 3.0;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "prepare failed: " << sqlite3_errmsg(db) << "\n";
        return;
    }

    sqlite3_bind_text(stmt, 1, path.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "step failed: " << sqlite3_errmsg(db) << "\n";
    }

    sqlite3_finalize(stmt);
}
