`default_nettype none

typedef enum {
    MAT_VEC_UNIT_OP_ZERO,
    MAT_VEC_UNIT_OP_LOAD,
    MAT_VEC_UNIT_OP_ADD
} MatVecUnitOp_t;

module MatVecUnit
    #(parameter WIDTH = 128)
    (input logic clock,
     input shortreal data_in[WIDTH-1:0],
     output shortreal data_out[WIDTH-1:0],
     input MatVecUnitOp_t op);

    genvar i;
    shortreal mem[WIDTH-1:0];

    // Register operation
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_ff @(posedge clock) begin
                case (op)
                MAT_VEC_UNIT_OP_LOAD: begin
                    mem[i] <= data_in[i];
                end
                endcase
            end
        end
    endgenerate

    // Arithmetic operation
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                case (op)
                MAT_VEC_UNIT_OP_ZERO: begin
                    data_out[i] = 0;
                end
                MAT_VEC_UNIT_OP_ADD: begin
                    data_out[i] = data_in[i] + mem[i];
                end
                endcase
            end
        end
    endgenerate

endmodule: MatVecUnit
