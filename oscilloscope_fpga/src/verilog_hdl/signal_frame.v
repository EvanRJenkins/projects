module signal_frame #(
  parameter SAMPLE_WIDTH = 12,
  parameter DEPTH        = 1024,
  parameter ADDR_WIDTH   = 10
)(
  // system clock (25 MHz)
  input                         i_clk,

  // from adc_qsys_inst
  input                         i_sample_valid,
  input      [SAMPLE_WIDTH-1:0] i_sample_data,

  // from trigger_generator
  input                         i_trigger,
  input                         i_RESET,

  // from data_ctrl_inst
  input                         i_rd_en,
  
  // to data_ctrl_inst
  output reg                    o_sample_valid,
  output reg [SAMPLE_WIDTH-1:0] o_sample_data,
  output reg                    o_capture_done
);

  // circular buffer
  reg [SAMPLE_WIDTH-1:0] mem [0:DEPTH-1];

  reg [ADDR_WIDTH-1:0] wr_ptr;
  reg [ADDR_WIDTH-1:0] rd_ptr;
  reg [ADDR_WIDTH-1:0] trig_ptr;

  reg capturing;
  reg triggered;
  reg [ADDR_WIDTH:0] samples_read; // counter for read samples


  // main sequence
  always @(posedge i_clk) 
  begin
    
    if (i_RESET) 
    begin
      
      // synchronous reset
      wr_ptr         <= 0;
      rd_ptr         <= 0;
      trig_ptr       <= 0;
      capturing      <= 0;
      triggered      <= 0;
      samples_read   <= 0;
      o_sample_valid <= 0;
      o_sample_data  <= 0;
      o_capture_done <= 0;
    
    end 
    else 
    begin
    
      // write
      if (i_sample_valid) 
      begin
        
        mem[wr_ptr] <= i_sample_data;
        wr_ptr <= wr_ptr + 1'b1;
        
      end

      // trigger & capture
      if (i_trigger && !triggered) 
      begin
        
        triggered     <= 1;
        capturing     <= 1;
        trig_ptr      <= wr_ptr;
        rd_ptr        <= wr_ptr;
        samples_read  <= 0;
        o_capture_done <= 0;
        
      end 
      else if (capturing) 
      begin
        
        if (i_rd_en) 
        begin
        
          o_sample_data  <= mem[rd_ptr];
          o_sample_valid <= 1;
          rd_ptr         <= rd_ptr + 1'b1;
          samples_read   <= samples_read + 1'b1;
          
        end 
        else 
        begin
          
          o_sample_valid <= 0;
          
        end

        // stop capture when full
        if (samples_read == DEPTH) 
        begin
          
          capturing      <= 0;
          o_capture_done <= 1;
        end
        
      end 
      else 
      begin
        
        o_sample_valid <= 0;
        
      end
      
    end
    
  end

endmodule
