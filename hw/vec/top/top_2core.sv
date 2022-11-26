`default_nettype none

module VecCoreSim
    #(parameter SWITCH_CORE_SIZE = 2,
                SWITCH_WIDTH = 16,

                // Auto-gen
                SWITCH_CORE_ADDR_SIZE = $clog2(SWITCH_CORE_SIZE)
    )
    ();

    logic clock, reset, done[1:0];

    // Switch
    logic switch_send_ready[SWITCH_CORE_SIZE-1:0];
    logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_send_core_idx[SWITCH_CORE_SIZE-1:0];
    shortreal switch_send_data[SWITCH_CORE_SIZE-1:0][SWITCH_WIDTH-1:0];
    logic switch_send_ok[SWITCH_CORE_SIZE-1:0];
    logic switch_recv_request[SWITCH_CORE_SIZE-1:0];
    logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_recv_core_idx[SWITCH_CORE_SIZE-1:0];
    logic switch_recv_ready[SWITCH_CORE_SIZE-1:0];
    shortreal switch_recv_data[SWITCH_CORE_SIZE-1:0][SWITCH_WIDTH-1:0];

    VecCore #(
        .SWITCH_WIDTH(SWITCH_WIDTH),
        .SWITCH_CORE_SIZE(SWITCH_CORE_SIZE)
    ) DUT[SWITCH_CORE_SIZE-1:0](.*);

    Switch #(.WIDTH(SWITCH_WIDTH), .CORE_SIZE(SWITCH_CORE_SIZE)) switch(
        .clock, .reset,
        .send_ready(switch_send_ready),
        .send_core_idx(switch_send_core_idx),
        .send_data(switch_send_data),
        .send_ok(switch_send_ok),
        .recv_request(switch_recv_request),
        .recv_core_idx(switch_recv_core_idx),
        .recv_ready(switch_recv_ready),
        .recv_data(switch_recv_data)
    );

    // Clock signal
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end

    // Timeout
    initial begin
        #10000 $finish;
    end
    
    integer i, data_mem_file;

    initial begin
        // Load instruction/data memory
        $readmemb("inst_mem0.txt", DUT[0].inst_mem.inst_mem);
        data_mem_file = $fopen("data_mem0.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", DUT[0].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        $readmemb("inst_mem1.txt", DUT[1].inst_mem.inst_mem);
        data_mem_file = $fopen("data_mem1.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", DUT[1].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        // Reset control
        reset = 1;
        #10 reset = 0;

        wait (done[0] == 1);
        wait (done[1] == 1);

        #10;
        data_mem_file = $fopen("output0.txt", "w");
        for (i = 0;i < DUT[0].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file,
                "%f\n", DUT[0].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output1.txt", "w");
        for (i = 0;i < DUT[1].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file,
                "%f\n", DUT[1].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        $finish;
    end

endmodule: VecCoreSim
