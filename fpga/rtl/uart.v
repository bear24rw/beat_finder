
// *********************************************
//
// UART.v
//
// www.cmosexod.com
// 4/13/2001 (c) 2001
// Jeung Joon Lee
// 
// Universal Asyhnchronous Receiver, Transmitter
// This is a reduced version of the UART.
// It is fully functional, synthesizable, ideal
// for embedded system.
//
// Update Log:
// 7/30/01  The 'bitCell_cntrH' compare in r_WAIT 
//          state of u_rec.v has been changed from
//          the incorrect F to E.
//          The 'bitCell_cntrH' compare in x_WAIT 
//          state of u_xmit.v has been changed from
//          the incorrect F to E.
// 
//
// SYNPLIFY SYNTHESYS REPORT (typical)
//
// Resource Usage Report
//
// Simple gate primitives:
// FD21            52 uses
// XOR2            26 uses
// AND2            117 uses
// IB11            12 uses
// OB11            12 uses
// INV             61 uses
// MUX2            25 uses
//
// *********************************************

module uart	(	sys_clk,
				sys_rst_l,
				uart_clk,

				// Transmitter
				uart_XMIT_dataH,
				xmitH,
				xmit_dataH,
				xmit_doneH,

				// Receiver
				uart_REC_dataH,
				rec_dataH,
				rec_readyH		
			);


`include "inc.h"				

input			sys_clk;
input			sys_rst_l;
output			uart_clk;

// Trasmitter
output			uart_XMIT_dataH;
input			xmitH;
input	[7:0]	xmit_dataH;
output			xmit_doneH;

// Receiver
input			uart_REC_dataH;
output	[7:0]	rec_dataH;
output			rec_readyH;

wire 			uart_clk;
wire	[7:0]	rec_dataH;
wire			rec_readyH;



// Instantiate the Transmitter
u_xmit  iXMIT(  .sys_clk(uart_clk),
				.sys_rst_l(sys_rst_l),

				.uart_xmitH(uart_XMIT_dataH),
				.xmitH(xmitH),
				.xmit_dataH(xmit_dataH),
				.xmit_doneH(xmit_doneH)
			);


// Instantiate the Receiver


u_rec iRECEIVER (// system connections
				.sys_rst_l(sys_rst_l),
				.sys_clk(uart_clk),

				// uart
				.uart_dataH(uart_REC_dataH),

				.rec_dataH(rec_dataH),
				.rec_readyH(rec_readyH)

				);


// Instantiate the Baud Rate Generator

baud iBAUD(	.sys_clk(sys_clk),
			.sys_rst_l(sys_rst_l),		
			.baud_clk(uart_clk)
		);



endmodule
