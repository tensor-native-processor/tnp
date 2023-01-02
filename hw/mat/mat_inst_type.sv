`ifndef __MAT_INST_TYPE__
`define __MAT_INST_TYPE__

typedef enum logic [7:0] {
    // Section 1:
    MAT_INST_SET_WEIGHT         = 8'b00100000,
    MAT_INST_MULTIPLY           = 8'b00100001,
    MAT_INST_TRANSPOSE          = 8'b00100010,
    MAT_INST_XFLIP              = 8'b00100100,
    MAT_INST_YFLIP              = 8'b00100101,

    MAT_INST_COPY               = 8'b00110000,
    MAT_INST_CLEAR              = 8'b00110001,

    // Section 2
    MAT_INST_LOAD_MAT           = 8'b01000000,
    MAT_INST_LOAD_ROW           = 8'b01000001,
    MAT_INST_LOAD_COL           = 8'b01000010,
    MAT_INST_LOAD_SCALAR        = 8'b01000011,
    MAT_INST_STORE_MAT          = 8'b01010000,
    MAT_INST_STORE_ROW          = 8'b01010001,
    MAT_INST_STORE_COL          = 8'b01010010,
    MAT_INST_STORE_SCALAR       = 8'b01010011,

    // Section 3
    MAT_INST_SEND_ROW           = 8'b01100000,
    MAT_INST_SEND_COL           = 8'b01100001,
    MAT_INST_SEND_SCALAR        = 8'b01100010,
    MAT_INST_SEND_DIAG          = 8'b01101000,

    MAT_INST_RECV_ROW           = 8'b01110000,
    MAT_INST_RECV_COL           = 8'b01110001,
    MAT_INST_RECV_SCALAR        = 8'b01110010,
    MAT_INST_RECV_DIAG          = 8'b01111000,
    MAT_INST_RECV_DIAG1         = 8'b01111001,
    MAT_INST_RECV_DIAG2         = 8'b01111010,

    // Section 4
    MAT_INST_HALT               = 8'b10000000
} MatCoreOpcode;

`endif
