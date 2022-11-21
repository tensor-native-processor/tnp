`default_nettype none

typedef enum {
    VEC_DATA_WRITE_DISABLE,
    VEC_DATA_WRITE_ZERO,
    VEC_DATA_WRITE_VEC,
    VEC_DATA_WRITE_SCALAR
} VecDataWriteOp_t;

typedef enum {
    VEC_DATA_READ_DISABLE,
    VEC_DATA_READ_VEC,
    VEC_DATA_READ_SCALAR
} VecDataReadOp_t;
