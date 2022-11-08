`default_nettype none

module MatCache
    #(parameter WIDTH = 128,
                DIAG_SIZE = 1 + $clog2(WIDTH),
                CACHE_SIZE = 256,
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE))
    (input logic clock,
     input logic read_enable,
     input logic write_enable,
     input logic [CACHE_ADDR_SIZE - 1 : 0] read_addr1,
     input logic [DIAG_SIZE - 1 : 0] read_diag1,
     input logic [CACHE_ADDR_SIZE - 1 : 0] read_addr2,
     input logic [DIAG_SIZE - 1 : 0] read_diag2,
     input logic [CACHE_ADDR_SIZE - 1 : 0] write_addr1,
     input logic [DIAG_SIZE - 1 : 0] write_diag1,
     input logic [CACHE_ADDR_SIZE - 1 : 0] write_addr2,
     input logic [DIAG_SIZE - 1 : 0] write_diag2,
     input shortreal data_in[WIDTH - 1 : 0],
     output shortreal data_out[WIDTH - 1 : 0]);

    // Cache memory
    shortreal mem[CACHE_SIZE - 1 : 0];


endmodule: MatCache
