`default_nettype none

// MatCore Instruction Memory
module MatInstMem
    #(parameter INST_MEM_SIZE = 100,
                INST_MEM_ADDR_SIZE = 32,
                INST_MEM_WIDTH_BYTES = 16,

                // Auto-generated sizes
                INST_MEM_WIDTH_SIZE = 8 * INST_MEM_WIDTH_BYTES)
    (input logic [INST_MEM_ADDR_SIZE-1:0] addr,
     output logic [INST_MEM_WIDTH_SIZE-1:0] value);

    // Instruction memory (initialized by testbench)
    logic [7:0] inst_mem[INST_MEM_SIZE-1:0];

    // Output to value
    genvar i;
    generate
        for (i = 0;i < INST_MEM_WIDTH_BYTES;i++)
            assign value[i*8+7:i*8] = inst_mem[addr + i][7:0];
    endgenerate

endmodule: MatInstMem

// MatCore Data Memory
module MatDataMem
    #(parameter DATA_MEM_SIZE = 100,
                DATA_MEM_ADDR_SIZE = 32,
                DATA_MEM_WIDTH_SIZE = 16)
    (input logic [DATA_MEM_ADDR_SIZE-1:0] addr,
     output shortreal value[DATA_MEM_WIDTH_SIZE-1:0]);

    // Data memory (initialized by testbench)
    logic data_mem[DATA_MEM_SIZE-1:0];

    // Output to value
    genvar i;
    generate
        for (i = 0;i < DATA_MEM_WIDTH_SIZE;i++)
            assign value[i] = data_mem[addr + i];
    endgenerate

endmodule: MatDataMem
