module datapath (

  input         i_clk,
  input         i_response_valid,
  input         i_trigger,
  input [11:0]  i_sample_data,
  input         i_RESET,
  
  output        o_TRANSFER_DONE,
  output        o_pc_serial

);


  // PC communication module

  pc_comm #(
    .SAMPLE_WIDTH(12),
    .DEPTH(1024),
    .ADDR_WIDTH(10)
  ) pc_comm_inst (
    .i_clk(i_clk),
    .i_tx_clk(i_clk),             // UART can run asynchronously if desired
    .i_response_valid(i_response_valid),
    .i_trigger(i_trigger),
    .i_transfer_data(i_sample_data),
    .i_RESET(i_RESET),
    .o_TRANSFER_DONE(o_TRANSFER_DONE),
    .o_tx_serial(o_pc_serial)
  );

endmodule
