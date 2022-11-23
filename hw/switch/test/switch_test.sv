`default_nettype none

// Testbench for switch
module Switch_test
    #(parameter WIDTH = 2,
                CORE_SIZE = 3,

                CORE_ADDR_SIZE = $clog2(CORE_SIZE)
    )
    ();

    logic clock, reset;

    logic send_ready[CORE_SIZE-1:0];
    logic [CORE_ADDR_SIZE-1:0] send_core_idx[CORE_SIZE-1:0];
    shortreal send_data[CORE_SIZE-1:0][WIDTH-1:0];
    logic send_ok[CORE_SIZE-1:0];

    logic recv_request[CORE_SIZE-1:0];
    logic [CORE_ADDR_SIZE-1:0] recv_core_idx[CORE_SIZE-1:0];
    logic recv_ready[CORE_SIZE-1:0];
    shortreal recv_data[CORE_SIZE-1:0][WIDTH-1:0];

    Switch #(.WIDTH(WIDTH), .CORE_SIZE(CORE_SIZE)) DUT(.*);

    // Clock
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end

    // Tests
    initial begin
        send_ready[0] = 0;
        send_ready[1] = 0;
        send_ready[2] = 0;
        send_core_idx[0] = 0;
        send_core_idx[1] = 0;
        send_core_idx[2] = 0;
        recv_request[0] = 0;
        recv_request[1] = 0;
        recv_request[2] = 0;
        recv_core_idx[0] = 0;
        recv_core_idx[1] = 0;
        recv_core_idx[2] = 0;

        // Reset state machines
        reset = 1;
        #10 reset = 0;

        @(posedge clock);#1;
        send_ready[2] = 1;
        send_core_idx[2] = 1;
        send_data[2][0] = 11;
        send_data[2][1] = 13;

        @(posedge clock);#1;
        send_ready[2] = 0;
        @(posedge clock);#1;
        @(posedge clock);#1;
        @(posedge clock);#1;
        recv_request[1] = 1;
        recv_core_idx[1] = 2;

        @(posedge clock);#1;
        recv_request[1] = 0;
        

        #100 $finish;
    end


endmodule: Switch_test
