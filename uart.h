#ifndef __UART_H
#define __UART_H

void uart_init(void);
void uart_putc(char c);
unsigned char uart_getc(void);

#endif
