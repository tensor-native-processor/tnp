`default_nettype none

// Vector cache system
module VecCache
    #(parameter WIDTH = 128,
                WIDTH_ADDR_SIZE = $clog2(WIDTH),
                CACHE_SIZE = 4,
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE))
    (input logic clock,
     input VecDataReadOp_t read_op,
     input logic [CACHE_ADDR_SIZE-1:0] read_addr,
     input VecDataWriteOp_t write_op,
     input logic [CACHE_ADDR_SIZE-1:0] write_addr,
     input logic [WIDTH_ADDR_SIZE-1:0] write_param,
     input shortreal data_in[WIDTH-1:0],
     output shortreal data_out[WIDTH-1:0]);

    VecDataWriteOp_t reg_write_op[CACHE_SIZE-1:0];
    shortreal reg_data_out[CACHE_SIZE-1:0][WIDTH-1:0];

    VecReg #(.WIDTH(WIDTH)) vec_reg[CACHE_SIZE-1:0](
        .clock, .read_op,
        .write_op(reg_write_op),
        .write_param, .data_in,
        .data_out(reg_data_out)
    );

    genvar blk;

    // Write operations
    generate
        for (blk = 0;blk < CACHE_SIZE;blk++) begin
            always_comb begin
                reg_write_op[blk] = blk == write_addr ?
                    write_op : VEC_DATA_WRITE_DISABLE;
            end
        end
    endgenerate

    // Read operations
    always_comb begin
        data_out = reg_data_out[read_addr];
    end

endmodule: VecCache
