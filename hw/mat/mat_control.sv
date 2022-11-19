`default_nettype none

typedef enum logic [7:0] {
	// Section 1:
	SET_WEIGHT      = 8'b00100000,
	MULTIPLY        = 8'b00100001,
	TRANSPOSE       = 8'b00100010,

	// Section 2
	LOAD_MAT        = 8'b01000000,
	LOAD_ROW        = 8'b01000001,
	LOAD_COL        = 8'b01000010,
	LOAD_SCALAR     = 8'b01000011,
	STORE_MAT       = 8'b01010000,
	STORE_ROW       = 8'b01010001,
	STORE_COL       = 8'b01010010,
	STORE_SCALAR    = 8'b01010011,

	// Section 3
	SEND_ROW        = 8'b01100000,
	SEND_COL        = 8'b01100001,
	SEND_DIAG       = 8'b01100010,
	RECV_ROW        = 8'b01110000,
	RECV_COL        = 8'b01110001,
	RECV_SCALAR     = 8'b01110010,
	RECV_DIAG       = 8'b01111000,
	RECV_DIAG1      = 8'b01111001,
	RECV_DIAG2      = 8'b01111010,

	// Section 4
	HALT            = 8'b10000000
} MatCoreOpcode;

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
     output logic [WIDTH_ADDR_SIZE-1:0] cache_read_param,
     output MatDataWriteOp_t cache_write_op,
     output logic [CACHE_ADDR_SIZE-1:0] cache_write_addr1, cache_write_addr2,
     output logic [WIDTH_ADDR_SIZE-1:0] cache_write_param1, cache_write_param2,
     output shortreal cache_data_in[WIDTH-1:0],
     input shortreal cache_data_out[WIDTH-1:0],

     // Interface with memory
     output logic [INST_MEM_ADDR_SIZE-1:0] inst_mem_read_addr,
     input logic [INST_MEM_WIDTH_SIZE-1:0] inst_mem_data_out,
     output logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_read_addr,
     input shortreal data_mem_data_out[WIDTH-1:0],
     output logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_write_addr,
     output logic [WIDTH_ADDR_SIZE-1:0] data_mem_write_size,
     output shortreal data_mem_data_in[WIDTH-1:0]
    );

    // State machine
    enum {
        INIT, READY, NEXT, STOP,
        TOP_HALF, BOTTOM_HALF, HYBRID
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
    logic [WIDTH_IDX_TYPE_SIZE-1:0] op_row_idx, op_col_idx, op_diag_idx, op_elem_idx;

    // Assign instruction to opcode/operands
    always_comb begin
        opcode = 0;
        op_addr = 0;
        op_core_idx = 0;
        {op_Md, op_M1, op_M2} = 0;
        {op_row_idx, op_col_idx, op_diag_idx, op_elem_idx} = 0;

        opcode = inst_mem_data_out[OPCODE_TYPE_SIZE-1:0];
        next_inst_offset = OPCODE_TYPE_BYTES;

        case (opcode)
            // Section 1
            SET_WEIGHT,
            TRANSPOSE: begin
                op_M1       = inst_mem_data_out[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end
            MULTIPLY: begin
                op_Md       = inst_mem_data_out[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_M1       = inst_mem_data_out[OPCODE_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+2*REG_ADDR_TYPE_BYTES;
            end

            // Section 2
            LOAD_MAT,
            STORE_MAT: begin
                op_addr     = inst_mem_data_out[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = inst_mem_data_out[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end
            LOAD_ROW,
            STORE_ROW: begin
                op_addr     = inst_mem_data_out[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = inst_mem_data_out[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_row_idx  = inst_mem_data_out[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            LOAD_COL,
            STORE_COL: begin
                op_addr     = inst_mem_data_out[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = inst_mem_data_out[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_col_idx  = inst_mem_data_out[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            LOAD_SCALAR,
            STORE_SCALAR: begin
                op_addr     = inst_mem_data_out[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = inst_mem_data_out[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_row_idx  = inst_mem_data_out[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                op_col_idx  = inst_mem_data_out[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE+WIDTH_IDX_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+2*WIDTH_IDX_TYPE_BYTES;
            end

            // Section 3
            // TODO: after interconnect

            // Section 4
            HALT: begin
                next_inst_offset = 0;
            end
        endcase
    end

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
        CACHE_DATA_FROM_DATA_MEM_DATA_OUT
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
                endcase
            end
        end
    endgenerate

    // Assign next state and output
    always_comb begin
        // Set default values
        next_inst_proceed = 0;
        done = 0;
        // DataMem
        data_mem_read_addr = 0;
        data_mem_write_addr = 0;
        data_mem_write_size = 0;
        // Cache
        cache_read_op = MAT_DATA_READ_DISABLE;
        cache_read_addr1 = 0;
        cache_read_addr2 = 0;
        cache_read_param = 0;
        cache_write_op = MAT_DATA_WRITE_DISABLE;
        cache_write_addr1 = 0;
        cache_write_addr2 = 0;
        cache_write_param1 = 0;
        cache_write_param2 = 0;
        cache_data_in_sel = CACHE_DATA_FROM_ZERO;

        case (state)
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
        endcase
    end

endmodule: MatControl
