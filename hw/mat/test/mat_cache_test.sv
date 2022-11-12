`default_nettype none

module MatCache_test();

    logic clock;
    MatCacheReadOp_t read_op;
    MatCacheWriteOp_t write_op;
    logic [1 : 0] read_addr1, read_addr2, write_addr1, write_addr2;
    logic [1 : 0] read_param, write_param;
    shortreal data_in[3 : 0], data_out[3 : 0];

    MatCache #(.WIDTH(4), .CACHE_SIZE(4)) DUT(.*);

    // Clock
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end

    // Validate data_out
    function check(int idx, shortreal ans);
        if (data_out[idx] != ans) begin
            $display("ERROR: data_out[%d] = %f -- %f", idx, data_out[idx], ans);
            $finish;
        end
    endfunction

    // Test series
    initial begin
        $monitor($time,, "data_in=(%f,%f,%f,%f)\t data_out=(%f,%f,%f,%f)",
            data_in[0], data_in[1], data_in[2], data_in[3],
            data_out[0], data_out[1], data_out[2], data_out[3]);

        write_op = MAT_CACHE_WRITE_ROW;write_addr1 = 0;write_param = 2'd0;
        data_in[0] = 4.0;   data_in[1] = 6.0;   data_in[2] = 1.0;   data_in[3] = 6.0;

        @(posedge clock);#1;
        read_op = MAT_CACHE_READ_ROW;read_addr1 = 0;read_param = 2'd0;
        write_op = MAT_CACHE_WRITE_ROW;write_addr1 = 0;write_param = 2'd1;
        data_in[0] = 1.0;   data_in[1] = 2.0;   data_in[2] = 3.0;   data_in[3] = 4.0;

        @(posedge clock);#1;
        write_op = MAT_CACHE_WRITE_ROW;write_addr1 = 0;write_param = 2'd2;
        data_in[0] = 3.0;   data_in[1] = 3.0;   data_in[2] = 3.0;   data_in[3] = 3.0;

        @(posedge clock);#1;
        write_op = MAT_CACHE_WRITE_ROW;write_addr1 = 0;write_param = 2'd3;
        data_in[0] = 9.0;   data_in[1] = 7.0;   data_in[2] = 5.0;   data_in[3] = 3.0;

        @(posedge clock);#1;
        #1; read_op = MAT_CACHE_READ_DIAG;read_addr1 = 0;read_addr2 = 0;read_param = 2'd0;
        #1; check(0, 4.0);  check(1, 4.0);      check(2, 3.0);      check(3, 7.0);
        #1; read_op = MAT_CACHE_READ_DIAG;read_addr1 = 0;read_addr2 = 0;read_param = 2'd1;
        #1; check(0, 6.0);  check(1, 1.0);      check(2, 3.0);      check(3, 5.0);
        #1; read_op = MAT_CACHE_READ_DIAG;read_addr1 = 0;read_addr2 = 0;read_param = 2'd2;
        #1; check(0, 1.0);  check(1, 2.0);      check(2, 3.0);      check(3, 3.0);
        #1; read_op = MAT_CACHE_READ_DIAG;read_addr1 = 0;read_addr2 = 0;read_param = 2'd3;
        #1; check(0, 6.0);  check(1, 3.0);      check(2, 3.0);      check(3, 9.0);

        @(posedge clock);#1;
        write_op = MAT_CACHE_WRITE_COL;write_addr1 = 2;write_param = 2'd1;
        data_in[0] = 1.0;   data_in[1] = 2.0;   data_in[2] = 3.0;   data_in[3] = 4.0;

        @(posedge clock);#1;
        write_op = MAT_CACHE_WRITE_COL;write_addr1 = 2;write_param = 2'd0;
        data_in[0] = 3.0;   data_in[1] = 3.0;   data_in[2] = 3.0;   data_in[3] = 3.0;

        @(posedge clock);#1;
        write_op = MAT_CACHE_WRITE_COL;write_addr1 = 2;write_param = 2'd3;
        data_in[0] = 9.0;   data_in[1] = 7.0;   data_in[2] = 5.0;   data_in[3] = 3.0;

        @(posedge clock);#1;
        write_op = MAT_CACHE_WRITE_COL;write_addr1 = 2;write_param = 2'd2;
        data_in[0] = 9.0;   data_in[1] = 7.0;   data_in[2] = 5.0;   data_in[3] = 3.0;


        #100;
        $display("All test succeeded!");
        $finish;
    end

endmodule: MatCache_test
