#include "mat_format.h"
#include <cstdio>

int main() {
    MatInstruction instr;
    instr.op = MatInstruction::SET_WEIGHT;
    printf("%d\n", instr.op);
    printf("%d\n", MatInstruction::SET_WEIGHT);
    printf("%d\n", MatInstruction::operands(MatInstruction::RECV_COL)[0]);
    return 0;
}
