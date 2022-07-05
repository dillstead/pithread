#ifndef __UART_H
#define __UART_H

void uart_init(void);
void uart_putc(char c);
void uart_puts(const char *s);

#endif
