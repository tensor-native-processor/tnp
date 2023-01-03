`default_nettype none

module MatCoreSimVerilator
    #(parameter SWITCH_CORE_SIZE = 4,
                SWITCH_WIDTH = 16,

                INST_MEM_SIZE = 65536,
                DATA_MEM_SIZE = 65536,

                // Auto-gen
                SWITCH_CORE_ADDR_SIZE = $clog2(SWITCH_CORE_SIZE)
    )
    (input logic clock, reset,
     output logic done);

    // Switch
    logic switch_send_ready;
    logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_send_core_idx;
    real switch_send_data[SWITCH_WIDTH-1:0];
    logic switch_send_ok;
    logic switch_recv_request;
    logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_recv_core_idx;
    logic switch_recv_ready;
    real switch_recv_data[SWITCH_WIDTH-1:0];

    MatCore #(
        .SWITCH_CORE_SIZE(SWITCH_CORE_SIZE),
        .SWITCH_WIDTH(SWITCH_WIDTH),
        .INST_MEM_SIZE(INST_MEM_SIZE),
        .DATA_MEM_SIZE(DATA_MEM_SIZE)
	) DUT(.*);

    // Write memory
    export "DPI-C" function mat_core_write_output;
    function void mat_core_write_output();
        data_mem_file = $fopen("output.txt", "w");
        for (i = 0;i < DUT.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", DUT.data_mem.data_mem[i]);
        end
    endfunction

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
    end

endmodule: MatCoreSimVerilator
