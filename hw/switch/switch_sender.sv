`default_nettype none

module SwitchSender
    #(parameter WIDTH = 32,
                CORE_SIZE = 4,

                // Auto-generated sizes
                CORE_ADDR_SIZE = $clog2(CORE_SIZE))
    (input logic clock, reset,

     // Sender signal
     input logic send_ready,
     input logic [CORE_ADDR_SIZE-1:0] send_core_idx,
     input shortreal send_data[WIDTH-1:0],
     output logic send_ok,

     // Write into transit
     output logic [CORE_ADDR_SIZE-1:0] transit_addr,
     input logic transit_empty,
     output logic transit_push,
     output shortreal transit_data[WIDTH-1:0]
    );

    // Registers
    logic [CORE_ADDR_SIZE-1:0] reg_core_idx;
    shortreal reg_data[WIDTH-1:0];
    logic reg_load;

    // Load at reg_load
    always_ff @(posedge clock) begin
        if (reg_load) begin
            reg_core_idx <= send_core_idx;
            reg_data <= send_data;
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
        TRANSIT_DATA_FROM_ZERO,
        TRANSIT_DATA_FROM_SEND_DATA,
        TRANSIT_DATA_FROM_REG_DATA
    } transit_data_sel;
    genvar i;
    generate
        for (i = 0;i < WIDTH;i++) begin
            always_comb begin
                unique case (transit_data_sel)
                TRANSIT_DATA_FROM_ZERO: transit_data[i] = 0;
                TRANSIT_DATA_FROM_SEND_DATA: transit_data[i] = send_data[i];
                TRANSIT_DATA_FROM_REG_DATA: transit_data[i] = reg_data[i];
                endcase
            end
        end
    endgenerate

    // Set next state
    always_comb begin
        // Send
        send_ok = 0;

        // Transit
        transit_addr = 0;
        transit_push = 0;
        transit_data_sel = TRANSIT_DATA_FROM_ZERO;

        // Register
        reg_load = 0;

        unique case (state)
        IDLE: begin
            if (~send_ready) begin
                next_state = IDLE;
            end else begin
                // Set transit address
                transit_addr = send_core_idx;
                if (transit_empty) begin
                    next_state = IDLE;

                    // Push data into transit
                    transit_push = 1;
                    transit_data_sel = TRANSIT_DATA_FROM_SEND_DATA;
                    send_ok = 1;
                end else begin
                    next_state = WAIT_TRANSIT;

                    reg_load = 1;
                end
            end
        end
        WAIT_TRANSIT: begin
            // Set transit address
            transit_addr = reg_core_idx;
            if (transit_empty) begin
                next_state = IDLE;

                // Push data into transit
                transit_push = 1;
                transit_data_sel = TRANSIT_DATA_FROM_REG_DATA;
                send_ok = 1;
            end else begin
                next_state = WAIT_TRANSIT;
            end
        end
        endcase
    end

endmodule: SwitchSender
