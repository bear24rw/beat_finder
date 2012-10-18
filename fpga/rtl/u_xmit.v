

//
// U_XMIT.v
//
// This is the asynchronous transmitter
// portion of the UART.
//

module u_xmit(	sys_clk,
				sys_rst_l,

				uart_xmitH,
				xmitH,
				xmit_dataH,
				xmit_doneH
			);


`include "inc.h"

// ******************************************
//
// PORT DEFINITIONS
//
// ******************************************
input			sys_clk;	// system clock. Must be 16 x Baud
input			sys_rst_l;	// asynch reset

output			uart_xmitH;	// this pin goes to the connector
input			xmitH;		// active high, Xmit command
input	[7:0]	xmit_dataH;	// data to be xmitted
output			xmit_doneH;	// status



// ******************************************
//
// MEMORY ELEMENT DEFINITIONS
//
// ******************************************
reg		[2:0]	next_state, state;
reg				load_shiftRegH;
reg				shiftEnaH;
reg		[4:0]	bitCell_cntrH;
reg				countEnaH;
reg		[7:0]	xmit_ShiftRegH;
reg		[3:0]	bitCountH;
reg				rst_bitCountH;
reg				ena_bitCountH;
reg		[1:0]	xmitDataSelH;
reg				uart_xmitH;
reg				xmit_doneInH;
reg				xmit_doneH;


always @(xmit_ShiftRegH or xmitDataSelH)
  case (xmitDataSelH)
	x_STARTbit: uart_xmitH = LO;
	x_STOPbit:  uart_xmitH = HI;
	x_ShiftReg: uart_xmitH = xmit_ShiftRegH[0];
	default:    uart_xmitH = X;	
  endcase


//
// Bit Cell time Counter
//
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) bitCell_cntrH <= 0;
  else if (countEnaH) bitCell_cntrH <= bitCell_cntrH + 1;
  else bitCell_cntrH <= 0;



//
// Shift Register
//
// The LSB must be shifted out first
//
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) xmit_ShiftRegH <= 0;
  else 
	if (load_shiftRegH) xmit_ShiftRegH <= xmit_dataH;
	else if (shiftEnaH) begin
		xmit_ShiftRegH[6:0] <= xmit_ShiftRegH[7:1];
		xmit_ShiftRegH[7]   <= HI;
	end else xmit_ShiftRegH <= xmit_ShiftRegH;



//
// Transmitted bit counter
//
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) bitCountH <= 0;
  else if (rst_bitCountH) bitCountH <= 0;
  else if (ena_bitCountH) bitCountH <= bitCountH + 1;


//
// STATE MACHINE
//

// State Variable
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) state <= x_IDLE;
  else state <= next_state;


// Next State, Output Decode
always @(state or xmitH or bitCell_cntrH or bitCountH)
begin
   
	// Defaults
	next_state 		= state;
	load_shiftRegH	= LO;
	countEnaH       = LO;
	shiftEnaH       = LO;
	rst_bitCountH   = LO;
	ena_bitCountH   = LO;
    xmitDataSelH    = x_STOPbit;
	xmit_doneInH	= LO;

	case (state)
    	
		//
		// x_IDLE
		// wait for the start command
		//
		x_IDLE: begin
			if (xmitH) begin 
                next_state = x_START;
				load_shiftRegH = HI;
			end else begin
				next_state    = x_IDLE;
				rst_bitCountH = HI; 
                xmit_doneInH  = HI;
			end
		end
  


		//
		// x_START
		// send start bit 
		//
		x_START: begin
            xmitDataSelH    = x_STARTbit;
			if (bitCell_cntrH == 4'hF)
				next_state = x_WAIT;
			else begin 
				next_state = x_START;
				countEnaH  = HI; // allow to count up
			end				
		end


		//
		// x_WAIT
		// wait 1 bit-cell time before sending
		// data on the xmit pin
		//
		x_WAIT: begin
            xmitDataSelH    = x_ShiftReg;
			// 1 bit-cell time wait completed
			if (bitCell_cntrH == 4'hE) begin
				if (bitCountH == WORD_LEN)
					next_state = x_STOP;
				else begin
					next_state = x_SHIFT;
					ena_bitCountH = HI; //1more bit sent
				end
			// bit-cell wait not complete
			end else begin
				next_state = x_WAIT;
				countEnaH  = HI;
			end		
		end



		//
		// x_SHIFT
		// shift out the next bit
		//
		x_SHIFT: begin
            xmitDataSelH    = x_ShiftReg;
			next_state = x_WAIT;
			shiftEnaH  = HI; // shift out next bit
		end


		//
		// x_STOP
		// send stop bit
		//
		x_STOP: begin
            xmitDataSelH    = x_STOPbit;
			if (bitCell_cntrH == 4'hF) begin
				next_state   = x_IDLE;
                xmit_doneInH = HI;
			end else begin
				next_state = x_STOP;
				countEnaH = HI; //allow bit cell cntr
			end
		end



		default: begin
			next_state     = 3'bxxx;
			load_shiftRegH = X;
			countEnaH      = X;
            shiftEnaH      = X;
            rst_bitCountH  = X;
            ena_bitCountH  = X;
            xmitDataSelH   = 2'bxx;
            xmit_doneInH   = X;
		end

    endcase

end


// register the state machine outputs
// to eliminate ciritical-path/glithces
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) xmit_doneH <= 0;
  else xmit_doneH <= xmit_doneInH;


endmodule
