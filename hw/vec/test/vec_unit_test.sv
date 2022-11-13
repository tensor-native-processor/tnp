`default_nettype none

module MatUnit_test();

    logic clock;

    shortreal data_inK, data_in1[3:0], data_in2[3:0], data_out[3:0], ans[3:0];
    VecUnitOp_t op;

    VecUnit #(4) DUT(.*);

    // Clock
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end

    // Validate data_out
    function check();
        if (data_out != ans) begin
            $display("ERROR: data_out = (%f,%f,%f,%f)",
                data_out[0], data_out[1], data_out[2], data_out[3]);
            $finish;
        end
    endfunction

    // Test series
    initial begin
        $monitor($time,, "data_in=(%f,%f,%f,%f,%f,%f,%f,%f)\t data_out=(%f,%f,%f,%f)",
            data_in1[0], data_in1[1], data_in1[2], data_in1[3],
            data_in2[0], data_in2[1], data_in2[2], data_in2[3],
            data_out[0], data_out[1], data_out[2], data_out[3]);

        // Add 
        @(posedge clock); #1;
        op = VEC_UNIT_OP_ADD;
        data_in1[0] = 1.0;   data_in1[1] = 1.0;   data_in1[2] = 0.0;   data_in1[3] = 0.0;
        data_in2[0] = 6.0;   data_in2[1] = 7.0;   data_in2[2] = 8.0;   data_in2[3] = 9.0;
        ans[0] = 7.0;   ans[1] = 8.0;   ans[2] = 8.0;   ans[3] = 9.0;
        @(posedge clock);
        check();

        // Subtract
        op = VEC_UNIT_OP_SUB;
        data_in1[0] = 1.0;   data_in1[1] = 1.0;   data_in1[2] = 0.0;   data_in1[3] = 0.0;
        data_in2[0] = 6.0;   data_in2[1] = 7.0;   data_in2[2] = 8.0;   data_in2[3] = 9.0;
        ans[0] = -5.0;   ans[1] = -6.0;   ans[2] = -8.0;   ans[3] = -9.0;
        @(posedge clock);
        check();

        // Scale
        op = VEC_UNIT_OP_SCALE;
        data_in1[0] = 1.0;   data_in1[1] = 3.0;   data_in1[2] = 2.0;   data_in1[3] = 0.0;
        data_inK = -5.0;
        ans[0] = -5.0;   ans[1] = -15.0;   ans[2] = -10.0;   ans[3] = -0.0;
        @(posedge clock);
        check();

        // Sigmoid
        op = VEC_UNIT_OP_ACT_SIGMOID;
        data_in1[0] = 1.0;   data_in1[1] = 3.0;   data_in1[2] = -2.0;   data_in1[3] = -20.0;
        @(posedge clock);


        #100;
        $display("All test succeeded!");
        $finish;
    end

endmodule: MatUnit_test
