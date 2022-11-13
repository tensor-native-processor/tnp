`default_nettype none

typedef enum {
    VEC_UNIT_OP_ADD,
    VEC_UNIT_OP_SUB,
    VEC_UNIT_OP_DOT,
    VEC_UNIT_OP_SCALE,
    VEC_UNIT_OP_DELTA,
    VEC_UNIT_OP_ACT_SIGMOID,
    VEC_UNIT_OP_ACT_TANH,
    VEC_UNIT_OP_ACT_RELU
} VecUnitOp_t;

module VecUnit
    #(parameter WIDTH = 128)
    (input logic clock,
     input shortreal data_inK,
     input shortreal data_in1[WIDTH - 1 : 0],
     input shortreal data_in2[WIDTH - 1 : 0],
     output shortreal data_out[WIDTH - 1 : 0],
     input VecUnitOp_t op);

    genvar i;
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (op)
                    VEC_UNIT_OP_ADD: begin
                        data_out[i] = data_in1[i] + data_in2[i];
                    end
                    VEC_UNIT_OP_SUB: begin
                        data_out[i] = data_in1[i] - data_in2[i];
                    end
                    VEC_UNIT_OP_DOT: begin
                        data_out[i] = data_in1[i] * data_in2[i];
                    end
                    VEC_UNIT_OP_SCALE: begin
                        data_out[i] = data_in1[i] * data_inK;
                    end
                    VEC_UNIT_OP_DELTA: begin
                        data_out[i] = data_in1[i] + data_inK;
                    end
                    VEC_UNIT_OP_ACT_SIGMOID: begin
                        data_out[i] = 1.0 / (1.0 + $exp(-data_in1[i]));
                    end
                    VEC_UNIT_OP_ACT_TANH: begin
                        data_out[i] = ($exp(data_in1[i]) - $exp(-data_in1[i]))
                            / ($exp(data_in1[i]) + $exp(-data_in1[i]));
                    end
                    VEC_UNIT_OP_ACT_RELU: begin
                        data_out[i] = data_in1[i] >= 0 ? 1 : 0;
                    end
                endcase
            end
        end
    endgenerate

endmodule: VecUnit
