#include "program.h"
#include <fstream>
#include <bitset>

void SaveProgram(const TNPProgramBinary& prog, const std::string& filename) {
    std::ofstream ofs(filename);
    for (auto const &byte : prog) {
        std::bitset<8> bs((uint8_t)byte);
        ofs << bs << std::endl;
    }
    ofs.close();
}
