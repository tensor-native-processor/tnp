`default_nettype none

// Top-level module for switch
module Switch
    #(parameter WIDTH = 32,
                CORE_SIZE = 4,

                // Auto-generated sizes
                CORE_ADDR_SIZE = $clog2(CORE_SIZE))
    (input logic clock, reset,

     // Senders
     input logic send_ready[CORE_SIZE-1:0],
     input logic [CORE_ADDR_SIZE-1:0] send_core_idx[CORE_SIZE-1:0],
     input shortreal send_data[CORE_SIZE-1:0][WIDTH-1:0],
     output logic send_ok[CORE_SIZE-1:0],

     // Receiver
     input logic recv_request[CORE_SIZE-1:0],
     input logic [CORE_ADDR_SIZE-1:0] recv_core_idx[CORE_SIZE-1:0],
     output logic recv_ready[CORE_SIZE-1:0],
     output shortreal recv_data[CORE_SIZE-1:0][WIDTH-1:0]
    );

    // Sender transit
    logic [CORE_ADDR_SIZE-1:0] send_transit_addr[CORE_SIZE-1:0];
    logic send_transit_empty[CORE_SIZE-1:0]; // Output
    logic send_transit_push[CORE_SIZE-1:0];
    shortreal send_transit_data[CORE_SIZE-1:0][WIDTH-1:0];

    // Receiver transit
    logic [CORE_ADDR_SIZE-1:0] recv_transit_addr[CORE_SIZE-1:0];
    logic recv_transit_empty[CORE_SIZE-1:0]; // Output
    logic recv_transit_pop[CORE_SIZE-1:0];
    shortreal recv_transit_data[CORE_SIZE-1:0][WIDTH-1:0]; // Output

    // Instantiate sender/receiver
    SwitchSender #(.WIDTH(WIDTH), .CORE_SIZE(CORE_SIZE)) senders[CORE_SIZE-1:0](
        .clock, .reset,
        .send_ready, .send_core_idx, .send_data, .send_ok,
        .transit_addr(send_transit_addr), .transit_empty(send_transit_empty),
        .transit_push(send_transit_push), .transit_data(send_transit_data)
    );
    SwitchReceiver #(.WIDTH(WIDTH), .CORE_SIZE(CORE_SIZE)) receivers[CORE_SIZE-1:0](
        .clock, .reset,
        .recv_request, .recv_core_idx, .recv_ready, .recv_data,
        .transit_addr(recv_transit_addr), .transit_empty(recv_transit_empty),
        .transit_pop(recv_transit_pop), .transit_data(recv_transit_data)
    );

    // Transit registers
    shortreal reg_transit_data[CORE_SIZE-1:0][CORE_SIZE-1:0][WIDTH-1:0];
    logic reg_transit_empty[CORE_SIZE-1:0][CORE_SIZE-1:0];

    // Assign empty
    genvar core, sender, receiver;
    generate
        for (core = 0;core < CORE_SIZE;core++) begin
            always_comb begin
                send_transit_empty[core] = reg_transit_empty[core][send_transit_addr[core]];
            end
            always_comb begin
                recv_transit_empty[core] = reg_transit_empty[recv_transit_addr[core]][core];
            end
        end
    endgenerate

    // Manage registers
    generate
        for (sender = 0;sender < CORE_SIZE;sender++)
        for (receiver = 0;receiver < CORE_SIZE;receiver++) begin
            always_ff @(posedge clock) begin
                if (reset) begin
                    reg_transit_empty[sender][receiver] = 1;
                end else begin
                    // Push operation
                    if (reg_transit_empty[sender][receiver] &&
                            send_transit_addr[sender] == receiver &&
                            send_transit_push[sender]) begin
                        reg_transit_data[sender][receiver] <= send_transit_data[sender];
                        reg_transit_empty[sender][receiver] <= 0;
                    end
                    // Pop operation
                    if (~reg_transit_empty[sender][receiver] &&
                            recv_transit_addr[receiver] == sender &&
                            recv_transit_pop[receiver]) begin
                        reg_transit_empty[sender][receiver] <= 1;
                    end
                end
            end
        end
    endgenerate

    // Output pop data
    generate
        for (receiver = 0;receiver < CORE_SIZE;receiver++) begin
            always_comb begin
                recv_transit_data[receiver] = 
                    reg_transit_data[recv_transit_addr[receiver]][receiver];
            end
        end
    endgenerate

endmodule: Switch
