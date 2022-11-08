`default_nettype none

typedef enum {
    MAT_CACHE_READ_DIAG,
    MAT_CACHE_READ_ROW,
    MAT_CACHE_READ_COL
} MatCacheReadType_t;

module MatCache
    #(parameter WIDTH = 128,
                WIDTH_ADDR_SIZE = 1 + $clog2(WIDTH),
                CACHE_SIZE = 4,
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE))
    (input logic clock,
     input logic read_enable,
     input logic write_enable,
     input logic transpose_enable,
     input MatCacheReadType_t read_type,
     input logic [CACHE_ADDR_SIZE - 1 : 0] read_addr1,
     input logic [CACHE_ADDR_SIZE - 1 : 0] read_addr2,
     input logic [WIDTH_ADDR_SIZE - 1 : 0] read_param,
     input logic [CACHE_ADDR_SIZE - 1 : 0] write_addr1,
     input logic [CACHE_ADDR_SIZE - 1 : 0] write_addr2,
     input logic [WIDTH_ADDR_SIZE - 1 : 0] write_diag,
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
                    if (write_addr1 == blk) begin
                        cache[blk][i][j] <= cache[blk][j][i];
                    end
                end
            end

        end
    endgenerate


    shortreal diag_out[WIDTH - 1 : 0], row_out[WIDTH - 1 : 0],
        col_out[WIDTH - 1 : 0];

    // Read diagonal memory
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                if (i <= read_param) begin
                    diag_out[i] = cache[read_addr1][i][read_param - i];
                end else begin
                    diag_out[i] = cache[read_addr2][i][WIDTH + read_param - i];
                end
                row_out[i] = cache[read_addr1][read_param][i];
                col_out[i] = cache[read_addr1][i][read_param];
            end
        end
    endgenerate

    always_comb begin
        unique case (read_type)
            MAT_CACHE_READ_DIAG: data_out = diag_out;
            MAT_CACHE_READ_ROW: data_out = row_out;
            MAT_CACHE_READ_COL: data_out = col_out;
        endcase
    end

endmodule: MatCache
