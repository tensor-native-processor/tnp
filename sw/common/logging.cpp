#include "logging.h"

#include <iostream>
#include <ctime>

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
    std::cerr << s << ": " << msg << std::endl;
}
