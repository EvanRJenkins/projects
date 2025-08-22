`timescale 1ns/1ns
module uart_tb();


  // arbitrary, don't want simulation to be too long
  parameter CLOCKS_PER_BIT = 2;

  wire w_tx_bit;
  wire w_tx_done;
  wire w_rx_dv;
  wire [7:0] w_rx_byte;
  
  reg tb_clk;
  reg [7:0] tb_byte;
  reg tb_dv;


  always #10 tb_clk <= !tb_clk;
  
  
  always @(w_rx_dv)
  begin
    
    tb_byte <= tb_byte - 1'b1;
    #20;
    tb_dv <= 1;
    #10;
    tb_dv <= 0;
    
  end
  
  initial
  begin
    
    tb_clk = 0;
    tb_byte = 8'h3F;
    tb_dv = 0;
    #10;
    tb_dv <= 1;
    #10;
    tb_dv <= 0;

    forever
    begin
    
      if (w_rx_byte == 8'h00)
      begin
      
        $display("testbench passed");
        $finish;
        
      end
      else #10;
    
    end
  
  end
  
  

  
  
  
uart_tx #(
  .CLOCKS_PER_BIT(CLOCKS_PER_BIT)
  ) uart_tx_inst (
    .i_clk(tb_clk),
    .i_tx_data(tb_byte),
    .i_tx_data_valid(tb_dv),
    .o_tx_done(w_tx_done),
    .o_tx_bit(w_tx_bit)
    );
    
    
    
uart_rx #(
  .CLOCKS_PER_BIT(CLOCKS_PER_BIT)
  ) uart_rx_inst (
    .i_clk(tb_clk),
    .i_rx_bit(w_tx_bit),
    .o_rx_data_valid(w_rx_dv),
    .o_rx_byte(w_rx_byte)
    );
  
endmodule