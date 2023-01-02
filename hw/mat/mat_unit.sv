module MatUnit
    #(parameter WIDTH = 128,

                // Auto-generated sizes
                WIDTH_ADDR_SIZE = $clog2(WIDTH))
    (input logic clock,
     input logic set_weight,
     input logic [WIDTH_ADDR_SIZE-1:0] set_weight_row,
     input shortreal data_in[WIDTH-1:0],
     output shortreal data_out[WIDTH-1:0]);


    // Sum, weight and pass register for each element
    shortreal sum[WIDTH:0][WIDTH:0];
    shortreal weight[WIDTH:0][WIDTH:0];
    shortreal pass[WIDTH:0][WIDTH:0];

    genvar i, j;

    // Main matrix elements
    generate

        for (i = 1;i <= WIDTH;i++) begin
            for (j = 1;j <= WIDTH;j++) begin

                always_ff @(posedge clock) begin
                    sum[i][j] <= sum[i - 1][j] +
                            pass[i][j - 1] * weight[i][j];
                    pass[i][j] <= pass[i][j - 1];

                    if (set_weight && set_weight_row + 1 == i) begin
                        weight[i][j] <= pass[i][j - 1];
                    end
                end

            end
        end

    endgenerate


    // Input for rows
    generate
        for (i = 1;i <= WIDTH;i++) begin
            assign pass[i][0] = data_in[i - 1];
        end
    endgenerate

    // Zero and output for columns
    generate
        for (j = 1;j <= WIDTH;j++) begin
            assign sum[0][j] = 1'b0;
            assign data_out[j - 1] = sum[WIDTH][j];
        end
    endgenerate

endmodule: MatUnit
