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

     // Interface with MatUnit
     output logic unit_set_weight,
     output logic [WIDTH_ADDR_SIZE-1:0] unit_set_weight_row,
     output shortreal unit_data_in[WIDTH-1:0],
     input shortreal unit_data_out[WIDTH-1:0],

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
     output shortreal data_mem_data_in[WIDTH-1:0]
    );

    // State machine
    enum {
        INIT, READY, NEXT, STOP,
        P1, P2, P3
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
    logic [WIDTH_IDX_TYPE_SIZE-1:0] op_row_idx, op_col_idx, op_diag_idx;

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
        .op_row_idx, .op_col_idx, .op_diag_idx
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
        .op_row_idx(), .op_col_idx(), .op_diag_idx()
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
    // Next instruction address
    assign inst_mem_read_addr2 = inst_mem_read_addr + next_inst_offset;

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

    // Unit diagonal progress counter
    always_ff @(posedge clock) begin
        if (diag_progress_counter_clr)
            diag_progress_counter <= 0;
        else if (diag_progress_counter_inc)
            diag_progress_counter <= diag_progress_counter + 1;
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
        data_mem_data_in_sel = DATA_MEM_DATA_FROM_CACHE_DATA_OUT;
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

        // Diagonal progress counter
        diag_progress_counter_inc = 0;
        diag_progress_counter_clr = 0;

        case (state)
            INIT: begin
                next_state = READY;
            end
            READY: begin
                next_state = NEXT;

// Case on opcode
case (opcode)
    // Section 1
    SET_WEIGHT: begin
        // Change next state
        next_state = P1;

        // Init counter
        diag_progress_counter_clr = 1;
    end
    // TODO

    // Section 2
    LOAD_ROW: begin
        // Read from DataMem
        data_mem_read_addr = op_addr;

        // Write into cache
        cache_write_op = MAT_DATA_WRITE_ROW;
        cache_write_addr1 = op_M1;
        cache_write_param1 = op_row_idx;
        cache_data_in_sel = CACHE_DATA_FROM_DATA_MEM_DATA_OUT;
    end
    LOAD_COL: begin
        // Read from DataMem
        data_mem_read_addr = op_addr;

        // Write into cache
        cache_write_op = MAT_DATA_WRITE_COL;
        cache_write_addr1 = op_M1;
        cache_write_param1 = op_col_idx;
        cache_data_in_sel = CACHE_DATA_FROM_DATA_MEM_DATA_OUT;
    end
    LOAD_SCALAR: begin
        // Read from DataMem
        data_mem_read_addr = op_addr; // data_mem_data_out[0] now contains scalar

        // Write into cache
        cache_write_op = MAT_DATA_WRITE_SCALAR;
        cache_write_addr1 = op_M1;
        cache_write_param1 = op_row_idx;
        cache_write_param2 = op_col_idx;
        cache_data_in_sel = CACHE_DATA_FROM_DATA_MEM_DATA_OUT;
    end
    STORE_ROW: begin
        // Read from cache
        cache_read_op = MAT_DATA_READ_ROW;
        cache_read_addr1 = op_M1;
        cache_read_param1 = op_row_idx;

        // Write into DataMem
        data_mem_write_op = MAT_DATA_MEM_WRITE_ALL;
        data_mem_write_addr = op_addr;
    end
    STORE_COL: begin
        // Read from cache
        cache_read_op = MAT_DATA_READ_COL;
        cache_read_addr1 = op_M1;
        cache_read_param1 = op_col_idx;

        // Write into DataMem
        data_mem_write_op = MAT_DATA_MEM_WRITE_ALL;
        data_mem_write_addr = op_addr;
    end
    STORE_SCALAR: begin
        // Read from cache
        cache_read_op = MAT_DATA_READ_SCALAR;
        cache_read_addr1 = op_M1;
        cache_read_param1 = op_row_idx;
        cache_read_param2 = op_col_idx;

        // Write into DataMem
        data_mem_write_op = MAT_DATA_MEM_WRITE_SINGLE;
        data_mem_write_addr = op_addr;
    end

    // Section 3

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

            P1: begin
                // Stage 1
                if (diag_progress_counter == WIDTH - 1) begin
                    next_state = P2;
                    diag_progress_counter_clr = 1;
                end else begin
                    next_state = P1;
                    diag_progress_counter_inc = 1;
                end

                // Read from cache
                cache_read_op = MAT_DATA_READ_DIAG;
                cache_read_addr1 = op_M1;
                cache_read_addr2 = op_M1;
                cache_read_param1 = diag_progress_counter;

                // Set unit data in
                unit_data_in_sel = UNIT_DATA_FROM_CACHE_DATA_OUT;

                // Write weight at the end
                if (diag_progress_counter == WIDTH - 1) begin
                    unit_set_weight = 1;
                    unit_set_weight_row = 0;
                end
            end
            P2: begin
                // Stage 2
                if (diag_progress_counter == WIDTH - 1) begin
                    next_state = P3;
                    diag_progress_counter_clr = 1;
                end else begin
                    next_state = P2;
                    diag_progress_counter_inc = 1;
                end

                // Read from cache
                cache_read_op = MAT_DATA_READ_DIAG;
                cache_read_addr1 = op_M1;
                cache_read_addr2 = op_M1;
                cache_read_param1 = diag_progress_counter;

                // Set unit data in
                unit_data_in_sel = UNIT_DATA_FROM_CACHE_DATA_OUT;

                // Write weight before the end
                if (diag_progress_counter < WIDTH - 1) begin
                    unit_set_weight = 1;
                    unit_set_weight_row = diag_progress_counter + 1;
                end
            end
            P3: begin
                // Stage 3
                if (diag_progress_counter == WIDTH - 1) begin
                    next_state = NEXT;
                end else begin
                    next_state = P3;
                    diag_progress_counter_inc = 1;
                end
                diag_progress_counter_inc = 1;

                // Read from cache
                cache_read_op = MAT_DATA_READ_DIAG;
                cache_read_addr1 = op_M1;
                cache_read_addr2 = op_M1;
                cache_read_param1 = diag_progress_counter;

                // Set unit data in
                unit_data_in_sel = UNIT_DATA_FROM_CACHE_DATA_OUT;
            end
        endcase
    end

endmodule: MatControl
