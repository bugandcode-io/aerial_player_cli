#pragma once

#include <string>

struct sqlite3;  // forward declaration

class PlayDatabase {
public:
    explicit PlayDatabase(const std::string& path);
    ~PlayDatabase();

    // quick check if DB is usable
    bool ok() const { return db_ != nullptr; }

    void logPlay(const std::string& trackPath);
    void logSkip(const std::string& trackPath);
    void logFinished(const std::string& trackPath);

private:
    bool initSchema();
    void logEvent(const std::string& trackPath,
                  const std::string& eventType);

     std::string dbPath_;

#ifdef AERIAL_USE_SQLITE
    void* db_;   // actually sqlite3*, but we keep the include in DB.cpp
#else
    void* db_;   // unused in JSON mode; kept for layout simplicity
#endif
    bool ok_ = false;
};
