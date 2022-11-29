`default_nettype none

typedef enum logic [7:0] {
	// Section 1
	VEC_INST_ADD                = 8'b00100000,
	VEC_INST_SUB                = 8'b00100001,
	VEC_INST_DOT                = 8'b00100010,
	VEC_INST_SCALE              = 8'b00100011,
	VEC_INST_DELTA              = 8'b00100100,
	VEC_INST_ACT_SIGMOID        = 8'b00110000,
	VEC_INST_ACT_TANH           = 8'b00110001,
	VEC_INST_ACT_RELU           = 8'b00110010,

    VEC_INST_CLEAR              = 8'b00111000,
    VEC_INST_COPY               = 8'b00111001,

	// Section 2
	VEC_INST_LOAD_VEC           = 8'b01000000,
	VEC_INST_LOAD_SCALAR        = 8'b01000001,
	VEC_INST_STORE_VEC          = 8'b01010000,
	VEC_INST_STORE_SCALAR       = 8'b01010001,

	// Section 3
	VEC_INST_SEND_VEC           = 8'b01100000,
	VEC_INST_SEND_SCALAR        = 8'b01100001,

	VEC_INST_RECV_VEC           = 8'b01110000,
	VEC_INST_RECV_SCALAR        = 8'b01110001,

	// Section 4
	VEC_INST_HALT               = 8'b10000000
} VecCoreOpcode;
