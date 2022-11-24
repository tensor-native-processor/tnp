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

                SWITCH_WIDTH = 128,
                SWITCH_CORE_SIZE = 4,

                // Auto-generated sizes
                OPCODE_TYPE_SIZE = 8 * OPCODE_TYPE_BYTES,
                MEM_ADDR_TYPE_SIZE = 8 * MEM_ADDR_TYPE_BYTES,
                CORE_IDX_TYPE_SIZE = 8 * CORE_IDX_TYPE_BYTES,
                REG_ADDR_TYPE_SIZE = 8 * REG_ADDR_TYPE_BYTES,
                WIDTH_IDX_TYPE_SIZE = 8 * WIDTH_IDX_TYPE_BYTES,

                WIDTH_ADDR_SIZE = $clog2(WIDTH),
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE),

                SWITCH_CORE_ADDR_SIZE = $clog2(SWITCH_CORE_SIZE)
    )
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
     output shortreal data_mem_data_in[WIDTH-1:0],

     // Interface with switch
     output logic switch_send_ready,
     output logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_send_core_idx,
     output shortreal switch_send_data[SWITCH_WIDTH-1:0],
     input logic switch_send_ok,

     // Switch recv
     output logic switch_recv_request,
     output logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_recv_core_idx,
     input logic switch_recv_ready,
     input shortreal switch_recv_data[SWITCH_WIDTH-1:0]
    );

    // State machine
    enum {
        INIT, READY, NEXT, STOP,
        READREG,
        WAIT_SWITCH
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

    // Input to VecUnit
    // Input 1
    assign unit_data_in1 = cache_data_out;
    // Control points
    VecDataReadOp_t reg_unit2_read_op, reg_unitK_read_op;
    VecDataWriteOp_t reg_unit2_write_op, reg_unitK_write_op;
    logic [WIDTH_ADDR_SIZE-1:0] reg_unit2_read_param, reg_unit2_write_param,
                                reg_unitK_read_param, reg_unitK_write_param;
    // Input 2
    VecReg #(.WIDTH(WIDTH)) reg_unit2(.clock,
        .read_op(reg_unit2_read_op),
        .read_param(reg_unit2_read_param),
        .write_op(reg_unit2_write_op),
        .write_param(reg_unit2_write_param),
        .data_in(cache_data_out),
        .data_out(unit_data_in2)
    );
    // Input K
    assign unit_data_inK = unit_data_in2[0];

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
        CACHE_DATA_FROM_UNIT_DATA_OUT,
        CACHE_DATA_FROM_CACHE_DATA_OUT,
        CACHE_DATA_FROM_SWITCH_RECV_DATA
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
                    CACHE_DATA_FROM_CACHE_DATA_OUT: begin
                        cache_data_in[i] = cache_data_out[i];
                    end
                    CACHE_DATA_FROM_SWITCH_RECV_DATA: begin
                        cache_data_in[i] = switch_recv_data[i];
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

    // Connect switch send data from cache data out
    assign switch_send_data = cache_data_out;

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
        unit_op = VEC_UNIT_OP_ZERO;
        // Registers for unit
        reg_unit2_read_op = VEC_DATA_READ_DISABLE;
        reg_unitK_read_op = VEC_DATA_READ_DISABLE;
        reg_unit2_write_op = VEC_DATA_WRITE_DISABLE;
        reg_unitK_write_op = VEC_DATA_WRITE_DISABLE;
        reg_unit2_read_param = 0;
        reg_unitK_read_param = 0;
        reg_unit2_write_param = 0;
        reg_unitK_write_param = 0;
        // Switch
        switch_send_ready = 0;
        switch_send_core_idx = 0;
        switch_recv_request = 0;
        switch_recv_core_idx = 0;

        unique case (state)
            INIT: begin
                next_state = READY;
            end
            READY: begin
                next_state = NEXT;

