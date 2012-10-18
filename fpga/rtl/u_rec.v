

//
// U_REC.v
//
// www.cmosexod.com
// 4/13/2001 (c) 2001
// Jeung Joon Lee
//
// This is the receiver portion of the UART.
//
//

module u_rec (	// system connections
				sys_rst_l,
				sys_clk,

				// uart
				uart_dataH,

				//
				rec_dataH,
				rec_readyH

				);


`include "inc.h"

// ******************************************
//
// PORT DEFINITIONS
//
// ******************************************
input			sys_rst_l;	// async reset
input			sys_clk;	// main clock must be 16 x Baud Rate

input			uart_dataH;	// goes to the UART pin

output	[7:0]	rec_dataH;	// parallel received data
output			rec_readyH;	// when high, new data is ok to be read


// ******************************************
//
// MEMORY ELEMENT DEFINITIONS
//
// ******************************************

reg		[2:0]	next_state, state;
reg				rec_datH, rec_datSyncH;
reg		[3:0]	bitCell_cntrH;
reg				cntr_resetH;
reg		[7:0]	par_dataH;
reg				shiftH;
reg		[3:0]	recd_bitCntrH;
reg				countH;
reg				rstCountH;
reg				rec_readyH;
reg				rec_readyInH;


wire	[7:0]	rec_dataH;


assign rec_dataH = par_dataH;

//
// synchronize the asynchrnous input
// to the system clock domain
// dual-rank
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) begin
     rec_datSyncH <= 1;
     rec_datH     <= 1;
  end else begin
     rec_datSyncH <= uart_dataH;
     rec_datH     <= rec_datSyncH;
  end


// Bit-cell counter
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) bitCell_cntrH <= 0;
  else if (cntr_resetH) bitCell_cntrH <= 0;
  else bitCell_cntrH <= bitCell_cntrH + 1;


// Shifte Register to hold the incoming 
// serial data
// LSB is shifted in first
//
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) par_dataH <= 0;
  else if(shiftH) begin
     par_dataH[6:0] <= par_dataH[7:1];
     par_dataH[7]   <= rec_datH;
  end


// RECEIVED BIT Counter
// This coutner keeps track of the number of
// bits received
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) recd_bitCntrH <= 0;
  else if (countH) recd_bitCntrH <= recd_bitCntrH + 1;
  else if (rstCountH) recd_bitCntrH <= 0;




// State Machine - Next State Assignment
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) state <= r_START;
  else state <= next_state;


// State Machine - Next State and Output Decode
always @(state or rec_datH or bitCell_cntrH or recd_bitCntrH)
begin

  // default
  next_state  = state;
  cntr_resetH = HI;
  shiftH      = LO;
  countH      = LO;
  rstCountH   = LO;
  rec_readyInH= LO;

  case (state)
     
    //
    // START
    // Wait for the start bit
    // 
    r_START: begin
       if (~rec_datH ) next_state = r_CENTER;
       else begin 
         next_state = r_START;
         rstCountH  = HI; // place the bit counter in rst state
         rec_readyInH = HI; // by default, we're ready
       end
    end

    //
	// CENTER
	// Find the center of the bit-cell 
	// A bit cell is composed of 16 system-clock 
	// ticks
	//
    r_CENTER: begin
       if (bitCell_cntrH == 4'h4) begin
         // if after having waited 1/2 bit cell,
 		 // it is still 0, then it is a genuine start bit
         if (~rec_datH) next_state = r_WAIT;
		 // otherwise, could have been a false noise
         else next_state = r_START;
       end else begin
         next_state  = r_CENTER;
		 cntr_resetH = LO;  // allow counter to tick          
       end
    end


    //
	// WAIT
	// Wait a bit-cell time before sampling the
	// state of the data pin
	//
	r_WAIT: begin
		if (bitCell_cntrH == 4'hE) begin
           if (recd_bitCntrH == WORD_LEN)
             next_state = r_STOP; // we've sampled all 8 bits
           else begin
             next_state = r_SAMPLE;
           end
        end else begin
             next_state  = r_WAIT;
             cntr_resetH = LO;  // allow counter to tick 
        end
    end

    // 
	// SAMPLE
	// Sample the state of the RECEIVE data pin 
   	//
	r_SAMPLE: begin
		shiftH = HI; // shift in the serial data
		countH = HI; // one more bit received
		next_state = r_WAIT;
	end	


    // 
    // STOP
    // make sure that we've seen the stop
    // bit
    //
    r_STOP: begin
		next_state = r_START;
        rec_readyInH = HI;
    end

    default: begin
       next_state  = 3'bxxx;
       cntr_resetH = X;
	   shiftH      = X;
	   countH      = X;
       rstCountH   = X;
       rec_readyInH  = X;

    end

  endcase


end


// register the state machine outputs
// to eliminate ciritical-path/glithces
always @(posedge sys_clk or negedge sys_rst_l)
  if (~sys_rst_l) rec_readyH <= 0;
  else rec_readyH <= rec_readyInH;




endmodule
