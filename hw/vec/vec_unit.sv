`default_nettype none

typedef enum {
    VEC_OP_ADD,
    VEC_OP_SUB,
    VEC_OP_DOT,
    VEC_OP_SCALE
} VecOp_t;

module VecUnit
    #(parameter WIDTH = 128)
    (input logic clock,
     input real data_k,
     input real data_in1[WIDTH - 1 : 0],
     input real data_in2[WIDTH - 1 : 0],
     output real data_out[WIDTH - 1 : 0],
     input VecOp_t op);

    genvar i;
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (op)
                    VEC_OP_ADD: begin
                        data_out[i] = data_in1[i] + data_in2[i];
                    end
                    VEC_OP_SUB: begin
                        data_out[i] = data_in1[i] - data_in2[i];
                    end
                    VEC_OP_DOT: begin
                        data_out[i] = data_in1[i] * data_in2[i];
                    end
                    VEC_OP_SCALE: begin
                        data_out[i] = data_in1[i] * data_k;
                    end
                endcase
            end
        end
    endgenerate

endmodule: VecUnit
