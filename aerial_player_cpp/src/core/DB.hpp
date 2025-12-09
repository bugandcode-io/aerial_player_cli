#pragma once

#include <string>

// Forward declaration so we can use sqlite3* in the header
// even if sqlite3.h is only included in DB.cpp
struct sqlite3;

class PlayDatabase
{
public:
    explicit PlayDatabase(const std::string& path);
    ~PlayDatabase();

    bool ok() const { return ok_; }

    void logPlay(const std::string& trackPath);
    void logSkip(const std::string& trackPath);
    void logFinished(const std::string& trackPath);

private:
    bool initSchema();
    void logEvent(const std::string& trackPath,
                  const std::string& eventType);

    std::string dbPath_;
    sqlite3*    db_;   // <— REAL sqlite3* now (not void*)
    bool        ok_;
};
