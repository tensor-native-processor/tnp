`default_nettype none

module MatUnit_test();

    logic clock;
    logic load_weight;
    logic [2 : 0] weight_progress;

    shortreal data_in[3 : 0], data_out[3 : 0];

    MatUnit #(4) DUT(.*);

    // Clock
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end

    // Test series
    initial begin
        $monitor($time,, "data_in=(%f,%f,%f,%f)\t data_out=(%f,%f,%f,%f)",
            data_in[0], data_in[1], data_in[2], data_in[3],
            data_out[0], data_out[1], data_out[2], data_out[3]);
        load_weight = 1;

        weight_progress = 3'd0;
        data_in[0] = 1.0;   data_in[1] = 0.0;   data_in[2] = 0.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        weight_progress = 3'd1;
        data_in[0] = 1.0;   data_in[1] = 1.0;   data_in[2] = 0.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        weight_progress = 3'd2;
        data_in[0] = 1.0;   data_in[1] = 1.0;   data_in[2] = 1.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        weight_progress = 3'd3;
        data_in[0] = 1.0;   data_in[1] = 1.0;   data_in[2] = 1.0;   data_in[3] = 0.0;

        @(posedge clock); #1;
        weight_progress = 3'd4;
        data_in[0] = 1.0;   data_in[1] = 1.0;   data_in[2] = 1.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        weight_progress = 3'd5;
        data_in[0] = 5.0;   data_in[1] = 1.0;   data_in[2] = 1.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        weight_progress = 3'd6;
        data_in[0] = 5.0;   data_in[1] = 5.0;   data_in[2] = 1.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        weight_progress = 3'd7;
        data_in[0] = 5.0;   data_in[1] = 5.0;   data_in[2] = 5.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        load_weight = 0;
        data_in[0] = 5.0;   data_in[1] = 5.0;   data_in[2] = 5.0;   data_in[3] = 5.0;

        @(posedge clock); #1;
        data_in[0] = 1.0;   data_in[1] = 5.0;   data_in[2] = 5.0;   data_in[3] = 5.0;

        @(posedge clock); #1;
        data_in[0] = 0.0;   data_in[1] = 1.0;   data_in[2] = 5.0;   data_in[3] = 5.0;

        @(posedge clock); #1;
        data_in[0] = 0.0;   data_in[1] = 0.0;   data_in[2] = 1.0;   data_in[3] = 5.0;

        @(posedge clock); #1;
        data_in[0] = 0.0;   data_in[1] = 0.0;   data_in[2] = 0.0;   data_in[3] = 1.0;

        @(posedge clock); #1;
        data_in[0] = 0.0;   data_in[1] = 0.0;   data_in[2] = 0.0;   data_in[3] = 0.0;

        #100;
        $finish;
    end

endmodule: MatUnit_test
