#include "error.h"

#include <iostream>
#include <cstdlib>

void FatalError(std::string msg) {
    std::cerr << "Fatal error: " << msg << std::endl;
    exit(1);
}
