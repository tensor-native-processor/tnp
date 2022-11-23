`default_nettype none

module SwitchReceiver
    #(parameter WIDTH = 32,
                CORE_SIZE = 4,

                // Auto-generated sizes
                CORE_ADDR_SIZE = $clog2(CORE_SIZE))
    (input logic clock, reset,

     // Receiver signal
     input logic recv_request,
     input logic [CORE_ADDR_SIZE-1:0] recv_core_idx,
     output logic recv_ready,
     output shortreal recv_data[WIDTH-1:0],

     // Read from transit
     output logic [CORE_ADDR_SIZE-1:0] transit_addr,
     input logic transit_empty,
     output logic transit_pop,
     input shortreal transit_data[WIDTH-1:0]
    );

    // Registers
    logic [CORE_ADDR_SIZE-1:0] reg_core_idx;
    logic reg_load;

    // Load at reg_load
    always_ff @(posedge clock) begin
        if (reg_load) begin
            reg_core_idx <= recv_core_idx;
        end
    end


    // State machine
    enum {
        IDLE, WAIT_TRANSIT
    } state, next_state;

    always_ff @(posedge clock) begin
        if (reset)
            state <= IDLE;
        else
            state <= next_state;
    end

    // Multiplexer into transit_data
    enum {
        RECV_DATA_FROM_ZERO,
        RECV_DATA_FROM_TRANSIT_DATA
    } recv_data_sel;
    genvar i;
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (recv_data_sel)
                RECV_DATA_FROM_ZERO: recv_data[i] = 0;
                RECV_DATA_FROM_TRANSIT_DATA: recv_data[i] = transit_data[i];
                endcase
            end
        end
    endgenerate

    // Set next state
    always_comb begin
        // Receiver
        recv_ready = 0;
        recv_data_sel = RECV_DATA_FROM_ZERO;

        // Transit
        transit_addr = 0;
        transit_pop = 0;

        // Register
        reg_load = 0;

        unique case (state)
        IDLE: begin
            if (~recv_request) begin
                next_state = IDLE;
            end else begin
                // Set transit address
                transit_addr = recv_core_idx;
                if (~transit_empty) begin
                    next_state = IDLE;

                    // Pop from transit
                    transit_pop = 1;

                    // Signal receiver ready
                    recv_ready = 1;
                    recv_data_sel = RECV_DATA_FROM_TRANSIT_DATA;
                end else begin
                    next_state = WAIT_TRANSIT;

                    reg_load = 1;
                end
            end
        end

        WAIT_TRANSIT: begin
            // Set transit address
            transit_addr = reg_core_idx;
            if (~transit_empty) begin
                next_state = IDLE;

                // Pop from transit
                transit_pop = 1;

                // Signal receiver ready
                recv_ready = 1;
                recv_data_sel = RECV_DATA_FROM_TRANSIT_DATA;
            end else begin
                next_state = WAIT_TRANSIT;
            end
        end

        endcase
    end

endmodule: SwitchReceiver
