`default_nettype none

// Main control unit for MatCore
module MatControl
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

     // Interface with MatUnit
     output logic unit_set_weight,
     output logic [WIDTH_ADDR_SIZE-1:0] unit_set_weight_row,
     output shortreal unit_data_in[WIDTH-1:0],
     input shortreal unit_data_out[WIDTH-1:0],

     // Interface with MatVecUnit
     output shortreal vec_unit_data_in[WIDTH-1:0],
     input shortreal vec_unit_data_out[WIDTH-1:0],
     output MatVecUnitOp_t vec_unit_op,

     // Interface with MatCache
     output MatDataReadOp_t cache_read_op,
     output logic [CACHE_ADDR_SIZE-1:0] cache_read_addr1, cache_read_addr2,
     output logic [WIDTH_ADDR_SIZE-1:0] cache_read_param1, cache_read_param2,
     output MatDataWriteOp_t cache_write_op,
     output logic [CACHE_ADDR_SIZE-1:0] cache_write_addr1, cache_write_addr2,
     output logic [WIDTH_ADDR_SIZE-1:0] cache_write_param1, cache_write_param2,
     output shortreal cache_data_in[WIDTH-1:0],
     input shortreal cache_data_out[WIDTH-1:0],

     // Interface with memory
     output logic [INST_MEM_ADDR_SIZE-1:0] inst_mem_read_addr, inst_mem_read_addr2,
     input logic [INST_MEM_WIDTH_SIZE-1:0] inst_mem_data_out, inst_mem_data_out2,
     output logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_read_addr,
     input shortreal data_mem_data_out[WIDTH-1:0],
     output MatDataMemWriteOp_t data_mem_write_op,
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
        P0XX, P01X, P012,
        PX0X, PX01,
        PXX0,
        ACCESS_MEM,
        WAIT_SWITCH,
        READREG
    } state, next_state;

    // Proceed to next_inst
    logic next_inst_proceed;
    // Offset to next instruction (size of current instruction)
    logic [INST_MEM_ADDR_SIZE-1:0] next_inst_offset;

    // Opcode/operand register
    logic [OPCODE_TYPE_SIZE-1:0] opcode;
    logic [MEM_ADDR_TYPE_SIZE-1:0] op_addr;
    logic [CORE_IDX_TYPE_SIZE-1:0] op_core_idx;
    logic [REG_ADDR_TYPE_SIZE-1:0] op_Md, op_M1, op_M2;
    logic [WIDTH_IDX_TYPE_SIZE-1:0] op_row_idx, op_col_idx, op_diag_idx, op_row_idx_1, op_row_idx_2;

    // View next instruction (to pipeline)
    logic [OPCODE_TYPE_SIZE-1:0] next_opcode;
    logic [REG_ADDR_TYPE_SIZE-1:0] next_op_Md, next_op_M1, next_op_M2;

    // Diagnoal progress counter
    logic [WIDTH_ADDR_SIZE-1:0] diag_progress_counter;
    // Increase/Clear diagnoal progress counter
    logic diag_progress_counter_inc, diag_progress_counter_clr;

    // Instruction decoder
    MatInstDecoder #(.OPCODE_TYPE_BYTES(OPCODE_TYPE_BYTES),
        .MEM_ADDR_TYPE_BYTES(MEM_ADDR_TYPE_BYTES),
        .CORE_IDX_TYPE_BYTES(CORE_IDX_TYPE_BYTES),
        .REG_ADDR_TYPE_BYTES(REG_ADDR_TYPE_BYTES),
        .WIDTH_IDX_TYPE_BYTES(WIDTH_IDX_TYPE_BYTES),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_SIZE(INST_MEM_WIDTH_SIZE)
    ) decoder(.inst_value(inst_mem_data_out), .inst_size(next_inst_offset),
        .opcode, .op_addr, .op_core_idx,
        .op_Md, .op_M1, .op_M2,
        .op_row_idx, .op_col_idx, .op_diag_idx,
        .op_row_idx_1, .op_row_idx_2
    );
    // Decode next instruction
    MatInstDecoder #(.OPCODE_TYPE_BYTES(OPCODE_TYPE_BYTES),
        .MEM_ADDR_TYPE_BYTES(MEM_ADDR_TYPE_BYTES),
        .CORE_IDX_TYPE_BYTES(CORE_IDX_TYPE_BYTES),
        .REG_ADDR_TYPE_BYTES(REG_ADDR_TYPE_BYTES),
        .WIDTH_IDX_TYPE_BYTES(WIDTH_IDX_TYPE_BYTES),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_SIZE(INST_MEM_WIDTH_SIZE)
    ) next_decoder(.inst_value(inst_mem_data_out2),
        .inst_size(),
        .opcode(next_opcode),
        .op_addr(), .op_core_idx(),
        .op_Md(next_op_Md), .op_M1(next_op_M1), .op_M2(next_op_M2),
        .op_row_idx(), .op_col_idx(), .op_diag_idx(),
        .op_row_idx_1(), .op_row_idx_2()
    );

    // Next opcode is MatUnit op
    logic next_opcode_is_unit;
    assign next_opcode_is_unit = next_opcode == MAT_INST_SET_WEIGHT || next_opcode == MAT_INST_MULTIPLY;


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
    // Next instruction address
    assign inst_mem_read_addr2 = inst_mem_read_addr + next_inst_offset;

    // Multiplex input into cache_data_in
    enum {
        CACHE_DATA_FROM_ZERO,
        CACHE_DATA_FROM_DATA_MEM_DATA_OUT,
        CACHE_DATA_FROM_UNIT_DATA_OUT,
        CACHE_DATA_FROM_VEC_UNIT_DATA_OUT,
        CACHE_DATA_FROM_SWITCH_RECV_DATA,
        CACHE_DATA_FROM_CACHE_DATA_OUT
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
                    CACHE_DATA_FROM_VEC_UNIT_DATA_OUT: begin
                        cache_data_in[i] = vec_unit_data_out[i];
                    end
                    CACHE_DATA_FROM_SWITCH_RECV_DATA: begin
                        cache_data_in[i] = switch_recv_data[i];
                    end
                    CACHE_DATA_FROM_CACHE_DATA_OUT: begin
                        cache_data_in[i] = cache_data_out[i];
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

    // Multiplex input into unit_data_in
    enum {
        UNIT_DATA_FROM_ZERO,
        UNIT_DATA_FROM_CACHE_DATA_OUT
    } unit_data_in_sel;

    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (unit_data_in_sel)
                    UNIT_DATA_FROM_ZERO: begin
                        unit_data_in[i] = 0;
                    end
                    UNIT_DATA_FROM_CACHE_DATA_OUT: begin
                        unit_data_in[i] = cache_data_out[i];
                    end
                endcase
            end
        end
    endgenerate

    // Connect vec_unit_data_in from cache data out
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                vec_unit_data_in[i] = cache_data_out[i];
            end
        end
    endgenerate

    // Connect switch send data to from cache data out
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                switch_send_data[i] = cache_data_out[i];
            end
        end
    endgenerate

    // Unit diagonal progress counter
    always_ff @(posedge clock) begin
        if (diag_progress_counter_clr)
            diag_progress_counter <= 0;
        else if (diag_progress_counter_inc)
            diag_progress_counter <= diag_progress_counter + 1;
    end

    // L0, L1, L2 shift registers to pipeline
    logic [OPCODE_TYPE_SIZE-1:0] l0_opcode, l1_opcode, l2_opcode;
    logic [REG_ADDR_TYPE_SIZE-1:0] l0_Md, l0_M1, l1_Md, l1_M1, l2_Md, l2_M1;
    enum {
        UNIT_SHIFT_REG_DISABLE,
        UNIT_SHIFT_REG_FLIP_CUR,
        UNIT_SHIFT_REG_FLIP_NEXT
    } unit_shift_reg_flip_sel;
    always_ff @(posedge clock) begin
        unique case (unit_shift_reg_flip_sel)
            UNIT_SHIFT_REG_DISABLE: begin
                // Do nothing
            end
            UNIT_SHIFT_REG_FLIP_CUR: begin
                {l0_opcode, l0_Md, l0_M1} <= {opcode, op_Md, op_M1};
                {l1_opcode, l1_Md, l1_M1} <= {l0_opcode, l0_Md, l0_M1};
                {l2_opcode, l2_Md, l2_M1} <= {l1_opcode, l1_Md, l1_M1};
            end
            UNIT_SHIFT_REG_FLIP_NEXT: begin
                {l0_opcode, l0_Md, l0_M1} <= {next_opcode, next_op_Md, next_op_M1};
                {l1_opcode, l1_Md, l1_M1} <= {l0_opcode, l0_Md, l0_M1};
                {l2_opcode, l2_Md, l2_M1} <= {l1_opcode, l1_Md, l1_M1};
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
        data_mem_write_op = MAT_DATA_MEM_WRITE_DISABLE;
        data_mem_write_addr = 0;
        data_mem_data_in_sel = DATA_MEM_DATA_FROM_ZERO;
        // Cache
        cache_read_op = MAT_DATA_READ_DISABLE;
        cache_read_addr1 = 0;
        cache_read_addr2 = 0;
        cache_read_param1 = 0;
        cache_read_param2 = 0;
        cache_write_op = MAT_DATA_WRITE_DISABLE;
        cache_write_addr1 = 0;
        cache_write_addr2 = 0;
        cache_write_param1 = 0;
        cache_write_param2 = 0;
        cache_data_in_sel = CACHE_DATA_FROM_ZERO;
        // Unit
        unit_set_weight = 0;
        unit_set_weight_row = 0;
        unit_data_in_sel = UNIT_DATA_FROM_ZERO;
        // VecUnit
        vec_unit_op = MAT_VEC_UNIT_OP_ZERO;
        // Switch
        switch_send_ready = 0;
        switch_send_core_idx = 0;
        switch_recv_request = 0;
        switch_recv_core_idx = 0;

        // Diagonal progress counter
        diag_progress_counter_inc = 0;
        diag_progress_counter_clr = 0;

        // Unit shift register
        unit_shift_reg_flip_sel = UNIT_SHIFT_REG_DISABLE;

        unique case (state)
            INIT: begin
                next_state = READY;
            end
            READY: begin
                next_state = NEXT;

// Case on opcode
case (opcode)
    // Section 1
    MAT_INST_SET_WEIGHT,
    MAT_INST_MULTIPLY: begin
        // Change next state
        next_state = P0XX;

        // Init counter
        diag_progress_counter_clr = 1;

        // Insert into L0
        unit_shift_reg_flip_sel = UNIT_SHIFT_REG_FLIP_CUR;
    end
    MAT_INST_TRANSPOSE: begin
        // Transpose cache matrix
        cache_write_op = MAT_DATA_WRITE_TRANSPOSE;
        cache_write_addr1 = op_M1;
    end
    MAT_INST_XFLIP: begin
        // XFLIP cache matrix
        cache_write_op = MAT_DATA_WRITE_XFLIP;
        cache_write_addr1 = op_M1;
    end
    MAT_INST_YFLIP: begin
        // XFLIP cache matrix
        cache_write_op = MAT_DATA_WRITE_YFLIP;
        cache_write_addr1 = op_M1;
    end
    MAT_INST_CLEAR: begin
        // Zero cache matrix
        cache_write_op = MAT_DATA_WRITE_ZERO;
        cache_write_addr1 = op_M1;
    end

    MAT_INST_COPY: begin
        // Change next state
        next_state = ACCESS_MEM; // Not really "access-mem"

        // Init counter ("row progress counter")
        diag_progress_counter_clr = 1;
    end

    MAT_INST_ADD_ROW: begin
        // Change next state
        next_state = READREG;

        // Read from cache
        cache_read_op = MAT_DATA_READ_ROW;
        cache_read_addr1 = op_M1;
        cache_read_param1 = op_row_idx_1;

        // Write into MatVecUnit
        vec_unit_op = MAT_VEC_UNIT_OP_LOAD;
    end

    // Section 2
    MAT_INST_LOAD_MAT: begin
        // Change next state
        next_state = ACCESS_MEM;

        // Init counter ("row progress counter")
        diag_progress_counter_clr = 1;
    end
    MAT_INST_LOAD_ROW: begin
        // Read from DataMem
        data_mem_read_addr = op_addr;

        // Write into cache
        cache_write_op = MAT_DATA_WRITE_ROW;
        cache_write_addr1 = op_M1;
        cache_write_param1 = op_row_idx;
        cache_data_in_sel = CACHE_DATA_FROM_DATA_MEM_DATA_OUT;
    end
    MAT_INST_LOAD_COL: begin
        // Read from DataMem
        data_mem_read_addr = op_addr;

        // Write into cache
        cache_write_op = MAT_DATA_WRITE_COL;
        cache_write_addr1 = op_M1;
        cache_write_param1 = op_col_idx;
        cache_data_in_sel = CACHE_DATA_FROM_DATA_MEM_DATA_OUT;
    end
    MAT_INST_LOAD_SCALAR: begin
        // Read from DataMem
        data_mem_read_addr = op_addr; // data_mem_data_out[0] now contains scalar

        // Write into cache
        cache_write_op = MAT_DATA_WRITE_SCALAR;
        cache_write_addr1 = op_M1;
        cache_write_param1 = op_row_idx;
        cache_write_param2 = op_col_idx;
        cache_data_in_sel = CACHE_DATA_FROM_DATA_MEM_DATA_OUT;
    end
    MAT_INST_STORE_MAT: begin
        // Change next state
        next_state = ACCESS_MEM;

        // Init counter
        diag_progress_counter_clr = 1;
    end
    MAT_INST_STORE_ROW: begin
        // Read from cache
        cache_read_op = MAT_DATA_READ_ROW;
        cache_read_addr1 = op_M1;
        cache_read_param1 = op_row_idx;

        // Write into DataMem
        data_mem_write_op = MAT_DATA_MEM_WRITE_ALL;
        data_mem_write_addr = op_addr;
        data_mem_data_in_sel = DATA_MEM_DATA_FROM_CACHE_DATA_OUT;
    end
    MAT_INST_STORE_COL: begin
        // Read from cache
        cache_read_op = MAT_DATA_READ_COL;
        cache_read_addr1 = op_M1;
        cache_read_param1 = op_col_idx;

        // Write into DataMem
        data_mem_write_op = MAT_DATA_MEM_WRITE_ALL;
        data_mem_write_addr = op_addr;
        data_mem_data_in_sel = DATA_MEM_DATA_FROM_CACHE_DATA_OUT;
    end
    MAT_INST_STORE_SCALAR: begin
        // Read from cache
        cache_read_op = MAT_DATA_READ_SCALAR;
        cache_read_addr1 = op_M1;
        cache_read_param1 = op_row_idx;
        cache_read_param2 = op_col_idx;

        // Write into DataMem
        data_mem_write_op = MAT_DATA_MEM_WRITE_SINGLE;
        data_mem_write_addr = op_addr;
        data_mem_data_in_sel = DATA_MEM_DATA_FROM_CACHE_DATA_OUT;
    end

    // Section 3
    MAT_INST_SEND_ROW,
    MAT_INST_SEND_COL,
    MAT_INST_SEND_SCALAR,
    MAT_INST_SEND_DIAG: begin
        // Change next state
        next_state = WAIT_SWITCH;

        // Read from cache
        unique case (opcode)
        MAT_INST_SEND_ROW: begin
            cache_read_op = MAT_DATA_READ_ROW;
            cache_read_addr1 = op_M1;
            cache_read_param1 = op_row_idx;
        end
        MAT_INST_SEND_COL: begin
            cache_read_op = MAT_DATA_READ_COL;
            cache_read_addr1 = op_M1;
            cache_read_param1 = op_col_idx;
        end
        MAT_INST_SEND_SCALAR: begin
            cache_read_op = MAT_DATA_READ_SCALAR;
            cache_read_addr1 = op_M1;
            cache_read_param1 = op_row_idx;
            cache_read_param2 = op_col_idx;
        end
        MAT_INST_SEND_DIAG: begin
            cache_read_op = MAT_DATA_READ_DIAG;
            cache_read_addr1 = op_M1;
            cache_read_addr2 = op_M2;
            cache_read_param1 = op_diag_idx;
        end
        endcase

        // Send vector
        switch_send_ready = 1;
        switch_send_core_idx = op_core_idx;

        // Test send ok
        if (switch_send_ok) begin
            next_state = NEXT;
        end
    end
    MAT_INST_RECV_ROW,
    MAT_INST_RECV_COL,
    MAT_INST_RECV_SCALAR,
    MAT_INST_RECV_DIAG,
    MAT_INST_RECV_DIAG1,
    MAT_INST_RECV_DIAG2: begin
        // Change next state
        next_state = WAIT_SWITCH;

        // Switch recv
        switch_recv_request = 1;
        switch_recv_core_idx = op_core_idx;

        // Test recv ready
        if (switch_recv_ready) begin
            next_state = NEXT;
            // Write to cache
            unique case (opcode)
            MAT_INST_RECV_ROW: begin
                cache_write_op = MAT_DATA_WRITE_ROW;
                cache_write_addr1 = op_M1;
                cache_write_param1 = op_row_idx;
            end
            MAT_INST_RECV_COL: begin
                cache_write_op = MAT_DATA_WRITE_COL;
                cache_write_addr1 = op_M1;
                cache_write_param1 = op_col_idx;
            end
            MAT_INST_RECV_SCALAR: begin
                cache_write_op = MAT_DATA_WRITE_SCALAR;
                cache_write_addr1 = op_M1;
                cache_write_param1 = op_row_idx;
                cache_write_param2 = op_col_idx;
            end
            MAT_INST_RECV_DIAG: begin
                cache_write_op = MAT_DATA_WRITE_DIAG;
                cache_write_addr1 = op_M1;
                cache_write_addr2 = op_M2;
                cache_write_param1 = op_diag_idx;
            end
            MAT_INST_RECV_DIAG1: begin
                cache_write_op = MAT_DATA_WRITE_DIAG1;
                cache_write_addr1 = op_M1;
                cache_write_param1 = op_diag_idx;
            end
            MAT_INST_RECV_DIAG2: begin
                cache_write_op = MAT_DATA_WRITE_DIAG2;
                cache_write_addr1 = op_M1;
                cache_write_param1 = op_diag_idx;
            end
            endcase
            cache_data_in_sel = CACHE_DATA_FROM_SWITCH_RECV_DATA;
        end
    end

    // Section 4
    MAT_INST_HALT: begin
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
            WAIT_SWITCH: begin
                unique case (opcode)
                MAT_INST_SEND_ROW,
                MAT_INST_SEND_COL,
                MAT_INST_SEND_SCALAR,
                MAT_INST_SEND_DIAG: begin
                    if (switch_send_ok) begin
                        next_state = NEXT;
                    end else begin
                        next_state = WAIT_SWITCH;
                    end
                end
                MAT_INST_RECV_ROW,
                MAT_INST_RECV_COL,
                MAT_INST_RECV_SCALAR,
                MAT_INST_RECV_DIAG,
                MAT_INST_RECV_DIAG1,
                MAT_INST_RECV_DIAG2: begin
                    if (switch_recv_ready) begin
                        next_state = NEXT;
                        // Write into cache
                        unique case (opcode)
                        MAT_INST_RECV_ROW: begin
                            cache_write_op = MAT_DATA_WRITE_ROW;
                            cache_write_addr1 = op_M1;
                            cache_write_param1 = op_row_idx;
                        end
                        MAT_INST_RECV_COL: begin
                            cache_write_op = MAT_DATA_WRITE_COL;
                            cache_write_addr1 = op_M1;
                            cache_write_param1 = op_col_idx;
                        end
                        MAT_INST_RECV_SCALAR: begin
                            cache_write_op = MAT_DATA_WRITE_SCALAR;
                            cache_write_addr1 = op_M1;
                            cache_write_param1 = op_row_idx;
                            cache_write_param2 = op_col_idx;
                        end
                        MAT_INST_RECV_DIAG: begin
                            cache_write_op = MAT_DATA_WRITE_DIAG;
                            cache_write_addr1 = op_M1;
                            cache_write_addr2 = op_M2;
                            cache_write_param1 = op_diag_idx;
                        end
                        MAT_INST_RECV_DIAG1: begin
                            cache_write_op = MAT_DATA_WRITE_DIAG1;
                            cache_write_addr1 = op_M1;
                            cache_write_param1 = op_diag_idx;
                        end
                        MAT_INST_RECV_DIAG2: begin
                            cache_write_op = MAT_DATA_WRITE_DIAG2;
                            cache_write_addr1 = op_M1;
                            cache_write_param1 = op_diag_idx;
                        end
                        endcase
                        cache_data_in_sel = CACHE_DATA_FROM_SWITCH_RECV_DATA;
                    end else begin
                        next_state = WAIT_SWITCH;
                    end
                end
                endcase
            end
            READREG: begin
                unique case (opcode)
                    MAT_INST_ADD_ROW: begin
                        // Change next state
                        next_state = NEXT;

                        // Read from cache
                        cache_read_op = MAT_DATA_READ_ROW;
                        cache_read_addr1 = op_M2;
                        cache_read_param1 = op_row_idx_2;

                        // Vector addition
                        vec_unit_op = MAT_VEC_UNIT_OP_ADD;

                        // Write into cache
                        cache_write_op = MAT_DATA_WRITE_ROW;
                        cache_write_addr1 = op_Md;
                        cache_write_param1 = op_row_idx;
                        cache_data_in_sel = CACHE_DATA_FROM_VEC_UNIT_DATA_OUT;
                    end
                endcase
            end
            ACCESS_MEM: begin
                if (diag_progress_counter == WIDTH - 1) begin
                    next_state = NEXT;
                end else begin
                    next_state = ACCESS_MEM;
                    diag_progress_counter_inc = 1;
                end
                unique case (opcode)
                    MAT_INST_LOAD_MAT: begin
                        // Read from DataMem
                        data_mem_read_addr = op_addr +
                            WIDTH * diag_progress_counter;
                        // Write into cache
                        cache_write_op = MAT_DATA_WRITE_ROW;
                        cache_write_addr1 = op_M1;
                        cache_write_param1 = diag_progress_counter;
                        cache_data_in_sel = CACHE_DATA_FROM_DATA_MEM_DATA_OUT;
                    end
                    MAT_INST_STORE_MAT: begin
                        // Read from cache
                        cache_read_op = MAT_DATA_READ_ROW;
                        cache_read_addr1 = op_M1;
                        cache_read_param1 = diag_progress_counter;

                        // Write into DataMem
                        data_mem_write_op = MAT_DATA_MEM_WRITE_ALL;
                        data_mem_write_addr = op_addr +
                            WIDTH * diag_progress_counter;
                        data_mem_data_in_sel = DATA_MEM_DATA_FROM_CACHE_DATA_OUT;
                    end
                    MAT_INST_COPY: begin
                        // Read from cache
                        cache_read_op = MAT_DATA_READ_ROW;
                        cache_read_addr1 = op_M1;
                        cache_read_param1 = diag_progress_counter;
                        // Write into cache
                        cache_write_op = MAT_DATA_WRITE_ROW;
                        cache_write_addr1 = op_Md;
                        cache_write_param1 = diag_progress_counter;
                        cache_data_in_sel = CACHE_DATA_FROM_CACHE_DATA_OUT;
                    end
                endcase
            end

            P0XX: begin
                // Next state
                if (diag_progress_counter == WIDTH - 1) begin
                    if (next_opcode_is_unit) begin
                        next_state = P01X;
                        unit_shift_reg_flip_sel = UNIT_SHIFT_REG_FLIP_NEXT;
                        next_inst_proceed = 1;
                    end else begin
                        next_state = PX0X;
                    end
                    diag_progress_counter_clr = 1;
                end else begin
                    next_state = P0XX;
                    diag_progress_counter_inc = 1;
                end
                // Read from cache
                unit_data_in_sel = UNIT_DATA_FROM_CACHE_DATA_OUT;
                cache_read_op = MAT_DATA_READ_DIAG;
                cache_read_addr1 = l0_M1;
                cache_read_param1 = diag_progress_counter;

                // Write weight at the end
                if (diag_progress_counter == WIDTH - 1 &&
                        l0_opcode == MAT_INST_SET_WEIGHT) begin
                    unit_set_weight = 1;
                    unit_set_weight_row = 0;
                end
            end
            P01X: begin
                // Next state
                if (diag_progress_counter == WIDTH - 1) begin
                    if (next_opcode_is_unit) begin
                        next_state = P012;
                        unit_shift_reg_flip_sel = UNIT_SHIFT_REG_FLIP_NEXT;
                        next_inst_proceed = 1;
                    end else begin
                        next_state = PX01;
                    end
                    diag_progress_counter_clr = 1;
                end else begin
                    next_state = P01X;
                    diag_progress_counter_inc = 1;
                end
                // Read from cache
                unit_data_in_sel = UNIT_DATA_FROM_CACHE_DATA_OUT;
                cache_read_op = MAT_DATA_READ_DIAG;
                cache_read_addr1 = l0_M1;
                cache_read_addr2 = l1_M1;
                cache_read_param1 = diag_progress_counter;

                // Write weight at the end
                if (diag_progress_counter == WIDTH - 1 &&
                        l0_opcode == MAT_INST_SET_WEIGHT) begin
                    unit_set_weight = 1;
                    unit_set_weight_row = 0;
                end
                // Write weight before the end
                if (diag_progress_counter != WIDTH - 1 &&
                        l1_opcode == MAT_INST_SET_WEIGHT) begin
                    unit_set_weight = 1;
                    unit_set_weight_row = diag_progress_counter + 1;
                end
                // Write into cache
                if (l1_opcode == MAT_INST_MULTIPLY) begin
                    cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                    cache_write_op = MAT_DATA_WRITE_DIAG1;
                    cache_write_addr1 = l1_Md;
                    cache_write_param1 = diag_progress_counter;
                end
            end
            PX0X: begin
                if (diag_progress_counter == WIDTH - 1) begin
                    // Cannot pipeline
                    next_state = PXX0;
                    diag_progress_counter_clr = 1;
                end else begin
                    next_state = PX0X;
                    diag_progress_counter_inc = 1;
                end

                // Read from cache
                unit_data_in_sel = UNIT_DATA_FROM_CACHE_DATA_OUT;
                cache_read_op = MAT_DATA_READ_DIAG;
                cache_read_addr2 = l0_M1;
                cache_read_param1 = diag_progress_counter;

                // Write weight before the end
                if (diag_progress_counter != WIDTH - 1 &&
                        l0_opcode == MAT_INST_SET_WEIGHT) begin
                    unit_set_weight = 1;
                    unit_set_weight_row = diag_progress_counter + 1;
                end
                // Write into cache
                if (l0_opcode == MAT_INST_MULTIPLY) begin
                    cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                    cache_write_op = MAT_DATA_WRITE_DIAG1;
                    cache_write_addr1 = l0_Md;
                    cache_write_param1 = diag_progress_counter;
                end
            end
            PXX0: begin
                // Stage 3
                if (diag_progress_counter == WIDTH - 1) begin
                    next_state = NEXT;
                end else begin
                    next_state = PXX0;
                    diag_progress_counter_inc = 1;
                end

                // Write into cache
                if (l0_opcode == MAT_INST_MULTIPLY) begin
                    cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                    cache_write_op = MAT_DATA_WRITE_DIAG2;
                    cache_write_addr1 = l0_Md;
                    cache_write_param1 = diag_progress_counter;
                end
            end
            PX01: begin
                // Next state
                if (diag_progress_counter == WIDTH - 1) begin
                    // Cannot pipeline
                    next_state = PXX0;
                    diag_progress_counter_clr = 1;
                end else begin
                    next_state = PX01;
                    diag_progress_counter_inc = 1;
                end
                // Read from cache
                unit_data_in_sel = UNIT_DATA_FROM_CACHE_DATA_OUT;
                cache_read_op = MAT_DATA_READ_DIAG;
                cache_read_addr2 = l0_M1;
                cache_read_param1 = diag_progress_counter;

                // Write weight before the end
                if (diag_progress_counter != WIDTH - 1 &&
                        l0_opcode == MAT_INST_SET_WEIGHT) begin
                    unit_set_weight = 1;
                    unit_set_weight_row = diag_progress_counter + 1;
                end
                if (l0_opcode == MAT_INST_MULTIPLY && l1_opcode == MAT_INST_MULTIPLY) begin
                    cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                    cache_write_op = MAT_DATA_WRITE_DIAG;
                    cache_write_addr1 = l0_Md;
                    cache_write_addr2 = l1_Md;
                    cache_write_param1 = diag_progress_counter;
                end else if (l0_opcode == MAT_INST_MULTIPLY) begin
                    cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                    cache_write_op = MAT_DATA_WRITE_DIAG1;
                    cache_write_addr1 = l0_Md;
                    cache_write_param1 = diag_progress_counter;
                end else if (l1_opcode == MAT_INST_MULTIPLY) begin
                    cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                    cache_write_op = MAT_DATA_WRITE_DIAG2;
                    cache_write_addr1 = l1_Md;
                    cache_write_param1 = diag_progress_counter;
                end
            end
            P012: begin
                // Next state
                if (diag_progress_counter == WIDTH - 1) begin
                    if (next_opcode_is_unit) begin
                        next_state = P012;
                        unit_shift_reg_flip_sel = UNIT_SHIFT_REG_FLIP_NEXT;
                        next_inst_proceed = 1;
                    end else begin
                        next_state = PX01;
                    end
                    diag_progress_counter_clr = 1;
                end else begin
                    next_state = P012;
                    diag_progress_counter_inc = 1;
                end

                // Read from cache
                unit_data_in_sel = UNIT_DATA_FROM_CACHE_DATA_OUT;
                cache_read_op = MAT_DATA_READ_DIAG;
                cache_read_addr1 = l0_M1;
                cache_read_addr2 = l1_M1;
                cache_read_param1 = diag_progress_counter;

                // Write weight before the end
                if (diag_progress_counter == WIDTH - 1 &&
                        l0_opcode == MAT_INST_SET_WEIGHT) begin
                    unit_set_weight = 1;
                    unit_set_weight_row = 0;
                end

                // Write weight before the end
                if (diag_progress_counter != WIDTH - 1 &&
                        l1_opcode == MAT_INST_SET_WEIGHT) begin
                    unit_set_weight = 1;
                    unit_set_weight_row = diag_progress_counter + 1;
                end
                // Write into cache
                if (l1_opcode == MAT_INST_MULTIPLY && l2_opcode == MAT_INST_MULTIPLY) begin
                    cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                    cache_write_op = MAT_DATA_WRITE_DIAG;
                    cache_write_addr1 = l1_Md;
                    cache_write_addr2 = l2_Md;
                    cache_write_param1 = diag_progress_counter;
                end else if (l1_opcode == MAT_INST_MULTIPLY) begin
                    cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                    cache_write_op = MAT_DATA_WRITE_DIAG1;
                    cache_write_addr1 = l1_Md;
                    cache_write_param1 = diag_progress_counter;
                end else if (l2_opcode == MAT_INST_MULTIPLY) begin
                    cache_data_in_sel = CACHE_DATA_FROM_UNIT_DATA_OUT;
                    cache_write_op = MAT_DATA_WRITE_DIAG2;
                    cache_write_addr1 = l2_Md;
                    cache_write_param1 = diag_progress_counter;
                end
            end
        endcase
    end

endmodule: MatControl
