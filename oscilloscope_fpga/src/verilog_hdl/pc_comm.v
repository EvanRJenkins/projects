module pc_comm #(
  parameter SAMPLE_WIDTH = 12,
  parameter DEPTH        = 2048,   // Number of samples to capture on trigger
  parameter ADDR_WIDTH   = 11
)(
  input                     i_clk,              // System Clock (25 MHz)
  input                     i_tx_clk,           // UART clock
  input                     i_response_valid,   // sample ready from ADC
  input                     i_trigger,          // trigger pulse
  input  [SAMPLE_WIDTH-1:0] i_transfer_data,    // ADC sample input
  input        i_RESET,

  output                    o_TRANSFER_DONE,    // call for system reset
  output                    o_tx_serial         // UART output
);
  
  wire                    w_sample_valid;
  wire [SAMPLE_WIDTH-1:0] w_sample_data;
  wire                    w_capture_done;
  wire                    w_rd_en;
  wire [7:0]              w_tx_data;
  wire                    w_tx_en;
  wire                    w_tx_done;
  
  
  // Circular buffer and sample memory
  
  signal_frame #(
    .SAMPLE_WIDTH(SAMPLE_WIDTH),
    .DEPTH(DEPTH),
    .ADDR_WIDTH(ADDR_WIDTH)
  ) signal_frame_inst (
    .i_clk(i_clk),
    .i_sample_valid(i_response_valid),
    .i_sample_data(i_transfer_data),
    .i_trigger(i_trigger),
    .i_RESET(i_RESET),

    .i_rd_en(w_rd_en),
    .o_sample_valid(w_sample_valid),
    .o_sample_data(w_sample_data),
    .o_capture_done(w_capture_done)
  );
 
 
  // Data and communication controller
 
  data_ctrl #(
    .SAMPLE_WIDTH(SAMPLE_WIDTH)
  ) data_ctrl_inst (
    .i_clk(i_clk),
    .i_trigger_pulse(i_trigger),
    .i_tx_done(w_tx_done),

    // wires from signal_frame_inst
    .i_sample_valid(w_sample_valid),
    .i_sample_data(w_sample_data),
    .i_capture_done(w_capture_done),
    
    // Synchronous reset
    .i_RESET(i_RESET),

    // wires to signal_frame_inst
    .o_rd_en(w_rd_en),

    // wires to uart_tx_inst
    .o_tx_data(w_tx_data),
    .o_tx_en(w_tx_en),

    // signal to reset system after transfer
    .o_transfer_done(o_TRANSFER_DONE),
    
    // simulation/debug probes
    .o_state()
  );


  // UART transmitter

  uart_tx #(
  .CLOCKS_PER_BIT(326)
  ) uart_tx_inst (
    
    // this clock can be asynchronous
    .i_clk(i_tx_clk),
    
    // wires from data_ctrl_inst
    .i_tx_data(w_tx_data),
    .i_tx_data_valid(w_tx_en),
    
    // handshake signal to data_ctrl (allows asynchronous operation)
    .o_tx_done(w_tx_done),
    
    // wires to top level
    .o_tx_bit(o_tx_serial),
    
    // simulation/debug probes
    .o_tx_active()
  );

endmodule
