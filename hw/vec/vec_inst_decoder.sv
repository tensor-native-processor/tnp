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
            VEC_INST_ADD,
            VEC_INST_SUB,
            VEC_INST_DOT: begin
                op_Vd       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V2       = inst_value[OPCODE_TYPE_SIZE+2*REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+3*REG_ADDR_TYPE_BYTES;
            end
            VEC_INST_SCALE,
            VEC_INST_DELTA: begin
                op_Vd       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V2       = inst_value[OPCODE_TYPE_SIZE+2*REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_vec_idx  = inst_value[OPCODE_TYPE_SIZE+3*REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+3*REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end
            VEC_INST_ACT_SIGMOID,
            VEC_INST_ACT_TANH,
            VEC_INST_ACT_RELU,
            VEC_INST_COPY: begin
                op_Vd       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+2*REG_ADDR_TYPE_BYTES;
            end
            VEC_INST_CLEAR: begin
                op_V1       = inst_value[OPCODE_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end

            // Section 2
            VEC_INST_LOAD_VEC,
            VEC_INST_STORE_VEC: begin
                op_addr     = inst_value[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end
            VEC_INST_LOAD_SCALAR,
            VEC_INST_STORE_SCALAR: begin
                op_addr     = inst_value[OPCODE_TYPE_SIZE +: MEM_ADDR_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_vec_idx  = inst_value[OPCODE_TYPE_SIZE+MEM_ADDR_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+MEM_ADDR_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end

            // Section 3
            VEC_INST_SEND_VEC,
            VEC_INST_RECV_VEC: begin
                op_core_idx = inst_value[OPCODE_TYPE_SIZE +: CORE_IDX_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+CORE_IDX_TYPE_BYTES+REG_ADDR_TYPE_BYTES;
            end
            VEC_INST_SEND_SCALAR,
            VEC_INST_RECV_SCALAR: begin
                op_core_idx = inst_value[OPCODE_TYPE_SIZE +: CORE_IDX_TYPE_SIZE];
                op_V1       = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE +: REG_ADDR_TYPE_SIZE];
                op_vec_idx  = inst_value[OPCODE_TYPE_SIZE+CORE_IDX_TYPE_SIZE+REG_ADDR_TYPE_SIZE +: WIDTH_IDX_TYPE_SIZE];
                inst_size = OPCODE_TYPE_BYTES+CORE_IDX_TYPE_BYTES+REG_ADDR_TYPE_BYTES+WIDTH_IDX_TYPE_BYTES;
            end

            // Section 4
            VEC_INST_HALT: begin
                inst_size = 0;
            end
        endcase
    end

endmodule: VecInstDecoder
