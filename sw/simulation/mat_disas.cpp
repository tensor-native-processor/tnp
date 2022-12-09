#include "mat_program.h"
#include "error.h"

#include <cstdio>
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        FatalError("Insufficient argc");
    }
    MatCoreProgram prog;
    prog.fromBinary(LoadProgram(argv[1]));

    std::cout << prog.toText() << std::endl;
    return 0;
}
