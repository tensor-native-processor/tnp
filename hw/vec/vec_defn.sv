`default_nettype none

typedef enum logic [7:0] {
	// Section 1
	ADD             = 8'b00100000,
	SUB             = 8'b00100001,
	DOT             = 8'b00100010,
	SCALE           = 8'b00100011,
	DELTA           = 8'b00100100,
	ACT_SIGMOID     = 8'b00110000,
	ACT_TANH        = 8'b00110001,
	ACT_RELU        = 8'b00110010,

	// Section 2
	LOAD_VEC        = 8'b01000000,
	LOAD_SCALAR     = 8'b01000001,
	STORE_VEC       = 8'b01010000,
	STORE_SCALAR    = 8'b01010001,

	// Section 3
	SEND_VEC        = 8'b01100000,
	SEND_SCALAR     = 8'b01100001,

	RECV_VEC        = 8'b01110000,
	RECV_SCALAR     = 8'b01110001,

	// Section 4
	HALT            = 8'b10000000
} VecCoreOpcode;
