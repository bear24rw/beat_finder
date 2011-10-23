#ifndef SERIAL_H
#define SERIAL_H

#define BAUD     2400

int init_serial(void);
void send_serial_fpga(void);
void send_serial_table(void);

#endif
