module data_ctrl #(
  parameter SAMPLE_WIDTH = 12
)(
  
  // system clock (25 MHz)
  input  wire        i_clk,
  
  // from trigger_generator_inst
  input  wire        i_trigger_pulse,
  
  // uart_tx handshake signals
  input  wire        i_tx_done,
  
  // from signal_frame_inst
  input  wire        i_sample_valid,
  input  wire [SAMPLE_WIDTH-1:0] i_sample_data,
  input  wire        i_capture_done,
  input  wire        i_RESET,
  
  // to signal_frame_inst
  output reg         o_rd_en,
  
  // uart_tx_inst handshake signals
  output reg  [7:0]  o_tx_data,
  output reg         o_tx_en,
  
  // signal to top level for synchronous reset
  output reg         o_transfer_done,
  
  // simulation/debug probes
  output reg  [2:0]  o_state
);

  // State definitions
  localparam s_IDLE     = 3'b000;
  localparam s_REQUEST  = 3'b001;
  localparam s_LOWER    = 3'b010;
  localparam s_UPPER    = 3'b011;
  localparam s_WAIT_TX  = 3'b100;
  localparam s_DONE     = 3'b101;

  reg [SAMPLE_WIDTH-1:0] r_sample_reg;
  reg byte_sel;  // 0 = LS, 1 = MS

  always @(posedge i_clk) 
  begin
  
    if (i_RESET) 
    begin
      
      // Synchronous reset: set all state and outputs to known values
      r_sample_reg    <= 0;
      byte_sel        <= 0;
      o_state         <= s_IDLE;
      o_tx_data       <= 0;
      o_tx_en         <= 0;
      o_rd_en         <= 0;
      o_transfer_done <= 0;
      
    end 
    else 
    begin
      
      case (o_state)
        
        s_IDLE: 
        begin
          
          o_transfer_done <= 0;
          o_tx_en <= 0;
          o_rd_en <= 0;
          byte_sel <= 0;

          if (i_trigger_pulse) 
          begin
            
            o_state <= s_REQUEST;
            o_rd_en <= 1;
            
          end
          
        end

        s_REQUEST: 
        begin
          
          o_rd_en <= 0;
          
          if (i_sample_valid) 
          begin
            
            r_sample_reg <= i_sample_data;
            o_state <= s_LOWER;
            
          end
          
        end

        s_LOWER: 
        begin
          
          o_tx_data <= r_sample_reg[7:0];
          o_tx_en   <= 1;
          o_state   <= s_WAIT_TX;
          byte_sel  <= 0;
          
        end

        s_UPPER: 
        begin
          
          o_tx_data <= {4'b0000, r_sample_reg[11:8]};
          o_tx_en   <= 1;
          o_state   <= s_WAIT_TX;
          byte_sel  <= 1;
          
        end

        s_WAIT_TX: 
        begin
          
          o_tx_en <= 0;
          if (i_tx_done) 
          begin
            
            if (byte_sel == 0) 
            begin
              
              o_state <= s_UPPER;
              
            end 
            else 
            begin
              
              if (i_capture_done) 
              begin
                
                o_state <= s_DONE;
                
              end 
              else 
              begin
               
                o_rd_en <= 1;
                o_state <= s_REQUEST;
                
              end
              
            end
            
          end
          
        end

        s_DONE: 
        begin
        
          o_transfer_done <= 1;
          o_state <= s_IDLE;
          
        end

        default: 
        begin  
          
          o_state <= s_IDLE;
          
        end
        
      endcase
      
    end
    
  end

endmodule
