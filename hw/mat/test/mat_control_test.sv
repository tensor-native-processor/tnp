`default_nettype none

module MatControl_test
    #(parameter WIDTH = 16,
                CACHE_SIZE = 8,

                WIDTH_ADDR_SIZE = $clog2(WIDTH),
                CACHE_ADDR_SIZE = $clog2(CACHE_SIZE)
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

    MatControl #(.WIDTH(WIDTH), .CACHE_SIZE(CACHE_SIZE)) DUT(.*);
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


    // Clock signal
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end
    
    initial begin
        $readmemb("inst_mem.txt", DUT.inst_mem);
        reset = 1;
        #10 reset = 0;

        #1000 $finish;
    end

endmodule: MatControl_test
