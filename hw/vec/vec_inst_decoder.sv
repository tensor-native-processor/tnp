`default_nettype none

// Instruction decoder
module VecInstDecoder
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
     output logic [REG_ADDR_TYPE_SIZE-1:0] op_Vd, op_V1, op_V2,
     output logic [WIDTH_IDX_TYPE_SIZE-1:0] op_vec_idx
    );

    // Assign instruction to opcode/operands
    always_comb begin
        opcode = 0;
        op_addr = 0;
        op_core_idx = 0;
        {op_Vd, op_V1, op_V2} = 0;
        op_vec_idx = 0;

        opcode = inst_value[OPCODE_TYPE_SIZE-1:0];
        inst_size = OPCODE_TYPE_BYTES;

        case (opcode)
            // Section 1
            ADD,
            SUB,
            DOT: begin
                op_Vd       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V2       = inst_value[OPCODE_TYPE_SIZE+2*REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+3*REG_ADDR_TYPE_BYTES;
            end
            SCALE,
            DELTA: begin
                op_Vd       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V2       = inst_value[OPCODE_TYPE_SIZE+2*REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_vec_idx  = inst_value[OPCODE_TYPE_SIZE+3*REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+3*REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            ACT_SIGMOID,
            ACT_TANH,
            ACT_RELU,
            COPY: begin
                op_Vd       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+2*REG_ADDR_TYPE_BYTES;
            end
            CLEAR: begin
                op_V1       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end

            // Section 2
            LOAD_VEC,
            STORE_VEC: begin
                op_addr     = inst_value[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end
            LOAD_SCALAR,
            STORE_SCALAR: begin
                op_addr     = inst_value[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_vec_idx  = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end

            // Section 3
            // TODO: after interconnect

            // Section 4
            HALT: begin
                inst_size = 0;
            end
        endcase
    end

endmodule: VecInstDecoder
