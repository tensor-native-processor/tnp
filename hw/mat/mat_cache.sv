`default_nettype none

module MatCache
    #(parameter WIDTH = 128,
                DIAG_SIZE = 1 + $clog2(WIDTH),
                CACHE_SIZE = 4,
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE))
    (input logic clock,
     input logic read_enable,
     input logic write_enable,
     input logic transpose_enable,
     input logic [CACHE_ADDR_SIZE - 1 : 0] read_addr1,
     input logic [DIAG_SIZE - 1 : 0] read_diag1,
     input logic [CACHE_ADDR_SIZE - 1 : 0] read_addr2,
     input logic [DIAG_SIZE - 1 : 0] read_diag2,
     input logic [CACHE_ADDR_SIZE - 1 : 0] write_addr1,
     input logic [DIAG_SIZE - 1 : 0] write_diag1,
     input logic [CACHE_ADDR_SIZE - 1 : 0] write_addr2,
     input logic [DIAG_SIZE - 1 : 0] write_diag2,
     input logic [CACHE_ADDR_SIZE - 1 : 0] transpose_addr,
     input shortreal data_in[WIDTH - 1 : 0],
     output shortreal data_out[WIDTH - 1 : 0]);

    // Cache memory
    shortreal cache[CACHE_SIZE - 1 : 0][WIDTH - 1 : 0][WIDTH - 1 : 0];

    genvar blk, i, j;

    // Transpose any matrix in cache
    generate
        for (blk = 0;blk < CACHE_SIZE;blk++)
        for (i = 0;i < WIDTH;i++)
        for (j = 0;j < WIDTH;j++) begin

            always_ff @(posedge clock) begin

                if (transpose_enable) begin
                    if (transpose_addr == blk) begin
                        cache[blk][i][j] <= cache[blk][j][i];
                    end
                end
            end

        end
    endgenerate

endmodule: MatCache
