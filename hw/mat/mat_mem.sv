`include "mat_mem_type.sv"

// MatCore Instruction Memory
module MatInstMem
    #(parameter INST_MEM_SIZE = 100,
                INST_MEM_ADDR_SIZE = 32,
                INST_MEM_WIDTH_BYTES = 16,

                // Auto-generated sizes
                INST_MEM_WIDTH_SIZE = 8 * INST_MEM_WIDTH_BYTES)
    (input logic [INST_MEM_ADDR_SIZE-1:0] read_addr, read_addr2,
     output logic [INST_MEM_WIDTH_SIZE-1:0] data_out, data_out2);

    // Instruction memory (initialized by testbench)
    logic [7:0] inst_mem[INST_MEM_SIZE-1:0];

    // Output to value
    genvar i;
    generate
        for (i = 0;i < INST_MEM_WIDTH_BYTES;i++)
            assign data_out[i*8+7:i*8] = inst_mem[read_addr + i][7:0];
    endgenerate

    // Output second output
    generate
        for (i = 0;i < INST_MEM_WIDTH_BYTES;i++)
            assign data_out2[i*8+7:i*8] = inst_mem[read_addr2 + i][7:0];
    endgenerate

endmodule: MatInstMem


// MatCore Data Memory
module MatDataMem
    #(parameter DATA_MEM_SIZE = 100,
                DATA_MEM_ADDR_SIZE = 32,
                DATA_MEM_WIDTH_SIZE = 16,

                // Auto-generated sizes
                DATA_MEM_WIDTH_ADDR_SIZE = $clog2(DATA_MEM_WIDTH_SIZE)
    )
    (input logic clock,
     input logic [DATA_MEM_ADDR_SIZE-1:0] read_addr,
     output real data_out[DATA_MEM_WIDTH_SIZE-1:0],

     // Write
     input MatDataMemWriteOp_t write_op,
     input logic [DATA_MEM_ADDR_SIZE-1:0] write_addr,
     input real data_in[DATA_MEM_WIDTH_SIZE-1:0]
    );

    // Data memory (initialized by testbench)
    real data_mem[DATA_MEM_SIZE-1:0];

    // Read memory
    genvar i;
    generate
        for (i = 0;i < DATA_MEM_WIDTH_SIZE;i++)
            assign data_out[i] = data_mem[read_addr + i];
    endgenerate

    // Write memory
    generate
        for (i = 0;i < DATA_MEM_SIZE;i++)
            always_ff @(posedge clock) begin
               if ((write_op == MAT_DATA_MEM_WRITE_SINGLE &&
                       i == write_addr) ||
                   (write_op == MAT_DATA_MEM_WRITE_ALL &&
                       i >= write_addr &&
                       i < write_addr + DATA_MEM_WIDTH_SIZE))
                   data_mem[i] <= data_in[i - write_addr];
            end
    endgenerate

endmodule: MatDataMem
