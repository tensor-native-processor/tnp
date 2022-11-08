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
     input logic [WIDTH_ADDR_SIZE - 1 : 0] write_param,
     input shortreal data_in[WIDTH - 1 : 0],
     output shortreal data_out[WIDTH - 1 : 0]);

    // Cache memory
    shortreal cache[CACHE_SIZE - 1 : 0][WIDTH - 1 : 0][WIDTH - 1 : 0];

    genvar blk, i, j;

    logic match_wr1[CACHE_SIZE - 1 : 0], match_wr2[CACHE_SIZE - 1 : 0];

    // Match two write addresses
    generate
        for (blk = 0;blk < CACHE_SIZE;blk++) begin
            assign match_wr1[blk] = blk == write_addr1;
            assign match_wr2[blk] = (blk == write_addr2) & write_enable;
        end
    endgenerate

    // Write into matrix in cache
    generate
        for (blk = 0;blk < CACHE_SIZE;blk++)
        for (i = 0;i < WIDTH;i++)
        for (j = 0;j < WIDTH;j++) begin

            always_ff @(posedge clock) begin
                if (match_wr1[blk]) begin
                    if (transpose_enable) begin
                        // Matrix transpose
                        cache[blk][i][j] <= cache[blk][j][i];
                    end else if (write_enable) begin
                        // Write left half
                        if (i + j == write_param) begin
                            cache[blk][i][j] <= data_in[i];
                        end
                    end
                end else if (match_wr2[blk]) begin
                    // Write right half
                    if (i + j == WIDTH + write_param) begin
                        cache[blk][i][j] <= data_in[i];
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
