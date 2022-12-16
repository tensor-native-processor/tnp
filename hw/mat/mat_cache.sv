`default_nettype none

// Matrix cache system
module MatCache
    #(parameter WIDTH = 128,
                CACHE_SIZE = 4,

                // Auto-generated sizes
                WIDTH_ADDR_SIZE = $clog2(WIDTH),
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE))
    (input logic clock,
     input MatDataReadOp_t read_op,
     input logic [CACHE_ADDR_SIZE-1:0] read_addr1, read_addr2,
     input logic [WIDTH_ADDR_SIZE-1:0] read_param1, read_param2,
     input MatDataWriteOp_t write_op,
     input logic [CACHE_ADDR_SIZE-1:0] write_addr1, write_addr2,
     input logic [WIDTH_ADDR_SIZE-1:0] write_param1, write_param2,
     input shortreal data_in[WIDTH-1:0],
     output shortreal data_out[WIDTH-1:0]);

    MatDataWriteOp_t reg_write_op[CACHE_SIZE-1:0];
    shortreal reg_data_out[CACHE_SIZE-1:0][WIDTH-1:0];

    MatReg #(.WIDTH(WIDTH)) mat_reg[CACHE_SIZE-1:0](
        .clock, .read_op, .read_param1, .read_param2,
        .write_op(reg_write_op), .write_param1, .write_param2,
        .data_in, .data_out(reg_data_out)
    );

    genvar blk, i;

    // Assign write operations to each matrix register
    generate
        for (blk = 0;blk < CACHE_SIZE;blk++) begin

        always_comb begin
            reg_write_op[blk] = MAT_DATA_WRITE_DISABLE;
            unique case (write_op)
                MAT_DATA_WRITE_DISABLE,
                MAT_DATA_WRITE_TRANSPOSE,
                MAT_DATA_WRITE_XFLIP,
                MAT_DATA_WRITE_YFLIP,
                MAT_DATA_WRITE_ZERO,
                MAT_DATA_WRITE_ROW,
                MAT_DATA_WRITE_COL,
                MAT_DATA_WRITE_SCALAR,
                MAT_DATA_WRITE_DIAG1,
                MAT_DATA_WRITE_DIAG2: begin
                    reg_write_op[blk] = blk == write_addr1 ? 
                        write_op : MAT_DATA_WRITE_DISABLE;
                end
                MAT_DATA_WRITE_DIAG: begin
                    if (write_addr1 == write_addr2) begin
                        reg_write_op[blk] = blk == write_addr1 ?
                            MAT_DATA_WRITE_DIAG : MAT_DATA_WRITE_DISABLE;
                    end else begin
                        reg_write_op[blk] = blk == write_addr1 ?
                            MAT_DATA_WRITE_DIAG1 : blk == write_addr2 ?
                            MAT_DATA_WRITE_DIAG2 : MAT_DATA_WRITE_DISABLE;
                    end
                end
            endcase
        end

        end
    endgenerate


    // Assign output to data_out
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (read_op)
                    MAT_DATA_READ_DISABLE: begin
                        data_out[i] = 0;
                    end
                    MAT_DATA_READ_ROW,
                    MAT_DATA_READ_COL,
                    MAT_DATA_READ_SCALAR: begin
                        data_out[i] = reg_data_out[read_addr1][i];
                    end
                    MAT_DATA_READ_DIAG: begin
                        if (i <= read_param1) begin
                            data_out[i] = reg_data_out[read_addr1][i];
                        end else begin
                            data_out[i] = reg_data_out[read_addr2][i];
                        end
                    end
                endcase
            end
        end
    endgenerate

endmodule: MatCache
