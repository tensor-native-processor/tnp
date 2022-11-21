`default_nettype none

// Main control unit for VecCore
module VecControl
    #(parameter OPCODE_TYPE_BYTES = 1,
                MEM_ADDR_TYPE_BYTES = 8,
                CORE_IDX_TYPE_BYTES = 1,
                REG_ADDR_TYPE_BYTES = 2,
                WIDTH_IDX_TYPE_BYTES = 2,

                INST_MEM_ADDR_SIZE = 32,
                DATA_MEM_ADDR_SIZE = 32,
                INST_MEM_WIDTH_SIZE = 16,

                WIDTH = 128,
                CACHE_SIZE = 4,

                // Auto-generated sizes
                OPCODE_TYPE_SIZE = 8 * OPCODE_TYPE_BYTES,
                MEM_ADDR_TYPE_SIZE = 8 * MEM_ADDR_TYPE_BYTES,
                CORE_IDX_TYPE_SIZE = 8 * CORE_IDX_TYPE_BYTES,
                REG_ADDR_TYPE_SIZE = 8 * REG_ADDR_TYPE_BYTES,
                WIDTH_IDX_TYPE_SIZE = 8 * WIDTH_IDX_TYPE_BYTES,

                WIDTH_ADDR_SIZE = $clog2(WIDTH),
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE))
    (input logic clock, reset,
     output logic done,

     // Interface with VecUnit
     output VecUnitOp_t unit_op,
     output shortreal unit_data_inK,
     output shortreal unit_data_in1[WIDTH-1:0],
     output shortreal unit_data_in2[WIDTH-1:0],
     input shortreal unit_data_out[WIDTH-1:0],

     // Interface with VecCache
     output VecDataReadOp_t cache_read_op,
     output logic [CACHE_ADDR_SIZE-1:0] cache_read_addr,
     output logic [WIDTH_ADDR_SIZE-1:0] cache_read_param,
     output VecDataWriteOp_t cache_write_op,
     output logic [CACHE_ADDR_SIZE-1:0] cache_write_addr,
     output logic [WIDTH_ADDR_SIZE-1:0] cache_write_param,
     output shortreal cache_data_in[WIDTH-1:0],
     input shortreal cache_data_out[WIDTH-1:0],

     // Interface with memory
     output logic [INST_MEM_ADDR_SIZE-1:0] inst_mem_read_addr,
     input logic [INST_MEM_WIDTH_SIZE-1:0] inst_mem_data_out,
     output logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_read_addr,
     input shortreal data_mem_data_out[WIDTH-1:0],
     output VecDataMemWriteOp_t data_mem_write_op,
     output logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_write_addr,
     output shortreal data_mem_data_in[WIDTH-1:0]
    );

    // State machine
    enum {
        INIT, READY, LOAD1, LOAD2, NEXT, STOP
    } state, next_state;

    // Proceed to next_inst
    logic next_inst_proceed;
    // Offset to next instruction (size of current instruction)
    logic [INST_MEM_ADDR_SIZE-1:0] next_inst_offset;

    // Opcode/operand register
    logic [OPCODE_TYPE_SIZE-1:0] opcode;
    logic [MEM_ADDR_TYPE_SIZE-1:0] op_addr;
    logic [CORE_IDX_TYPE_SIZE-1:0] op_core_idx;
    logic [REG_ADDR_TYPE_SIZE-1:0] op_Vd, op_V1, op_V2;
    logic [WIDTH_IDX_TYPE_SIZE-1:0] op_vec_idx;

    // Instruction decoder
    VecInstDecoder #(.OPCODE_TYPE_BYTES(OPCODE_TYPE_BYTES),
        .MEM_ADDR_TYPE_BYTES(MEM_ADDR_TYPE_BYTES),
        .CORE_IDX_TYPE_BYTES(CORE_IDX_TYPE_BYTES),
        .REG_ADDR_TYPE_BYTES(REG_ADDR_TYPE_BYTES),
        .WIDTH_IDX_TYPE_BYTES(WIDTH_IDX_TYPE_BYTES),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_SIZE(INST_MEM_WIDTH_SIZE)
    ) decoder(.inst_value(inst_mem_data_out), .inst_size(next_inst_offset),
        .opcode, .op_addr, .op_core_idx,
        .op_Vd, .op_V1, .op_V2,
        .op_vec_idx
    );

    // State machine
    always_ff @(posedge clock) begin
        if (reset)
            state <= INIT;
        else
            state <= next_state;
    end

    // Remember program counter
    always_ff @(posedge clock) begin
        if (reset)
            inst_mem_read_addr <= 0;
        else if (next_inst_proceed)
            inst_mem_read_addr <= inst_mem_read_addr + next_inst_offset;
    end

    // Multiplex input into cache_data_in
    enum {
        CACHE_DATA_FROM_ZERO,
        CACHE_DATA_FROM_DATA_MEM_DATA_OUT,
        CACHE_DATA_FROM_UNIT_DATA_OUT
    } cache_data_in_sel;

    genvar i;
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (cache_data_in_sel)
                    CACHE_DATA_FROM_ZERO: begin
                        cache_data_in[i] = 0;
                    end
                    CACHE_DATA_FROM_DATA_MEM_DATA_OUT: begin
                        cache_data_in[i] = data_mem_data_out[i];
                    end
                    CACHE_DATA_FROM_UNIT_DATA_OUT: begin
                        cache_data_in[i] = unit_data_out[i];
                    end
                endcase
            end
        end
    endgenerate

    // Multiplex input into data_mem_data_in
    enum {
        DATA_MEM_DATA_FROM_ZERO,
        DATA_MEM_DATA_FROM_CACHE_DATA_OUT
    } data_mem_data_in_sel;

    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (data_mem_data_in_sel)
                    DATA_MEM_DATA_FROM_ZERO: begin
                        data_mem_data_in[i] = 0;
                    end
                    DATA_MEM_DATA_FROM_CACHE_DATA_OUT: begin
                        data_mem_data_in[i] = cache_data_out[i];
                    end
                endcase
            end
        end
    endgenerate

    // Multiplex input into unit_data_in1
    enum {
        UNIT_DATA1_FROM_ZERO,
        UNIT_DATA1_FROM_CACHE_DATA_OUT
    } unit_data_in1_sel;

    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (unit_data_in1_sel)
                    UNIT_DATA1_FROM_ZERO: begin
                        unit_data_in1[i] = 0;
                    end
                    UNIT_DATA1_FROM_CACHE_DATA_OUT: begin
                        unit_data_in1[i] = cache_data_out[i];
                    end
                endcase
            end
        end
    endgenerate

    // Multiplex input into unit_data_in2
    enum {
        UNIT_DATA2_FROM_ZERO,
        UNIT_DATA2_FROM_CACHE_DATA_OUT
    } unit_data_in2_sel;

    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (unit_data_in2_sel)
                    UNIT_DATA2_FROM_ZERO: begin
                        unit_data_in2[i] = 0;
                    end
                    UNIT_DATA2_FROM_CACHE_DATA_OUT: begin
                        unit_data_in2[i] = cache_data_out[i];
                    end
                endcase
            end
        end
    endgenerate

    // Multiplex input into unit_data_inK
    enum {
        UNIT_DATAK_FROM_ZERO,
        UNIT_DATAK_FROM_CACHE_DATA_OUT
    } unit_data_inK_sel;

    always_comb begin
        unique case (unit_data_inK_sel)
            UNIT_DATAK_FROM_ZERO: begin
                unit_data_inK = 0;
            end
            UNIT_DATAK_FROM_CACHE_DATA_OUT: begin
                unit_data_inK = cache_data_out[0];
            end
        endcase
    end


    // Assign next state and output
    always_comb begin
        // Set default values
        next_inst_proceed = 0;
        done = 0;
        // DataMem
        data_mem_read_addr = 0;
        data_mem_write_op = VEC_DATA_MEM_WRITE_DISABLE;
        data_mem_write_addr = 0;
        data_mem_data_in_sel = DATA_MEM_DATA_FROM_ZERO;
        // Cache
        cache_read_op = VEC_DATA_READ_DISABLE;
        cache_read_addr = 0;
        cache_read_param = 0;
        cache_write_op = VEC_DATA_WRITE_DISABLE;
        cache_write_addr = 0;
        cache_write_param = 0;
        cache_data_in_sel = CACHE_DATA_FROM_ZERO;
        // Unit
        unit_data_in1_sel = UNIT_DATA1_FROM_ZERO;
        unit_data_in2_sel = UNIT_DATA2_FROM_ZERO;
        unit_data_inK_sel = UNIT_DATAK_FROM_ZERO;

        unique case (state)
            INIT: begin
                next_state = READY;
            end
            READY: begin
                next_state = NEXT;

// Case on opcode
case (opcode)
    // Section 1
    // TODO

    // Section 2
    LOAD_VEC: begin
        // Read from DataMem
        data_mem_read_addr = op_addr;

        // Write into cache
        cache_write_op = VEC_DATA_WRITE_VEC;
        cache_write_addr = op_V1;
        cache_data_in_sel = CACHE_DATA_FROM_DATA_MEM_DATA_OUT;
    end
    LOAD_SCALAR: begin
        // Read from DataMem
        data_mem_read_addr = op_addr;

        // Write into cache
        cache_write_op = VEC_DATA_WRITE_SCALAR;
        cache_write_addr = op_V1;
        cache_write_param = op_vec_idx;
        cache_data_in_sel = CACHE_DATA_FROM_DATA_MEM_DATA_OUT;
    end
    STORE_VEC: begin
        // Read from cache
        cache_read_op = VEC_DATA_READ_VEC;
        cache_read_addr = op_V1;

        // Write into DataMem
        data_mem_write_op = VEC_DATA_MEM_WRITE_ALL;
        data_mem_write_addr = op_addr;
    end
    STORE_SCALAR: begin
        // Read from cache
        cache_read_op = VEC_DATA_READ_SCALAR;
        cache_read_addr = op_V1;
        cache_read_param = op_vec_idx;

        // Write into DataMem
        data_mem_write_op = VEC_DATA_MEM_WRITE_SINGLE;
        data_mem_write_addr = op_addr;
    end

    // Section 3
    // TODO

    // Section 4
    HALT: begin
        next_state = STOP;
    end
endcase

            end
            NEXT: begin
                next_inst_proceed = 1;
                next_state = READY;
            end
            STOP: begin
                next_state = STOP;
                done = 1;
            end
        endcase
    end

endmodule: VecControl
