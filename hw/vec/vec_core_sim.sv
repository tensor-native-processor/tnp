`default_nettype none

module VecCoreSim
    #(parameter SWITCH_CORE_SIZE = 4,
                SWITCH_WIDTH = 16,

                // Auto-gen
                SWITCH_CORE_ADDR_SIZE = $clog2(SWITCH_CORE_SIZE)
    )
    ();

    logic clock, reset, done;

    // Switch
    logic switch_send_ready;
    logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_send_core_idx;
    shortreal switch_send_data[SWITCH_WIDTH-1:0];
    logic switch_send_ok;
    logic switch_recv_request;
    logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_recv_core_idx;
    logic switch_recv_ready;
    shortreal switch_recv_data[SWITCH_WIDTH-1:0];

    VecCore #(
        .SWITCH_WIDTH(SWITCH_WIDTH),
        .SWITCH_CORE_SIZE(SWITCH_CORE_SIZE)
    ) DUT(.*);

    // Clock signal
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end
    
    integer i, data_mem_file;
    initial begin
        // Load instruction/data memory
        $readmemb("inst_mem.txt", DUT.inst_mem.inst_mem);
        data_mem_file = $fopen("data_mem.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", DUT.data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        // Reset control
        reset = 1;
        #10 reset = 0;

        @(posedge done);
        #10;
        data_mem_file = $fopen("output.txt", "w");
        for (i = 0;i < DUT.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", DUT.data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        $finish;
    end

endmodule: VecCoreSim
