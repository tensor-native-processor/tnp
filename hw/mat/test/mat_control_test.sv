`default_nettype none

module MatControl_test
    #(parameter WIDTH = 16,
                CACHE_SIZE = 8,

                INST_MEM_SIZE = 1024,
                DATA_MEM_SIZE = 1024,

                INST_MEM_ADDR_SIZE = 32,
                DATA_MEM_ADDR_SIZE = 32,
                INST_MEM_WIDTH_BYTES = 16,

                // Auto-gen
                WIDTH_ADDR_SIZE = $clog2(WIDTH),
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE),

                INST_MEM_WIDTH_SIZE = 8 * INST_MEM_WIDTH_BYTES
    ) ();

    logic clock, reset, done;

    logic unit_set_weight;
    logic [WIDTH_ADDR_SIZE-1:0] unit_set_weight_row;
    shortreal unit_data_in[WIDTH-1:0];
    shortreal unit_data_out[WIDTH-1:0];

    MatDataReadOp_t cache_read_op;
    logic [CACHE_ADDR_SIZE-1:0] cache_read_addr1, cache_read_addr2;
    logic [WIDTH_ADDR_SIZE-1:0] cache_read_param;
    MatDataWriteOp_t cache_write_op;
    logic [CACHE_ADDR_SIZE-1:0] cache_write_addr1, cache_write_addr2;
    logic [WIDTH_ADDR_SIZE-1:0] cache_write_param1, cache_write_param2;
    shortreal cache_data_in[WIDTH-1:0];
    shortreal cache_data_out[WIDTH-1:0];

    logic [INST_MEM_ADDR_SIZE-1:0] inst_mem_read_addr;
    logic [INST_MEM_WIDTH_SIZE-1:0] inst_mem_data_out;
    logic [DATA_MEM_ADDR_SIZE-1:0] data_mem_read_addr;
    shortreal data_mem_data_out[WIDTH-1:0];


    MatControl #(.WIDTH(WIDTH), .CACHE_SIZE(CACHE_SIZE),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .DATA_MEM_ADDR_SIZE(DATA_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_SIZE(INST_MEM_WIDTH_SIZE)
    ) DUT(.*);
    MatUnit #(.WIDTH(WIDTH)) UnitDUT(.clock, .set_weight(unit_set_weight),
        .set_weight_row(unit_set_weight_row), .data_in(unit_data_in),
        .data_out(unit_data_out)
    );
    MatCache #(.WIDTH(WIDTH), .CACHE_SIZE(CACHE_SIZE)) CacheDUT(.clock,
        .read_op(cache_read_op),
        .read_addr1(cache_read_addr1), .read_addr2(cache_read_addr2),
        .read_param(cache_read_param),
        .write_op(cache_write_op),
        .write_addr1(cache_write_addr1), .write_addr2(cache_write_addr2),
        .write_param1(cache_write_param1), .write_param2(cache_write_param2),
        .data_in(cache_data_in), .data_out(cache_data_out)
    );
    MatInstMem #(.INST_MEM_SIZE(INST_MEM_SIZE),
        .INST_MEM_ADDR_SIZE(INST_MEM_ADDR_SIZE),
        .INST_MEM_WIDTH_BYTES(INST_MEM_WIDTH_BYTES)
    ) InstMemDUT(.read_addr(inst_mem_read_addr), .data_out(inst_mem_data_out));
    MatDataMem #(.DATA_MEM_SIZE(DATA_MEM_SIZE),
        .DATA_MEM_ADDR_SIZE(DATA_MEM_ADDR_SIZE),
        .DATA_MEM_WIDTH_SIZE(WIDTH)
    ) DataMemDUT(.read_addr(data_mem_read_addr), .data_out(data_mem_data_out));


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

        #1000 $finish;
    end

endmodule: MatControl_test
