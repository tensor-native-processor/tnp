`default_nettype none

// Top-level module for VecCore
module VecCore
    #(parameter WIDTH = 16,
                CACHE_SIZE = 8,

                INST_MEM_SIZE = 2048,
                DATA_MEM_SIZE = 2048,

                INST_MEM_ADDR_SIZE = 32,
                DATA_MEM_ADDR_SIZE = 32,
                INST_MEM_WIDTH_BYTES = 16,

                SWITCH_WIDTH = 16,
                SWITCH_CORE_SIZE = 4,

                // Auto-gen
                WIDTH_ADDR_SIZE = $clog2(WIDTH),
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE),

                INST_MEM_WIDTH_SIZE = 8 * INST_MEM_WIDTH_BYTES,

                SWITCH_CORE_ADDR_SIZE = $clog2(SWITCH_CORE_SIZE)
    )
    (input logic clock, reset,
     output logic done,

     // Switch send
     output logic switch_send_ready,
     output logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_send_core_idx,
     output shortreal switch_send_data[SWITCH_WIDTH-1:0],
     input logic switch_send_ok,

     // Switch recv
     output logic switch_recv_request,
     output logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_recv_core_idx,
     input logic switch_recv_ready,
     input shortreal switch_recv_data[SWITCH_WIDTH-1:0]
    );

    // Lots of wires
    // Interface with VecUnit
    VecUnitOp_t unit_op;
    shortreal unit_data_inK;
    shortreal unit_data_in1[WIDTH-1:0];
    shortreal unit_data_in2[WIDTH-1:0];
    shortreal unit_data_out[WIDTH-1:0];

    // Interface with VecCache
    VecDataReadOp_t cache_read_op;
    logic [CACHE_ADDR_SIZE-1:0] cache_read_addr;
    logic [WIDTH_ADDR_SIZE-1:0] cache_read_param;
    VecDataWriteOp_t cache_write_op;
    logic [CACHE_ADDR_SIZE-1:0] cache_write_addr;
    logic [WIDTH_ADDR_SIZE-1:0] cache_write_param;
    shortreal cache_data_in[WIDTH-1:0];
    shortreal cache_data_out[WIDTH-1:0];

    // Interface with memory
    logic [INST_MEM_ADDR_SIZE-1:0] inst_mem_read_addr;
    logic [INST_MEM_WIDTH_SIZE-1:0] inst_mem_data_out;
    logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_read_addr;
    shortreal data_mem_data_out[WIDTH-1:0];
    VecDataMemWriteOp_t data_mem_write_op;
    logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_write_addr;
    shortreal data_mem_data_in[WIDTH-1:0];

    // Module Instantiations
    VecControl #(.WIDTH(WIDTH), .CACHE_SIZE(CACHE_SIZE),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .DATA_MEM_ADDR_SIZE(DATA_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_SIZE(INST_MEM_WIDTH_SIZE),
        .SWITCH_WIDTH(SWITCH_WIDTH),
        .SWITCH_CORE_SIZE(SWITCH_CORE_SIZE)
    ) control(.*);
    VecUnit #(.WIDTH(WIDTH)) unit(.clock, 
        .op(unit_op),
		.data_inK(unit_data_inK),
        .data_in1(unit_data_in1),
        .data_in2(unit_data_in2),
        .data_out(unit_data_out)
    );
    VecCache #(.WIDTH(WIDTH), .CACHE_SIZE(CACHE_SIZE)) cache(.clock,
        .read_op(cache_read_op),
        .read_addr(cache_read_addr),
        .read_param(cache_read_param),
        .write_op(cache_write_op),
        .write_addr(cache_write_addr),
        .write_param(cache_write_param),
        .data_in(cache_data_in), .data_out(cache_data_out)
    );
    VecInstMem #(.INST_MEM_SIZE(INST_MEM_SIZE),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_BYTES(INST_MEM_WIDTH_BYTES)
    ) inst_mem(
        .read_addr(inst_mem_read_addr), .data_out(inst_mem_data_out)
    );
    VecDataMem #(.DATA_MEM_SIZE(DATA_MEM_SIZE),
        .DATA_MEM_ADDR_SIZE(DATA_MEM_ADDR_SIZE),
        .DATA_MEM_WIDTH_SIZE(WIDTH)
    ) data_mem(.clock(clock),
        .read_addr(data_mem_read_addr), .data_out(data_mem_data_out),
        .write_op(data_mem_write_op),
        .write_addr(data_mem_write_addr),
        .data_in(data_mem_data_in)
    );

endmodule: VecCore
