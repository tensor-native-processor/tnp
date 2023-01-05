`default_nettype none

module U_16M4_16U4
    #(parameter SWITCH_CORE_SIZE = 8,
                SWITCH_WIDTH = 16,

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
    shortreal switch_send_data[SWITCH_CORE_SIZE-1:0][SWITCH_WIDTH-1:0];
    logic switch_send_ok[SWITCH_CORE_SIZE-1:0];
    logic switch_recv_request[SWITCH_CORE_SIZE-1:0];
    logic [SWITCH_CORE_ADDR_SIZE-1:0] switch_recv_core_idx[SWITCH_CORE_SIZE-1:0];
    logic switch_recv_ready[SWITCH_CORE_SIZE-1:0];
    shortreal switch_recv_data[SWITCH_CORE_SIZE-1:0][SWITCH_WIDTH-1:0];

    MatCore #(
        .SWITCH_WIDTH(SWITCH_WIDTH),
        .SWITCH_CORE_SIZE(SWITCH_CORE_SIZE),
        .CACHE_SIZE(256),
        .INST_MEM_SIZE(65536),
        .DATA_MEM_SIZE(65536)
	)
    MatDUT0(
        .clock, .reset,
        .done(done[0]),
        .switch_send_ready(switch_send_ready[0]),
        .switch_send_core_idx(switch_send_core_idx[0]),
        .switch_send_data(switch_send_data[0]),
        .switch_send_ok(switch_send_ok[0]),
        .switch_recv_request(switch_recv_request[0]),
        .switch_recv_core_idx(switch_recv_core_idx[0]),
        .switch_recv_ready(switch_recv_ready[0]),
        .switch_recv_data(switch_recv_data[0])
    ),
    MatDUT1(
        .clock, .reset,
        .done(done[1]),
        .switch_send_ready(switch_send_ready[1]),
        .switch_send_core_idx(switch_send_core_idx[1]),
        .switch_send_data(switch_send_data[1]),
        .switch_send_ok(switch_send_ok[1]),
        .switch_recv_request(switch_recv_request[1]),
        .switch_recv_core_idx(switch_recv_core_idx[1]),
        .switch_recv_ready(switch_recv_ready[1]),
        .switch_recv_data(switch_recv_data[1])
    ),
    MatDUT2(
        .clock, .reset,
        .done(done[2]),
        .switch_send_ready(switch_send_ready[2]),
        .switch_send_core_idx(switch_send_core_idx[2]),
        .switch_send_data(switch_send_data[2]),
        .switch_send_ok(switch_send_ok[2]),
        .switch_recv_request(switch_recv_request[2]),
        .switch_recv_core_idx(switch_recv_core_idx[2]),
        .switch_recv_ready(switch_recv_ready[2]),
        .switch_recv_data(switch_recv_data[2])
    ),
    MatDUT3(
        .clock, .reset,
        .done(done[3]),
        .switch_send_ready(switch_send_ready[3]),
        .switch_send_core_idx(switch_send_core_idx[3]),
        .switch_send_data(switch_send_data[3]),
        .switch_send_ok(switch_send_ok[3]),
        .switch_recv_request(switch_recv_request[3]),
        .switch_recv_core_idx(switch_recv_core_idx[3]),
        .switch_recv_ready(switch_recv_ready[3]),
        .switch_recv_data(switch_recv_data[3])
    );
    VecCore #(
        .SWITCH_WIDTH(SWITCH_WIDTH),
        .SWITCH_CORE_SIZE(SWITCH_CORE_SIZE),
        .CACHE_SIZE(256),
        .INST_MEM_SIZE(65536),
        .DATA_MEM_SIZE(65536)
	)
    VecDUT4(
        .clock, .reset,
        .done(done[4]),
        .switch_send_ready(switch_send_ready[4]),
        .switch_send_core_idx(switch_send_core_idx[4]),
        .switch_send_data(switch_send_data[4]),
        .switch_send_ok(switch_send_ok[4]),
        .switch_recv_request(switch_recv_request[4]),
        .switch_recv_core_idx(switch_recv_core_idx[4]),
        .switch_recv_ready(switch_recv_ready[4]),
        .switch_recv_data(switch_recv_data[4])
    ),
    VecDUT5(
        .clock, .reset,
        .done(done[5]),
        .switch_send_ready(switch_send_ready[5]),
        .switch_send_core_idx(switch_send_core_idx[5]),
        .switch_send_data(switch_send_data[5]),
        .switch_send_ok(switch_send_ok[5]),
        .switch_recv_request(switch_recv_request[5]),
        .switch_recv_core_idx(switch_recv_core_idx[5]),
        .switch_recv_ready(switch_recv_ready[5]),
        .switch_recv_data(switch_recv_data[5])
    ),
    VecDUT6(
        .clock, .reset,
        .done(done[6]),
        .switch_send_ready(switch_send_ready[6]),
        .switch_send_core_idx(switch_send_core_idx[6]),
        .switch_send_data(switch_send_data[6]),
        .switch_send_ok(switch_send_ok[6]),
        .switch_recv_request(switch_recv_request[6]),
        .switch_recv_core_idx(switch_recv_core_idx[6]),
        .switch_recv_ready(switch_recv_ready[6]),
        .switch_recv_data(switch_recv_data[6])
    ),
    VecDUT7(
        .clock, .reset,
        .done(done[7]),
        .switch_send_ready(switch_send_ready[7]),
        .switch_send_core_idx(switch_send_core_idx[7]),
        .switch_send_data(switch_send_data[7]),
        .switch_send_ok(switch_send_ok[7]),
        .switch_recv_request(switch_recv_request[7]),
        .switch_recv_core_idx(switch_recv_core_idx[7]),
        .switch_recv_ready(switch_recv_ready[7]),
        .switch_recv_data(switch_recv_data[7])
    );

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
        #10000000 $finish;
    end
    
    integer i, data_mem_file;
    initial begin
        // Load instruction/data memory
        $readmemb("inst_mem0.txt", MatDUT0.inst_mem.inst_mem);
        $readmemb("inst_mem1.txt", MatDUT1.inst_mem.inst_mem);
        $readmemb("inst_mem2.txt", MatDUT2.inst_mem.inst_mem);
        $readmemb("inst_mem3.txt", MatDUT3.inst_mem.inst_mem);
        $readmemb("inst_mem4.txt", VecDUT4.inst_mem.inst_mem);
        $readmemb("inst_mem5.txt", VecDUT5.inst_mem.inst_mem);
        $readmemb("inst_mem6.txt", VecDUT6.inst_mem.inst_mem);
        $readmemb("inst_mem7.txt", VecDUT7.inst_mem.inst_mem);

        data_mem_file = $fopen("data_mem0.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", MatDUT0.data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem1.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", MatDUT1.data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem2.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", MatDUT2.data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem3.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", MatDUT3.data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem4.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", VecDUT4.data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem5.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", VecDUT5.data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem6.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", VecDUT6.data_mem.data_mem[i]) == 1) begin
            i++;
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("data_mem7.txt", "r");
        i = 0;
        while ($fscanf(data_mem_file, "%f", VecDUT7.data_mem.data_mem[i]) == 1) begin
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
        for (i = 0;i < MatDUT0.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", MatDUT0.data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output1.txt", "w");
        for (i = 0;i < MatDUT1.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", MatDUT1.data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output2.txt", "w");
        for (i = 0;i < MatDUT2.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", MatDUT2.data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output3.txt", "w");
        for (i = 0;i < MatDUT3.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", MatDUT3.data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output4.txt", "w");
        for (i = 0;i < VecDUT4.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", VecDUT4.data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output5.txt", "w");
        for (i = 0;i < VecDUT5.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", VecDUT5.data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output6.txt", "w");
        for (i = 0;i < VecDUT6.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", VecDUT6.data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);

        data_mem_file = $fopen("output7.txt", "w");
        for (i = 0;i < VecDUT7.DATA_MEM_SIZE;i++) begin
            $fwrite(data_mem_file, "%f\n", VecDUT7.data_mem.data_mem[i]);
        end
        $fclose(data_mem_file);


        $display("Finished at time %0t", $time);
        $finish;
    end

endmodule: U_16M4_16U4
