`default_nettype none

// Matrix register
module MatReg
    #(parameter WIDTH = 128,
                WIDTH_ADDR_SIZE = $clog2(WIDTH))
    (input logic clock,
     input MatDataReadOp_t read_op,
     input logic [WIDTH_ADDR_SIZE-1:0] read_param1, read_param2,
     input MatDataWriteOp_t write_op,
     input logic [WIDTH_ADDR_SIZE-1:0] write_param1, write_param2,
     input shortreal data_in[WIDTH-1:0],
     output shortreal data_out[WIDTH-1:0]);

    // Matrix register
    shortreal mem[WIDTH-1:0][WIDTH-1:0];

    genvar i, j;
    
    // Write into register
    generate
        for (i = 0;i < WIDTH;i++)
        for (j = 0;j < WIDTH;j++) begin

            always_ff @(posedge clock) begin
                unique case (write_op)
                    MAT_DATA_WRITE_DISABLE: begin
                    end
                    MAT_DATA_WRITE_TRANSPOSE: begin
                        // Matrix transpose
                        mem[i][j] <= mem[j][i];
                    end
                    MAT_DATA_WRITE_ROW: begin
                        // Write row
                        if (i == write_param1) begin
                            mem[i][j] <= data_in[j];
                        end
                    end
                    MAT_DATA_WRITE_COL: begin
                        // Write column
                        if (j == write_param1) begin
                            mem[i][j] <= data_in[i];
                        end
                    end
                    MAT_DATA_WRITE_SCALAR: begin
                        // Write scalar
                        if (i == write_param1 && j == write_param2) begin
                            mem[i][j] <= data_in[0];
                        end
                    end
                    MAT_DATA_WRITE_DIAG1: begin
                        // Write primary diagonal
                        if (i + j == write_param1) begin
                            mem[i][j] <= data_in[i];
                        end
                    end
                    MAT_DATA_WRITE_DIAG2: begin
                        // Write secondary diagonal
                        if (i + j == WIDTH + write_param1) begin
                            mem[i][j] <= data_in[i];
                        end
                    end
                    MAT_DATA_WRITE_DIAG: begin
                        // Write both diagonals
                        if (i + j == write_param1) begin
                            mem[i][j] <= data_in[i];
                        end
                        if (i + j == WIDTH + write_param1) begin
                            mem[i][j] <= data_in[i];
                        end
                    end
                endcase
            end
        end
    endgenerate
    
    // Read row/col/diag from register
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (read_op)
                    MAT_DATA_READ_DISABLE: begin
                        data_out[i] = 0;
                    end
                    MAT_DATA_READ_ROW: begin
                        data_out[i] = mem[read_param1][i];
                    end
                    MAT_DATA_READ_COL: begin
                        data_out[i] = mem[i][read_param1];
                    end
                    MAT_DATA_READ_SCALAR: begin
                        data_out[i] = i == 0 ? mem[read_param1][read_param2] : 0;
                    end
                    MAT_DATA_READ_DIAG: begin
                        // Test primary/secondary diagonal
                        if (i <= read_param1) begin
                            data_out[i] = mem[i][read_param1 - i];
                        end else begin
                            data_out[i] = mem[i][WIDTH + read_param1 - i];
                        end
                    end
                endcase
            end
        end
    endgenerate

endmodule: MatReg
