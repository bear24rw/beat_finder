
//
// Receiver state definition
//
parameter	r_START 	= 3'b001,
          	r_CENTER	= 3'b010,
          	r_WAIT  	= 3'b011,
          	r_SAMPLE	= 3'b100,
		  	r_STOP  	= 3'b101;

//
// Xmitter state definition
//
parameter	x_IDLE		= 3'b000,
			x_START		= 3'b010,
			x_WAIT		= 3'b011,
			x_SHIFT		= 3'b100,
			x_STOP		= 3'b101;


parameter   x_STARTbit  = 2'b00,
			x_STOPbit   = 2'b01,
			x_ShiftReg  = 2'b10;

//
// Common parameter Definition
//
parameter	LO 		= 1'b0,
          	HI		= 1'b1,		
 		  	X		= 1'bx;


// *****************************
//
// Receiver Configuration
//
// *****************************

// Word length.  
// This defines the number of bits 
// in a "word".  Typcially 8.
// min=0, max=8

parameter	WORD_LEN = 8;



//
// The xtal-osc clock freq
//
parameter XTAL_CLK = 100000000;

//
// The desired baud rate
//
parameter BAUD = 2400;

parameter CLK_DIV = XTAL_CLK / (BAUD * 16 * 2);

//
// CW >= log2(CLK_DIV)
//
parameter CW   = 11;


`define DEBUG
