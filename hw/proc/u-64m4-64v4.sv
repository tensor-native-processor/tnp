`default_nettype none

module U_16M4_16U4
    #(parameter SWITCH_CORE_SIZE = 8,
                WIDTH = 64,

                MAT_CORE_COUNT = 4,
                VEC_CORE_COUNT = 4,

                // Auto-gen
                SWITCH_CORE_ADDR_SIZE = $clog2(SWITCH_CORE_SIZE)
    )
    ();

    logic clock, reset, done[SWITCH_CORE_SIZE-1:0];

    // Switch
    logic switch_send_ready[SWITCH_CORE_SIZE-1:0];
    logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_send_core_idx[SWITCH_CORE_SIZE-1:0];
    shortreal switch_send_data[SWITCH_CORE_SIZE-1:0][WIDTH-1:0];
    logic switch_send_ok[SWITCH_CORE_SIZE-1:0];
    logic switch_recv_request[SWITCH_CORE_SIZE-1:0];
    logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_recv_core_idx[SWITCH_CORE_SIZE-1:0];
    logic switch_recv_ready[SWITCH_CORE_SIZE-1:0];
    shortreal switch_recv_data[SWITCH_CORE_SIZE-1:0][WIDTH-1:0];

    MatCore #(
        .WIDTH(WIDTH),
        .SWITCH_WIDTH(WIDTH),
        .SWITCH_CORE_SIZE(SWITCH_CORE_SIZE),
        .CACHE_SIZE(256),
        .INST_MEM_SIZE(65536),
        .DATA_MEM_SIZE(65536)
	) MatDUT[MAT_CORE_COUNT-1:0](
        .clock, .reset,
        .done(done[MAT_CORE_COUNT-1:0]),
        .switch_send_ready(switch_send_ready[MAT_CORE_COUNT-1:0]),
        .switch_send_core_idx(switch_send_core_idx[MAT_CORE_COUNT-1:0]),
        .switch_send_data(switch_send_data[MAT_CORE_COUNT-1:0]),
        .switch_send_ok(switch_send_ok[MAT_CORE_COUNT-1:0]),
        .switch_recv_request(switch_recv_request[MAT_CORE_COUNT-1:0]),
        .switch_recv_core_idx(switch_recv_core_idx[MAT_CORE_COUNT-1:0]),
        .switch_recv_ready(switch_recv_ready[MAT_CORE_COUNT-1:0]),
        .switch_recv_data(switch_recv_data[MAT_CORE_COUNT-1:0])
    );

    VecCore #(
        .WIDTH(WIDTH),
        .SWITCH_WIDTH(WIDTH),
        .SWITCH_CORE_SIZE(SWITCH_CORE_SIZE),
        .CACHE_SIZE(256),
        .INST_MEM_SIZE(65536),
        .DATA_MEM_SIZE(65536)
	) VecDUT[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT](
        .clock, .reset,
        .done(done[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT]),
        .switch_send_ready(switch_send_ready[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT]),
        .switch_send_core_idx(switch_send_core_idx[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT]),
        .switch_send_data(switch_send_data[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT]),
        .switch_send_ok(switch_send_ok[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT]),
        .switch_recv_request(switch_recv_request[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT]),
        .switch_recv_core_idx(switch_recv_core_idx[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT]),
        .switch_recv_ready(switch_recv_ready[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT]),
        .switch_recv_data(switch_recv_data[VEC_CORE_COUNT+MAT_CORE_COUNT-1:MAT_CORE_COUNT])
    );

    Switch #(.WIDTH(WIDTH), .CORE_SIZE(SWITCH_CORE_SIZE)) switch(
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
        #10000000 $finish;
    end
    
    integer i, data_mem_file;
    initial begin
        // Load instruction/data memory
        $readmemb("inst_mem0.txt", MatDUT[0].inst_mem.inst_mem);
        $readmemb("inst_mem1.txt", MatDUT[1].inst_mem.inst_mem);
        $readmemb("inst_mem2.txt", MatDUT[2].inst_mem.inst_mem);
        $readmemb("inst_mem3.txt", MatDUT[3].inst_mem.inst_mem);
        $readmemb("inst_mem4.txt", VecDUT[4].inst_mem.inst_mem);
        $readmemb("inst_mem5.txt", VecDUT[5].inst_mem.inst_mem);
        $readmemb("inst_mem6.txt", VecDUT[6].inst_mem.inst_mem);
        $readmemb("inst_mem7.txt", VecDUT[7].inst_mem.inst_mem);

        data_mem_file = $fopen("data_mem0.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", MatDUT[0].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem1.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", MatDUT[1].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem2.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", MatDUT[2].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem3.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", MatDUT[3].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem4.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", VecDUT[4].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem5.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", VecDUT[5].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem6.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", VecDUT[6].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem7.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", VecDUT[7].data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);


        // Reset control
        reset = 1;
        #10 reset = 0;

        wait (done[0] == 1);
        wait (done[1] == 1);
        wait (done[2] == 1);
        wait (done[3] == 1);
        wait (done[4] == 1);
        wait (done[5] == 1);
        wait (done[6] == 1);
        wait (done[7] == 1);
        #10;

        data_mem_file = $fopen("output0.txt", "w");
        for (i = 0;i < MatDUT[0].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", MatDUT[0].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output1.txt", "w");
        for (i = 0;i < MatDUT[1].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", MatDUT[1].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output2.txt", "w");
        for (i = 0;i < MatDUT[2].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", MatDUT[2].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output3.txt", "w");
        for (i = 0;i < MatDUT[3].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", MatDUT[3].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output4.txt", "w");
        for (i = 0;i < VecDUT[4].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", VecDUT[4].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output5.txt", "w");
        for (i = 0;i < VecDUT[5].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", VecDUT[5].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output6.txt", "w");
        for (i = 0;i < VecDUT[6].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", VecDUT[6].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output7.txt", "w");
        for (i = 0;i < VecDUT[7].DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", VecDUT[7].data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);


        $finish;
    end

endmodule: U_16M4_16U4
