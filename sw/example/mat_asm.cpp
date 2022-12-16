#include "mat_program.h"
#include <cstdio>
#include <iostream>

int main() {
    MatCoreProgram prog;

    // set weight 258
    prog.append({MatCoreInstDefn::SET_WEIGHT, {
        {MatCoreInstDefn::M1, 258}
    }});

    // load row 3 5 3
    prog.append({MatCoreInstDefn::LOAD_ROW, {
        {MatCoreInstDefn::ADDR, 3},
        {MatCoreInstDefn::M1, 5},
        {MatCoreInstDefn::ROW_IDX, 3}
    }});

    // load col 45 4 2
    prog.append({MatCoreInstDefn::LOAD_COL, {
        {MatCoreInstDefn::ADDR, 45},
        {MatCoreInstDefn::M1, 4},
        {MatCoreInstDefn::COL_IDX, 2}
    }});

    // load scalar 67 2 8 9
    prog.append({MatCoreInstDefn::LOAD_SCALAR, {
        {MatCoreInstDefn::ADDR, 67},
        {MatCoreInstDefn::M1, 2},
        {MatCoreInstDefn::ROW_IDX, 8},
        {MatCoreInstDefn::COL_IDX, 9}
    }});

    // multiply 23 259
    prog.append({MatCoreInstDefn::MULTIPLY, {
        {MatCoreInstDefn::Md, 23},
        {MatCoreInstDefn::M1, 259}
    }});

    // store col 56 7 69
    prog.append({MatCoreInstDefn::STORE_COL, {
        {MatCoreInstDefn::ADDR, 56},
        {MatCoreInstDefn::M1, 7},
        {MatCoreInstDefn::COL_IDX, 69}
    }});

    // store row 100 5 3
    prog.append({MatCoreInstDefn::STORE_ROW, {
        {MatCoreInstDefn::ADDR, 100},
        {MatCoreInstDefn::M1, 5},
        {MatCoreInstDefn::ROW_IDX, 3}
    }});

    // store col 150 4 2
    prog.append({MatCoreInstDefn::STORE_COL, {
        {MatCoreInstDefn::ADDR, 150},
        {MatCoreInstDefn::M1, 4},
        {MatCoreInstDefn::COL_IDX, 2}
    }});

    // store scalar 160 5 3 2
    prog.append({MatCoreInstDefn::STORE_SCALAR, {
        {MatCoreInstDefn::ADDR, 160},
        {MatCoreInstDefn::M1, 5},
        {MatCoreInstDefn::ROW_IDX, 3},
        {MatCoreInstDefn::COL_IDX, 2}
    }});

    // store scalar 162 2 8 9
    prog.append({MatCoreInstDefn::STORE_SCALAR, {
        {MatCoreInstDefn::ADDR, 162},
        {MatCoreInstDefn::M1, 2},
        {MatCoreInstDefn::ROW_IDX, 8},
        {MatCoreInstDefn::COL_IDX, 9}
    }});

    // halt
    prog.append({MatCoreInstDefn::HALT, {
    }});

    std::cout << prog.toText() << std::endl;

    TNPProgramBinary bin = prog.toBinary();
    SaveProgram(bin, "inst_mem.txt");

    MatCoreProgram prog2;
    prog2.fromBinary(bin);
    std::cout << "Next: " << prog2.toText() << std::endl;

    return 0;
}
