`default_nettype none

module MatControl_test();

    logic clock, reset;

    MatControl DUT(.*);

    // Clock signal
    initial begin
        clock = 0;
        forever #5 clock = ~clock;
    end
    
    initial begin
        $readmemb("inst_mem.txt", DUT.inst_mem);
        reset = 1;
        #10 reset = 0;

        #1000 $finish;
    end

endmodule: MatControl_test
