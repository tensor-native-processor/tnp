`include "mat_mem_type.sv"
`include "mat_data_type.sv"

// Top-level module for MatCore
module MatCore
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
    logic unit_set_weight;
    logic [WIDTH_ADDR_SIZE-1:0] unit_set_weight_row;
    shortreal unit_data_in[WIDTH-1:0];
    shortreal unit_data_out[WIDTH-1:0];

    MatDataReadOp_t cache_read_op;
    logic [CACHE_ADDR_SIZE-1:0] cache_read_addr1, cache_read_addr2;
    logic [WIDTH_ADDR_SIZE-1:0] cache_read_param1, cache_read_param2;
    MatDataWriteOp_t cache_write_op;
    logic [CACHE_ADDR_SIZE-1:0] cache_write_addr1, cache_write_addr2;
    logic [WIDTH_ADDR_SIZE-1:0] cache_write_param1, cache_write_param2;
    shortreal cache_data_in[WIDTH-1:0];
    shortreal cache_data_out[WIDTH-1:0];

    logic [INST_MEM_ADDR_SIZE-1:0] inst_mem_read_addr, inst_mem_read_addr2;
    logic [INST_MEM_WIDTH_SIZE-1:0] inst_mem_data_out, inst_mem_data_out2;
    logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_read_addr;
    shortreal data_mem_data_out[WIDTH-1:0];
    MatDataMemWriteOp_t data_mem_write_op;
    logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_write_addr;
    shortreal data_mem_data_in[WIDTH-1:0];


    // Module instantiations
    MatControl #(.WIDTH(WIDTH), .CACHE_SIZE(CACHE_SIZE),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .DATA_MEM_ADDR_SIZE(DATA_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_SIZE(INST_MEM_WIDTH_SIZE),
        .SWITCH_WIDTH(SWITCH_WIDTH),
        .SWITCH_CORE_SIZE(SWITCH_CORE_SIZE)
    ) control(.*);
    MatUnit #(.WIDTH(WIDTH)) unit(.clock, .set_weight(unit_set_weight),
        .set_weight_row(unit_set_weight_row), .data_in(unit_data_in),
        .data_out(unit_data_out)
    );
    MatCache #(.WIDTH(WIDTH), .CACHE_SIZE(CACHE_SIZE)) cache(.clock,
        .read_op(cache_read_op),
        .read_addr1(cache_read_addr1), .read_addr2(cache_read_addr2),
        .read_param1(cache_read_param1), .read_param2(cache_read_param2),
        .write_op(cache_write_op),
        .write_addr1(cache_write_addr1), .write_addr2(cache_write_addr2),
        .write_param1(cache_write_param1), .write_param2(cache_write_param2),
        .data_in(cache_data_in), .data_out(cache_data_out)
    );
    MatInstMem #(.INST_MEM_SIZE(INST_MEM_SIZE),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_BYTES(INST_MEM_WIDTH_BYTES)
    ) inst_mem(.read_addr(inst_mem_read_addr), .data_out(inst_mem_data_out),
        .read_addr2(inst_mem_read_addr2), .data_out2(inst_mem_data_out2)
    );
    MatDataMem #(.DATA_MEM_SIZE(DATA_MEM_SIZE),
        .DATA_MEM_ADDR_SIZE(DATA_MEM_ADDR_SIZE),
        .DATA_MEM_WIDTH_SIZE(WIDTH)
    ) data_mem(.clock(clock),
        .read_addr(data_mem_read_addr), .data_out(data_mem_data_out),
        .write_op(data_mem_write_op),
        .write_addr(data_mem_write_addr),
        .data_in(data_mem_data_in)
    );

endmodule: MatCore
