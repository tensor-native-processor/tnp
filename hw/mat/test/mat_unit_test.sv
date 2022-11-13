`default_nettype none

module MatUnit_test();

    logic clock;
    logic set_weight;
    logic [1:0] set_weight_row;

    shortreal data_in[3 : 0], data_out[3 : 0];

    MatUnit #(4) DUT(.*);

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
        set_weight = 0;

        data_in[0] = 1.0;   data_in[1] = 0.0;   data_in[2] = 0.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        data_in[0] = 1.0;   data_in[1] = 1.0;   data_in[2] = 0.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        data_in[0] = 1.0;   data_in[1] = 1.0;   data_in[2] = 1.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        data_in[0] = 1.0;   data_in[1] = 1.0;   data_in[2] = 1.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        set_weight = 1;     set_weight_row = 0;
        data_in[0] = 1.0;   data_in[1] = 1.0;   data_in[2] = 1.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        set_weight = 1;     set_weight_row = 1;
        data_in[0] = 5.0;   data_in[1] = 1.0;   data_in[2] = 1.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        set_weight = 1;     set_weight_row = 2;
        data_in[0] = 5.0;   data_in[1] = 5.0;   data_in[2] = 1.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        set_weight = 1;     set_weight_row = 3;
        data_in[0] = 5.0;   data_in[1] = 5.0;   data_in[2] = 5.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        set_weight = 0;
        data_in[0] = 5.0;   data_in[1] = 5.0;   data_in[2] = 5.0;   data_in[3] = 5.0;

        @(posedge clock); #1;
        check(0, 20.0);
        data_in[0] = 1.0;   data_in[1] = 5.0;   data_in[2] = 5.0;   data_in[3] = 5.0;

        @(posedge clock); #1;
        check(0, 20.0);     check(1, 20.0);
        data_in[0] = 0.0;   data_in[1] = 1.0;   data_in[2] = 5.0;   data_in[3] = 5.0;

        @(posedge clock); #1;
        check(0, 20.0);     check(1, 20.0);     check(2, 20.0);
        data_in[0] = 1.0;   data_in[1] = 0.0;   data_in[2] = 1.0;   data_in[3] = 5.0;

        @(posedge clock); #1;
        check(0, 20.0);     check(1, 20.0);     check(2, 20.0);     check(3, 20.0);
        data_in[0] = 0.0;   data_in[1] = 2.0;   data_in[2] = 0.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        check(0, 4.0);      check(1, 20.0);     check(2, 20.0);     check(3, 20.0);
        data_in[0] = 0.0;   data_in[1] = 0.0;   data_in[2] = 3.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        check(0, 0.0);      check(1, 4.0);      check(2, 20.0);     check(3, 20.0);
        data_in[0] = 0.0;   data_in[1] = 0.0;   data_in[2] = 0.0;   data_in[3] = 4.0;

        @(posedge clock); #1;
        check(0, 10.0);     check(1, 0.0);      check(2, 4.0);      check(3, 20.0);
        data_in[0] = 0.0;   data_in[1] = 0.0;   data_in[2] = 0.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        check(0, 0.0);      check(1, 10.0);     check(2, 0.0);      check(3, 4.0);
        @(posedge clock); #1;
        check(0, 0.0);      check(1, 0.0);      check(2, 10.0);     check(3, 0.0);
        @(posedge clock); #1;
        check(0, 0.0);      check(1, 0.0);      check(2, 0.0);      check(3, 10.0);
        @(posedge clock); #1;
        check(0, 0.0);      check(1, 0.0);      check(2, 0.0);      check(3, 0.0);

        #100;
        $display("All test succeeded!");
        $finish;
    end

endmodule: MatUnit_test
