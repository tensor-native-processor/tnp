`default_nettype none

module MatUnit
    #(parameter WIDTH = 128)
    (input logic mode,
     input logic clock,
     input shortreal sin[WIDTH - 1 : 0],
     output shortreal sout[WIDTH - 1 : 0]);


    // Sum, weight and pass register for each element
    shortreal sum[WIDTH : 0][WIDTH : 0];
    shortreal weight[WIDTH : 0][WIDTH : 0];
    shortreal pass[WIDTH : 0][WIDTH : 0];

    genvar i, j;

    // Main matrix elements
    generate

        for (i = 1;i <= WIDTH;i++) begin
            for (j = 1;j <= WIDTH;j++) begin

                always_ff @(posedge clock) begin
                    sum[i][j] <= sum[i - 1][j] +
                            pass[i][j - 1] * weight[i][j];
                    pass[i][j] <= pass[i][j - 1];
                end

            end
        end

    endgenerate


    // Input for rows
    generate
        for (i = 1;i <= WIDTH;i++) begin
            assign sum[i][0] = sin[i - 1];
        end
    endgenerate

    // Zero and output for columns
    generate
        for (j = 1;j <= WIDTH;j++) begin
            assign sum[0][j] = 1'b0;
            assign sout[j - 1] = sum[WIDTH][j];
        end
    endgenerate

endmodule: MatUnit
