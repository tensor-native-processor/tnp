`include "mat_inst_type.sv"

// Instruction decoder
module MatInstDecoder
    #(parameter OPCODE_TYPE_BYTES = 1,
                MEM_ADDR_TYPE_BYTES = 8,
                CORE_IDX_TYPE_BYTES = 1,
                REG_ADDR_TYPE_BYTES = 2,
                WIDTH_IDX_TYPE_BYTES = 2,

				INST_MEM_ADDR_SIZE = 32,
                INST_MEM_WIDTH_SIZE = 16,

                // Auto-generated sizes
                OPCODE_TYPE_SIZE = 8 * OPCODE_TYPE_BYTES,
                MEM_ADDR_TYPE_SIZE = 8 * MEM_ADDR_TYPE_BYTES,
                CORE_IDX_TYPE_SIZE = 8 * CORE_IDX_TYPE_BYTES,
                REG_ADDR_TYPE_SIZE = 8 * REG_ADDR_TYPE_BYTES,
                WIDTH_IDX_TYPE_SIZE = 8 * WIDTH_IDX_TYPE_BYTES
    )
    (input logic [INST_MEM_WIDTH_SIZE-1:0] inst_value,
     output logic [INST_MEM_ADDR_SIZE-1:0] inst_size,
     output logic [OPCODE_TYPE_SIZE-1:0] opcode,
     output logic [MEM_ADDR_TYPE_SIZE-1:0] op_addr,
     output logic [CORE_IDX_TYPE_SIZE-1:0] op_core_idx,
     output logic [REG_ADDR_TYPE_SIZE-1:0] op_Md, op_M1, op_M2,
     output logic [WIDTH_IDX_TYPE_SIZE-1:0] op_row_idx, op_col_idx, op_diag_idx
    );

    // Assign instruction to opcode/operands
    always_comb begin
        opcode = 0;
        op_addr = 0;
        op_core_idx = 0;
        {op_Md, op_M1, op_M2} = 0;
        {op_row_idx, op_col_idx, op_diag_idx} = 0;

        opcode = inst_value[OPCODE_TYPE_SIZE-1:0];
        inst_size = OPCODE_TYPE_BYTES;

        case (opcode)
            // Section 1
            MAT_INST_SET_WEIGHT,
            MAT_INST_TRANSPOSE,
            MAT_INST_CLEAR,
            MAT_INST_XFLIP,
            MAT_INST_YFLIP: begin
                op_M1       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end
            MAT_INST_COPY,
            MAT_INST_MULTIPLY: begin
                op_Md       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+2*REG_ADDR_TYPE_BYTES;
            end

            // Section 2
            MAT_INST_LOAD_MAT,
            MAT_INST_STORE_MAT: begin
                op_addr     = inst_value[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end
            MAT_INST_LOAD_ROW,
            MAT_INST_STORE_ROW: begin
                op_addr     = inst_value[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_row_idx  = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            MAT_INST_LOAD_COL,
            MAT_INST_STORE_COL: begin
                op_addr     = inst_value[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_col_idx  = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            MAT_INST_LOAD_SCALAR,
            MAT_INST_STORE_SCALAR: begin
                op_addr     = inst_value[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_row_idx  = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                op_col_idx  = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE+WIDTH_IDX_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+2*WIDTH_IDX_TYPE_BYTES;
            end

            // Section 3
            MAT_INST_SEND_ROW,
            MAT_INST_RECV_ROW: begin
                op_core_idx = inst_value[OPCODE_TYPE_SIZE +: CORE_IDX_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_row_idx  = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+CORE_IDX_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            MAT_INST_SEND_COL,
            MAT_INST_RECV_COL: begin
                op_core_idx = inst_value[OPCODE_TYPE_SIZE +: CORE_IDX_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_col_idx  = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+CORE_IDX_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            MAT_INST_SEND_SCALAR,
            MAT_INST_RECV_SCALAR: begin
                op_core_idx = inst_value[OPCODE_TYPE_SIZE +: CORE_IDX_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_row_idx  = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                op_col_idx  = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE+REG_ADDR_TYPE_SIZE+WIDTH_IDX_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+CORE_IDX_TYPE_BYTES+REG_ADDR_TYPE_BYTES+2*WIDTH_IDX_TYPE_BYTES;
            end
            MAT_INST_SEND_DIAG,
            MAT_INST_RECV_DIAG: begin
                op_core_idx = inst_value[OPCODE_TYPE_SIZE +: CORE_IDX_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_M2       = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_diag_idx = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE+2*REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+CORE_IDX_TYPE_BYTES+2*REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            MAT_INST_RECV_DIAG1,
            MAT_INST_RECV_DIAG2: begin
                op_core_idx = inst_value[OPCODE_TYPE_SIZE +: CORE_IDX_TYPE_SIZE];
                op_M1       = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_diag_idx = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+CORE_IDX_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end


            // Section 4
            MAT_INST_HALT: begin
                inst_size = 0;
            end
        endcase
    end

endmodule: MatInstDecoder
