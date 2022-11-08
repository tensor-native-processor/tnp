`default_nettype none

module MatCache
    #(parameter WIDTH = 128,
                CACHESIZE = 256,
                CACHEADDR = $clog2(CACHESIZE))
    (input logic mode,
     input logic clock,
     input logic [CACHEADDR - 1 : 0] addr1,
     input logic [CACHEADDR - 1 : 0] addr2,
     input shortreal data_in[WIDTH - 1 : 0],
     output shortreal data_out[WIDTH - 1 : 0]);

    // Cache memory
    shortreal mem[CACHESIZE - 1 : 0];


endmodule: MatCache
