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
    #(parameter INST_MEM_SIZE = 100,
                DATA_MEM_SIZE = 100,
                INST_MEM_ADDR_SIZE = 32,
                OPCODE_TYPE_BYTES = 1,
                MEM_ADDR_TYPE_BYTES = 8,
                CORE_IDX_TYPE_BYTES = 1,
                REG_ADDR_TYPE_BYTES = 2,
                WIDTH_IDX_TYPE_BYTES = 2,
                MAX_INST_TOTAL_BYTES = 16,

                // Auto-generated sizes
                OPCODE_TYPE_SIZE = 8 * OPCODE_TYPE_BYTES,
                MEM_ADDR_TYPE_SIZE = 8 * MEM_ADDR_TYPE_BYTES,
                CORE_IDX_TYPE_SIZE = 8 * CORE_IDX_TYPE_BYTES,
                REG_ADDR_TYPE_SIZE = 8 * REG_ADDR_TYPE_BYTES,
                WIDTH_IDX_TYPE_SIZE = 8 * WIDTH_IDX_TYPE_BYTES,
                MAX_INST_TOTAL_SIZE = 8 * MAX_INST_TOTAL_BYTES)
    (input logic clock, reset);

    // Data memory and instruction memory (initialized by testbench)
    logic [7:0] inst_mem[INST_MEM_SIZE-1:0];
    shortreal data_mem[DATA_MEM_SIZE-1:0];

    // State machine
    enum {
        INIT, READY, NEXT, STOP
    } state, next_state;

    // Program counter register
    logic [INST_MEM_ADDR_SIZE-1:0] program_counter;
    logic program_counter_proceed;
    // Offset to next instruction (size of current instruction)
    logic [INST_MEM_ADDR_SIZE-1:0] next_inst_offset;
    // Content of current instruction
    logic [MAX_INST_TOTAL_SIZE-1:0] cur_inst;

    // Assign from inst_mem
    genvar i;
    generate
        for (i = 0;i < MAX_INST_TOTAL_BYTES;i++)
            assign cur_inst[i*8+7:i*8] = inst_mem[program_counter+i][7:0];
    endgenerate;

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

        opcode = cur_inst[OPCODE_TYPE_SIZE-1:0];
        next_inst_offset = OPCODE_TYPE_BYTES;

        case (opcode)
            // Section 1
            SET_WEIGHT,
            TRANSPOSE: begin
                op_M1       = cur_inst[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end
            MULTIPLY: begin
                op_Md       = cur_inst[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_M1       = cur_inst[OPCODE_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+2*REG_ADDR_TYPE_BYTES;
            end

            // Section 2
            LOAD_MAT,
            STORE_MAT: begin
                op_addr     = cur_inst[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = cur_inst[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end
            LOAD_ROW,
            STORE_ROW: begin
                op_addr     = cur_inst[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = cur_inst[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_row_idx  = cur_inst[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            LOAD_COL,
            STORE_COL: begin
                op_addr     = cur_inst[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = cur_inst[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_col_idx  = cur_inst[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                next_inst_offset = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            LOAD_SCALAR,
            STORE_SCALAR: begin
                op_addr     = cur_inst[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = cur_inst[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_row_idx  = cur_inst[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                op_col_idx  = cur_inst[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE+WIDTH_IDX_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
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
            program_counter <= 0;
        else if (program_counter_proceed)
            program_counter <= program_counter + next_inst_offset;
    end

    // Assign next state and output
    always_comb begin
        program_counter_proceed = 0;
        case (state)
            INIT: begin
                next_state = READY;
            end
            READY: begin
                next_state = NEXT;
            end
            NEXT: begin
                program_counter_proceed = 1;
                next_state = READY;
            end
        endcase
    end

endmodule: MatControl
