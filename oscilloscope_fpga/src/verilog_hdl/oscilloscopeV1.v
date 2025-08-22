module oscilloscopeV1 (

  // clock into adc ip pll
  input             MAX10_CLK1_50,
  
  // de10-lite peripherals
  input    [1: 0]   KEY,
  input    [9: 0]   SW,
  output   [9: 0]   LEDR,

  // tx pin to pc
  output   [1:0]    GPIO
  
);

wire reset_n;  // ignore, adc ip reset
wire sys_clk;  // 25 MHz clk out from adc ip, whole system uses this

assign reset_n = 1'b1;                // ignore, adc ip reset

wire KEY1_n;                          // optional for manual trigger
assign KEY1_n = !KEY[1];              // optional for manual trigger

wire  command_valid;                  // held high to always enable adc sampling
wire  [4:0] command_channel;          // channel select with SW[2:0]
wire  command_startofpacket;          // ignore
wire  command_endofpacket;            // ignore
wire  command_ready;                  // ignore

// continused send command
assign command_startofpacket = 1'b1;  // ignore
assign command_endofpacket = 1'b1;    // ignore
assign command_valid = 1'b1;          // ignore
assign command_channel = SW[2:0]+1;   // SW2/SW1/SW0 down is channel 1

wire [4:0]   response_channel;
wire [11:0]  response_data;           // binary sample data from adc
wire         RESPONSE_VALID;
wire         response_startofpacket;  // ignore
wire         response_endofpacket;    // ignore
reg  [4:0]   cur_adc_ch;              // currently unused
reg  [11:0]  adc_sample_data;          // debug

wire w_TRIGGER;
reg [12:0] test;
wire RESET;
wire TRANSFER_DONE;

adc_qsys adc_qsys_inst
(
	.clk_clk                              (MAX10_CLK1_50),          //  clk.clk
  .reset_reset_n                        (reset_n),                //  reset.reset_n
  .modular_adc_0_command_valid          (command_valid),          //  modular_adc_0_command.valid
  .modular_adc_0_command_channel        (command_channel),        //  .channel
  .modular_adc_0_command_startofpacket  (command_startofpacket),  //  .startofpacket
  .modular_adc_0_command_endofpacket    (command_endofpacket),    //  .endofpacket
  .modular_adc_0_command_ready          (command_ready),          //  .ready
  .modular_adc_0_response_valid         (RESPONSE_VALID),         //  modular_adc_0_response.valid
  .modular_adc_0_response_channel       (response_channel),       //  .channel
  .modular_adc_0_response_data          (response_data),          //  .data
  .modular_adc_0_response_startofpacket (response_startofpacket), //  .startofpacket
  .modular_adc_0_response_endofpacket   (response_endofpacket),   //  .endofpacket
  .clock_bridge_sys_out_clk_clk         (sys_clk)                 //  clock_bridge_sys_out_clk.clk
);


datapath datapath_inst
(
	.i_clk(sys_clk) ,	// input  i_clk_sig
	.i_response_valid(RESPONSE_VALID) ,	// input  i_response_valid_sig
	.i_trigger(w_TRIGGER) ,	// input  i_trigger_sig
  .i_RESET(RESET) ,
	.i_sample_data(response_data) ,	// input  i_adc_sample_sig
	.o_TRANSFER_DONE(TRANSFER_DONE),
  .o_pc_serial(GPIO[0]) 	// output  o_pc_serial_sig
);


// this is only for visual testing, doesnt impact functionality
always @ (posedge sys_clk)
begin

	if (RESPONSE_VALID)
	begin
  
		adc_sample_data <= response_data;
		cur_adc_ch <= response_channel;
		test <= response_data * 2;
    
	end
  
end


assign LEDR[9:0] = test[12:3];  // approximate adc reading prescale


trigger_generator trigger_generator_inst 
(
  .i_clk(sys_clk),
  .i_response_valid(RESPONSE_VALID),
  .i_sample_data(response_data),
  .TRANSFER_DONE(TRANSFER_DONE),
  .o_TRIGGER(w_TRIGGER),
  .o_RESET(RESET)
);



endmodule