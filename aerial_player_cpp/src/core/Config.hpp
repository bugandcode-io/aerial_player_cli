#pragma once
#include <string>

struct AerialConfig {
    std::string db_path;
    int port = 5050;
    bool scan_recursive = true;
    int default_volume = 50; 
};

AerialConfig load_config();
