module trigger_generator (

  input  wire        i_clk,
  input  wire        i_response_valid,
  input  wire [11:0] i_sample_data,
  input  wire        TRANSFER_DONE,
  
  output reg         o_TRIGGER,
  output reg         o_RESET

);

  // synchronous reset and trigger logic
  always @(posedge i_clk) 
  begin
    
    if (TRANSFER_DONE) 
    begin
      
      // reset when transfer is done
      o_RESET <= 1'b1;
      o_TRIGGER <= 1'b0;  // clear trigger on reset
      
    end 
    else 
    begin
      
      // turn off reset otherwise
      o_RESET <= 1'b0;
  
      // Trigger logic only active when not in reset
      if (i_response_valid) 
      begin
        
        if (i_sample_data > 12'h5DC)  // arbitrary trigger condition
        begin
          
          o_TRIGGER <= 1'b1;
          
        end 
        else 
        begin
          
          o_TRIGGER <= 1'b0;
          
        end
      end 
      else 
      begin
        
        o_TRIGGER <= 1'b0;
        
      end
      
    end
    
  end

endmodule
