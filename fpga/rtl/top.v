module top( input clk,
				input button,
				input [7:0] sw,
				output reg [7:0] leds,
				input rx,
				output tx,
				output reg [3:0] box_1,
				output reg [3:0] box_2,
				output reg [3:0] box_3
    );

wire rx_new_data;
wire [7:0] rx_data;

wire tx_done;
reg [7:0] tx_data;
reg tx_latch;				// tell uart to send byte

wire uart_rst;
wire uart_clk;

assign uart_rst = 1;		// keep out of reset

uart	uart(	.sys_clk(clk),
				.sys_rst_l(uart_rst),
				.uart_clk(uart_clk),

				// Transmitter
				.uart_XMIT_dataH(tx),
				.xmitH(tx_latch),
				.xmit_dataH(tx_data),
				.xmit_doneH(tx_done),

				// Receiver
				.uart_REC_dataH(rx),
				.rec_dataH(rx_data),
				.rec_readyH(rx_new_data)		
			);
			
always @ (posedge rx_new_data) begin
	leds <= rx_data;
	box_1 <= rx_data[3:0];
	box_2 <= rx_data[7:4];
	box_3 <= 0;
end

reg flag = 0;

always @ (posedge uart_clk) begin

	if (tx_done) begin
		tx_data <= sw;						// send switch data
		tx_latch <= 1;
		flag <= 0;
	end

	if (flag == 1) begin
		flag <= 0;
		tx_latch <= 0;
	end
	
	flag <= flag + 1;
end


endmodule
