`default_nettype none

module VecCoreSim();

    logic clock, reset, done;

    VecCore DUT(.*);

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
