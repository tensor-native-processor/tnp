`default_nettype none

typedef enum {
    MAT_DATA_WRITE_DISABLE,
    MAT_DATA_WRITE_TRANSPOSE,
    MAT_DATA_WRITE_ROW,
    MAT_DATA_WRITE_COL,
    MAT_DATA_WRITE_SCALAR,
    MAT_DATA_WRITE_DIAG,
    MAT_DATA_WRITE_DIAG1,
    MAT_DATA_WRITE_DIAG2
} MatDataWriteOp_t;

typedef enum {
    MAT_DATA_READ_DISABLE,
    MAT_DATA_READ_ROW,
    MAT_DATA_READ_COL,
    MAT_DATA_READ_SCALAR,
    MAT_DATA_READ_DIAG
} MatDataReadOp_t;
