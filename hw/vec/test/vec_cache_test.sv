`default_nettype none

module VecCache_test();

    logic clock;
    VecDataReadOp_t read_op;
    VecDataWriteOp_t write_op;
    logic [1:0] read_addr, write_addr;
    logic [1:0] read_param, write_param;
    shortreal data_in[3:0], data_out[3:0], ans[3:0];

    VecCache #(.WIDTH(4)) DUT(.*);

    // Clock
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end

    // Check output
    function check();
        if (data_out != ans) begin
            $display("ERROR: data_out = (%f,%f,%f,%f)",
                data_out[0], data_out[1], data_out[2], data_out[3]);
            $finish;
        end
    endfunction

    // Test series
    initial begin
        $monitor($time,, "data_in=(%f,%f,%f,%f)\t data_out=(%f,%f,%f,%f)",
            data_in[0], data_in[1], data_in[2], data_in[3],
            data_out[0], data_out[1], data_out[2], data_out[3]);

        write_op = VEC_DATA_WRITE_VEC;write_addr = 0;
        data_in[0] = 4.0;   data_in[1] = 6.0;   data_in[2] = 1.0;   data_in[3] = 6.0;
        @(posedge clock);#1;
        write_op = VEC_DATA_WRITE_VEC;write_addr = 1;
        data_in[0] = 9.0;   data_in[1] = 7.0;   data_in[2] = 5.0;   data_in[3] = 3.0;
        @(posedge clock);#1;
        write_op = VEC_DATA_WRITE_VEC;write_addr = 2;
        data_in[0] = 5.0;   data_in[1] = 3.0;   data_in[2] = 0.0;   data_in[3] = 3.0;
        @(posedge clock);#1;
        write_op = VEC_DATA_WRITE_VEC;write_addr = 3;
        data_in[0] = 5.0;   data_in[1] = 3.0;   data_in[2] = 0.0;   data_in[3] = 3.0;
        @(posedge clock);#1;

        // Read data
        #1 read_op = VEC_DATA_READ_VEC;read_addr = 1;
        #1 ans[0] = 9.0;       ans[1] = 7.0;       ans[2] = 5.0;       ans[3] = 3.0;
        check();

        // Write disable
        write_op = VEC_DATA_WRITE_DISABLE;write_addr = 1;
        data_in[0] = 7.0;   data_in[1] = 3.0;   data_in[2] = 2.0;   data_in[3] = 1.0;
        @(posedge clock);#1;
        #1 read_op = VEC_DATA_READ_VEC;read_addr = 1;
        #1 ans[0] = 9.0;       ans[1] = 7.0;       ans[2] = 5.0;       ans[3] = 3.0;

        // Write scalar
        write_op = VEC_DATA_WRITE_SCALAR;write_addr = 1;write_param = 2;
        data_in[0] = 2.0;
        @(posedge clock);#1;
        #1 read_op = VEC_DATA_READ_VEC;read_addr = 1;
        #1 ans[0] = 9.0;       ans[1] = 7.0;       ans[2] = 2.0;       ans[3] = 3.0;


        #100;
        $display("All test succeeded!");
        $finish;
    end

endmodule: VecCache_test
