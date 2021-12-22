#include <string.h>
#ifdef USE_UART0
#include <uart0.h>
#else
#include <aux.h>
#endif
#include <gpio.h>
#include <uart.h>
#include <mmio.h>

void uart_init(void)
{
#ifdef USE_UART0
    uint32_t selector;

    // Set alt0 for pins 14 and 15, TXD0 and RXD0
    // respectively 
    selector = mmio_read(GPFSEL1);
    selector &= ~(7 << 12);
    selector |= 4 << 12;
    selector &= ~(7 << 15);
    selector |= 4 << 15;
    mmio_write(GPFSEL1, selector);
    
    // Disable pull up/down for pin 14 and 15
    mmio_write(GPPUD, 0);
    delay(150);
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    // Write 0 to GPPUDCLK0 to make it take effect
    mmio_write(GPPUDCLK0, 0);

    // Disable UART0
    mmio_write(UART0_CR, 0);
    // Clear pending interrupts.
    mmio_write(UART0_ICR, 0x7FF);    
    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.
    // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
    mmio_write(UART0_IBRD, 1);
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    mmio_write(UART0_FBRD, 40);
    // Enable FIFO & 8 bit data transmissio (1 stop bit, no parity).
    mmio_write(UART0_LCRH, (1 << 4) | (1 << 5) | (1 << 6));
    // Mask all interrupts.
    mmio_write(UART0_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) |
               (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));
    // Enable UART0, receive & transfer part of UART.
    mmio_write(UART0_CR, (3 << 8) | 1);
#else
    uint32_t selector;

    // Set alt5 for pins 14 and 15, TXD1 and RXD1
    // respectively 
    selector = mmio_read(GPFSEL1);
    selector &= ~(7 << 12);
    selector |= 2 << 12;
    selector &= ~(7 << 15);
    selector |= 2 << 15;
    mmio_write(GPFSEL1, selector);
    
    // Disable pull up/down for pin 14 and 15
    mmio_write(GPPUD, 0);
    delay(150);
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);
    // Write 0 to GPPUDCLK0 to make it take effect
    mmio_write(GPPUDCLK0, 0);
    
    // Enable the mini UART    
    mmio_write(AUX_ENABLES, 1);
    // Disable auto flow control and disable receiver and transmitter 
    mmio_write(AUX_MU_CNTL_REG, 0);
    // Disable receive and transmit interrupts
    mmio_write(AUX_MU_IER_REG, 0);
    // Enable 8-bit mode
    mmio_write(AUX_MU_LCR_REG, 3);
    // Set RTS line low
    mmio_write(AUX_MU_MCR_REG, 2);
    // Clear both FIFOs
    mmio_write(AUX_MU_IIR_REG, 0xC6);
    // Set baud rate to 115200
    mmio_write(AUX_MU_BAUD_REG, 270);
    // Enable transmitter
    mmio_write(AUX_MU_CNTL_REG, 2);
#endif    
}

void uart_putc(char c)
{
    // Wait for UART to become ready to transmit.
#ifdef USE_UART0
    while (mmio_read(UART0_FR) & 0x20);
    mmio_write(UART0_DR, c);
#else
    while (!(mmio_read(AUX_MU_LSR_REG) & 0x20));
    mmio_write(AUX_MU_IO_REG, c);
#endif    
}

