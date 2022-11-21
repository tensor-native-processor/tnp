`default_nettype none

// Vector register
module VecReg
    #(parameter WIDTH = 128,
                WIDTH_ADDR_SIZE = $clog2(WIDTH))
    (input logic clock,
     input VecDataReadOp_t read_op,
     input logic [WIDTH_ADDR_SIZE-1:0] read_param,
     input VecDataWriteOp_t write_op,
     input logic [WIDTH_ADDR_SIZE-1:0] write_param,
     input shortreal data_in[WIDTH-1:0],
     output shortreal data_out[WIDTH-1:0]);

    // Vector register
    shortreal mem[WIDTH-1:0];

    genvar i;

    // Write operation
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_ff @(posedge clock) begin
                unique case (write_op)
                    VEC_DATA_WRITE_DISABLE: begin
                    end
                    VEC_DATA_WRITE_VEC: begin
                        mem[i] <= data_in[i];
                    end
                    VEC_DATA_WRITE_SCALAR: begin
                        if (i == write_param) begin
                            mem[i] <= data_in[0];
                        end
                    end
                endcase
            end
        end
    endgenerate

    // Read operation
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (read_op)
                    VEC_DATA_READ_DISABLE: begin
                        data_out[i] = 0;
                    end
                    VEC_DATA_READ_VEC: begin
                        data_out[i] = mem[i];
                    end
                    VEC_DATA_READ_SCALAR: begin
                        data_out[i] = i == 0 ? mem[read_param] : 0;
                    end
                endcase
            end
        end
    endgenerate

endmodule: VecReg
