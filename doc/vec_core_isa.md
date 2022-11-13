# Vector Core ISA

## 0. Notes

- Main memory operations should be avoided if possible. 
- Compilers should generate matching send and receive instructions to avoid deadlock.

---

## 1. Arithmetic operations

#### ADD Vd V1 V2

Add vector `V1` and `V2` and store the output into `Vd`

#### SUB Vd V1 V2

Subtract vector `V1` and `V2` and store the output into `Vd`

#### DOT Vd V1 V2

Multiply vector `V1` and `V2` and store the output into `Vd`

#### SCALE Vd V1 V2 vec_idx

Multiply vector `V1` and scalar `V2`'s `vec_idx`-th element and store the output into `Vd`

#### DELTA Vd V1 V2 vec_idx

Add vector `V1` and scalar `V2`'s `vec_idx`-th element and store the output into `Vd`

#### ACT_SIGMOID Vd V1

Apply sigmoid function on `V1`and store the output into `Vd`

#### ACT_TANH Vd V1

Apply tanh function on `V1`and store the output into `Vd`

#### ACT_RELU Vd V1

Apply relu function on `V1`and store the output into `Vd`

---

## 2. Main memory operations

### Loading from memory

#### LOAD_VEC addr V1

Load vector `V1` from DRAM address `addr`

#### LOAD_SCALAR addr V1 vec_idx

Load vector element `vec_idx` into vector `V1` from DRAM address `addr`



### Storing to memory

#### STORE_VEC addr V1

Store vector `V1` into DRAM address `addr`

#### STORE_SCALAR addr V1 vec_idx

Store vector element `vec_idx` into vector `V1` into DRAM address `addr`

---

## 3. Interconnect operations (synchronous)

### Sending data to another core

#### SEND_VEC core_idx V1

Send vector `V1` to core `core_idx`



### Receiving data from another core

#### RECV_VEC core_idx V1

Receive from core `core_idx` into vector `V1` 

#### RECV_SCALAR core_idx V1 vec_idx elem_idx

Receive from core `core_idx`'s `elem_idx`-th element into `V1` `vec_idx` element 

---

## 4. Control operations

#### HALT

Stop processing
