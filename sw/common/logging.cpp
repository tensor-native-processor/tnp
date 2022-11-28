#include "logging.h"

#include <iostream>
#include <cstdio>
#include <ctime>
#include <unistd.h>

// Print info
void LogInfo(const std::string& msg) {
    time_t t = time(0);
    std::string s = ctime(&t);
    s.resize(s.size() - 1);
    std::cerr << s << ": " << msg << std::endl;
}

// Print warning
void LogWarning(const std::string& msg) {
    time_t t = time(0);
    std::string s = ctime(&t);
    s.resize(s.size() - 1);
    if (isatty(STDERR_FILENO)) {
        std::cerr << s << ": " << "\033[1;33m" << msg << "\033[0m" << std::endl;
    } else  {
        std::cerr << s << ": " << msg << std::endl;
    }
}
