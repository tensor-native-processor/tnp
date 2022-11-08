`default_nettype none

module MatCache
    #(parameter WIDTH = 128,
                CACHESIZE = 256,
                CACHEADDR = $clog2(CACHESIZE),
                FPSIZE = 16)
    (input logic mode,
     input logic clock,
     input logic [CACHEADDR - 1 : 0] addr,
     output logic [WIDTH - 1 : 0][FPSIZE - 1 : 0] cout);

    // Cache memory
    logic [FPSIZE - 1 : 0] mem[CACHESIZE - 1 : 0];


endmodule: MatCache
