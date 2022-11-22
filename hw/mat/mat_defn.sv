`default_nettype none

typedef enum logic [7:0] {
    // Section 1:
    SET_WEIGHT      = 8'b00100000,
    MULTIPLY        = 8'b00100001,
    TRANSPOSE       = 8'b00100010,
    XFLIP           = 8'b00100100,
    YFLIP           = 8'b00100101,

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
    SEND_SCALAR     = 8'b01100010,
    SEND_DIAG       = 8'b01101000,

    RECV_ROW        = 8'b01110000,
    RECV_COL        = 8'b01110001,
    RECV_SCALAR     = 8'b01110010,
    RECV_DIAG       = 8'b01111000,
    RECV_DIAG1      = 8'b01111001,
    RECV_DIAG2      = 8'b01111010,

    // Section 4
    HALT            = 8'b10000000
} MatCoreOpcode;