// Case on opcode
case (opcode)
    // Section 1
    ADD,
    SUB,
    DOT: begin
        // Change next state
        next_state = READREG;
        // Read from cache
        cache_read_op = VEC_DATA_READ_VEC;
        cache_read_addr = op_V2;
        // Store into reg_unit2
        reg_unit2_write_op = VEC_DATA_WRITE_VEC;
    end
    DELTA,
    SCALE: begin
        // Change next state
        next_state = READREG;
        // Read from cache
        cache_read_op = VEC_DATA_READ_SCALAR;
        cache_read_addr = op_V2;
        cache_read_param = op_vec_idx;
        // Store into reg_unit2
        reg_unit2_write_op = VEC_DATA_WRITE_VEC;
    end

    ACT_SIGMOID,
    ACT_TANH,
    ACT_RELU: begin
        // Change next state
        next_state = NEXT;
        // Read from cache
        cache_read_op = VEC_DATA_READ_VEC;
        cache_read_addr = op_V1;
        // Set unit op
        unique case (opcode)
            ACT_SIGMOID: unit_op = VEC_UNIT_OP_ACT_SIGMOID;
            ACT_TANH: unit_op = VEC_UNIT_OP_ACT_TANH;
            ACT_RELU: unit_op = VEC_UNIT_OP_ACT_RELU;
        endcase
        // Write into cache
        cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
        cache_write_op = VEC_DATA_WRITE_VEC;
        cache_write_addr = op_Vd;
    end

    CLEAR: begin
        // Write into cache
        cache_write_op = VEC_DATA_WRITE_ZERO;
        cache_write_addr = op_V1;
    end
    COPY: begin
        // Read from cache
        cache_read_op = VEC_DATA_READ_VEC;
        cache_read_addr = op_V1;
        // Write into cache
        cache_data_in_sel = CACHE_DATA_FROM_CACHE_DATA_OUT;
        cache_write_op = VEC_DATA_WRITE_VEC;
        cache_write_addr = op_Vd;
    end

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
        data_mem_data_in_sel = DATA_MEM_DATA_FROM_CACHE_DATA_OUT;
    end
    STORE_SCALAR: begin
        // Read from cache
        cache_read_op = VEC_DATA_READ_SCALAR;
        cache_read_addr = op_V1;
        cache_read_param = op_vec_idx;

        // Write into DataMem
        data_mem_write_op = VEC_DATA_MEM_WRITE_SINGLE;
        data_mem_write_addr = op_addr;
        data_mem_data_in_sel = DATA_MEM_DATA_FROM_CACHE_DATA_OUT;
    end

    // Section 3
    // TODO
    SEND_VEC: begin
        // Change next state
        next_state = WAIT_SWITCH;

        // Read from cache
        cache_read_op = VEC_DATA_READ_VEC;
        cache_read_addr = op_V1;

        // Send vector
        switch_send_ready = 1;
        switch_send_core_idx = op_core_idx;

        // Test send ok
        if (switch_send_ok) begin
            next_state = NEXT;
        end
    end
    RECV_VEC: begin
        // Change next state
        next_state = WAIT_SWITCH;

        // Set recv
        switch_recv_request = 1;
        switch_recv_core_idx = op_core_idx;

        // Test recv ready
        if (switch_recv_ready) begin
            next_state = NEXT;
            // Write to cache
            cache_write_op = VEC_DATA_WRITE_VEC;
            cache_write_addr = op_V1;
            cache_data_in_sel = CACHE_DATA_FROM_SWITCH_RECV_DATA;
        end
    end

    // Section 4
    HALT: begin
        next_state = STOP;
    end
endcase

            end
            WAIT_SWITCH: begin
                unique case (opcode)
                SEND_VEC: begin
                    if (switch_send_ok) begin
                        next_state = NEXT;
                    end else begin
                        next_state = WAIT_SWITCH;
                    end
                end
                RECV_VEC: begin
                    if (switch_recv_ready) begin
                        next_state = NEXT;
                        // Write to cache
                        cache_write_op = VEC_DATA_WRITE_VEC;
                        cache_write_addr = op_V1;
                        cache_data_in_sel = CACHE_DATA_FROM_SWITCH_RECV_DATA;
                    end else begin
                        next_state = WAIT_SWITCH;
                    end
                end
                endcase
            end
            READREG: begin
                // Change next state
                next_state = NEXT;
                // Read from cache
                cache_read_op = VEC_DATA_READ_VEC;
                cache_read_addr = op_V1;
                // Read from reg_unit2
                reg_unit2_read_op = VEC_DATA_READ_VEC;
                // Set unit op
                unique case (opcode)
                    ADD: unit_op = VEC_UNIT_OP_ADD;
                    SUB: unit_op = VEC_UNIT_OP_SUB;
                    DOT: unit_op = VEC_UNIT_OP_DOT;
                    SCALE: unit_op = VEC_UNIT_OP_SCALE;
                    DELTA: unit_op = VEC_UNIT_OP_DELTA;
                endcase
                // Write into cache
                cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                cache_write_op = VEC_DATA_WRITE_VEC;
                cache_write_addr = op_Vd;
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
