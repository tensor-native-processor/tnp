`default_nettype none

module VecControl_test
    #(parameter WIDTH = 16,
                CACHE_SIZE = 8,

                INST_MEM_SIZE = 2048,
                DATA_MEM_SIZE = 2048,

                INST_MEM_ADDR_SIZE = 32,
                DATA_MEM_ADDR_SIZE = 32,
                INST_MEM_WIDTH_BYTES = 16,

                // Auto-gen
                WIDTH_ADDR_SIZE = $clog2(WIDTH),
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE),

                INST_MEM_WIDTH_SIZE = 8 * INST_MEM_WIDTH_BYTES
    ) ();

    logic clock, reset, done;
     // Interface with VecUnit
    VecUnitOp_t unit_op;
    shortreal unit_data_inK;
    shortreal unit_data_in1[WIDTH-1:0];
    shortreal unit_data_in2[WIDTH-1:0];
    shortreal unit_data_out[WIDTH-1:0];

    // Interface with VecCache
    VecDataReadOp_t cache_read_op;
    logic [CACHE_ADDR_SIZE-1:0] cache_read_addr;
    logic [WIDTH_ADDR_SIZE-1:0] cache_read_param;
    VecDataWriteOp_t cache_write_op;
    logic [CACHE_ADDR_SIZE-1:0] cache_write_addr;
    logic [WIDTH_ADDR_SIZE-1:0] cache_write_param;
    shortreal cache_data_in[WIDTH-1:0];
    shortreal cache_data_out[WIDTH-1:0];

    // Interface with memory
    logic [INST_MEM_ADDR_SIZE-1:0] inst_mem_read_addr;
    logic [INST_MEM_WIDTH_SIZE-1:0] inst_mem_data_out;
    logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_read_addr;
    shortreal data_mem_data_out[WIDTH-1:0];
    VecDataMemWriteOp_t data_mem_write_op;
    logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_write_addr;
    shortreal data_mem_data_in[WIDTH-1:0];

    VecControl #(.WIDTH(WIDTH), .CACHE_SIZE(CACHE_SIZE),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .DATA_MEM_ADDR_SIZE(DATA_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_SIZE(INST_MEM_WIDTH_SIZE)
    ) DUT(.*);
    VecUnit #(.WIDTH(WIDTH)) UnitDUT(.clock, 
        .op(unit_op),
		.data_inK(unit_data_inK),
        .data_in1(unit_data_in1),
        .data_in2(unit_data_in2),
        .data_out(unit_data_out)
    );
    VecCache #(.WIDTH(WIDTH), .CACHE_SIZE(CACHE_SIZE)) CacheDUT(.clock,
        .read_op(cache_read_op),
        .read_addr(cache_read_addr),
        .read_param(cache_read_param),
        .write_op(cache_write_op),
        .write_addr(cache_write_addr),
        .write_param(cache_write_param),
        .data_in(cache_data_in), .data_out(cache_data_out)
    );
    VecInstMem #(.INST_MEM_SIZE(INST_MEM_SIZE),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_BYTES(INST_MEM_WIDTH_BYTES)
    ) InstMemDUT(
        .read_addr(inst_mem_read_addr), .data_out(inst_mem_data_out)
    );
    VecDataMem #(.DATA_MEM_SIZE(DATA_MEM_SIZE),
        .DATA_MEM_ADDR_SIZE(DATA_MEM_ADDR_SIZE),
        .DATA_MEM_WIDTH_SIZE(WIDTH)
    ) DataMemDUT(.clock(clock),
        .read_addr(data_mem_read_addr), .data_out(data_mem_data_out),
        .write_op(data_mem_write_op),
        .write_addr(data_mem_write_addr),
        .data_in(data_mem_data_in)
    );


    // Clock signal
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end
    
    integer i, data_mem_file;
    initial begin
        // Load instruction/data memory
        $readmemb("inst_mem.txt", InstMemDUT.inst_mem);
        data_mem_file = $fopen("data_mem.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", DataMemDUT.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        // Reset control
        reset = 1;
        #10 reset = 0;

        @(posedge done);
        #10;
        data_mem_file = $fopen("output.txt", "w");
        for (i = 0;i < DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", DataMemDUT.data_mem[i]);
        end
        $fclose(data_mem_file);

        $finish;
    end

endmodule: VecControl_test
