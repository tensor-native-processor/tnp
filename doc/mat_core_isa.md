# Matrix Core ISA

## 0. Notes

- Md, M1, M2, etc. are matrix registers (inside matrix cache). There are a specific number of matrix registers available on each core.
- Grouping `MULTIPLY` together is strongly recommended (maximum 2x throughput).
- Main memory operations should be avoided if possible. 
- Compilers should generate matching send and receive instructions to avoid deadlock.

---

## 1. Arithmetic operations

#### SET_WEIGHT M1

Load weight matrix `M1` into systolic array (to be multiplied by `MULTIPLY` instruction).

#### MULTIPLY Md M1

Multiply matrix `M1` by loaded weights, and store output matrix to `Md`

__Warning__: `MULTIPLY` equivalents to $Md_{i, j} = \sum W_{k, WIDTH-i} \cdot M1_{k, j}$. To use normal matrix multiplication, `XFLIP` and `TRANSPOSE` the weight matrix before `SET_WEIGHT`.

#### TRANSPOSE M1

Transpose matrix `M1`

#### XFLIP M1

Flip matrix `M1` on X axis (vertically).

#### YFLIP M1

Flip matrix `M1` on Y axis (horizontally).

---

## 2. Main memory operations

### Loading from memory

#### LOAD_MAT addr M1

Load matrix `M1` from DRAM address `addr`

#### LOAD_ROW addr M1 row_idx

Load matrix row `row_idx` into matrix `M1`, from DRAM address `addr`

#### LOAD_COL addr M1 col_idx

Load matrix column `col_idx` into matrix `M1`, from DRAM address `addr`

#### LOAD_SCALAR addr M1 row_idx col_idx

Load matrix element (`row_idx`, `col_idx`) into matrix `M1`, from DRAM address `addr`



### Storing to memory

#### STORE_MAT addr M1

Store matrix `M1` into DRAM address `addr`

#### STORE_ROW addr M1 row_idx

Store matrix `M1` row `row_idx` into DRAM address `addr`

#### STORE_COL addr M1 col_idx

Store matrix `M1` column `col_idx` into DRAM address `addr`

#### STORE_SCALAR addr M1 row_idx col_idx

Store matrix `M1` element (`row_idx`, `col_idx`) into DRAM address `addr`

---

## 3. Interconnect operations (synchronous)

### Sending data to another core

#### SEND_ROW core_idx M1 row_idx

Send matrix `M1` row `row_idx` to core `core_idx`

#### SEND_COL core_idx M1 col_idx

Send matrix `M1` column `col_idx` to core `core_idx`

#### SEND_SCALAR core_idx M1 row_idx col_idx

Send scalar in matrix `M1` row `row_idx` column `col_idx` to core `core_idx`

#### SEND_DIAG core_idx M1 M2 diag_idx

Send matrix `M1`'s primary diagonal and `M2`'s secondary diagonal concatenated together (`diag_idx`-th diagonal) to core `core_idx`



### Receiving data from another core

#### RECV_ROW core_idx M1 row_idx

Receive from core `core_idx` into `M1` matrix row `row_idx`

#### RECV_COL core_idx M1 col_idx

Receive from core `core_idx` into `M1`matrix column `col_idx`

#### RECV_SCALAR core_idx M1 row_idx col_idx

Receive scalar from core `core_idx` into `M1` matrix element (`row_idx`, `col_idx`)

#### RECV_DIAG core_idx M1 M2 diag_idx

Receive from core `core_idx` into `M1` primary diagonal and `M2`'s secondary diagonal concatenated together (`diag_idx`-th diagonal)

#### RECV_DIAG1 core_idx M1 diag_idx

Receive from core `core_idx` into `M1` primary diagonal at `diag_idx`

#### RECV_DIAG2 core_idx M1 diag_idx

Receive from core `core_idx` into `M1` secondary diagonal at `diag_idx`

---

## 4. Control operations

#### HALT

Stop processing
