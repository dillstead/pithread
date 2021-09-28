#include <string.h>
#include <uart.h>
#include <mmio.h>
#include <peripheral.h>

#define UART0_OFFSET  0x201000
#define GPIO_OFFSET   0x200000

union uart_flags
{
    struct
    {
        uint8_t clear_to_send: 1;
        uint8_t data_set_ready: 1;
        uint8_t data_carrier_detected: 1;
        uint8_t busy: 1;
        uint8_t recieve_queue_empty: 1;
        uint8_t transmit_queue_full: 1;
        uint8_t recieve_queue_full: 1;
        uint8_t transmit_queue_empty: 1;
        uint8_t ring_indicator: 1;
        uint32_t padding: 23;
    };
    uint32_t as_int;
};

union uart_control
{
    struct
    {
        uint8_t uart_enabled: 1;
        uint8_t sir_enabled: 1;
        uint8_t sir_low_power_mode: 1;
        uint8_t reserved: 4;
        uint8_t loop_back_enabled: 1;
        uint8_t transmit_enabled: 1;
        uint8_t receive_enabled: 1;
        uint8_t data_transmit_ready: 1;
        uint8_t request_to_send: 1;
        uint8_t out1: 1;
        uint8_t out2: 1;
        uint8_t rts_hardware_flow_control_enabled: 1;
        uint8_t cts_hardware_flow_control_enabled: 1;
        uint16_t padding;
    };
    uint32_t as_int;
};

enum
{
    GPIO_BASE    = PERIPHERAL_BASE + GPIO_OFFSET,
    // Controls actuation of pull up/down to ALL GPIO pins.
    GPPUD        = GPIO_BASE + 0x94,
    // Controls actuation of pull up/down for specific GPIO pin.
    GPPUDCLK0    = GPIO_BASE + 0x98,

    UART0_BASE   = PERIPHERAL_BASE + UART0_OFFSET,
    // The offsets for reach register for the UART.
    UART0_DR     = UART0_BASE + 0x00,
    UART0_RSRECR = UART0_BASE + 0x04,
    UART0_FR     = UART0_BASE + 0x18,
    UART0_ILPR   = UART0_BASE + 0x20,
    UART0_IBRD   = UART0_BASE + 0x24,
    UART0_FBRD   = UART0_BASE + 0x28,
    UART0_LCRH   = UART0_BASE + 0x2C,
    UART0_CR     = UART0_BASE + 0x30,
    UART0_IFLS   = UART0_BASE + 0x34,
    UART0_IMSC   = UART0_BASE + 0x38,
    UART0_RIS    = UART0_BASE + 0x3C,
    UART0_MIS    = UART0_BASE + 0x40,
    UART0_ICR    = UART0_BASE + 0x44,
    UART0_DMACR  = UART0_BASE + 0x48,
    UART0_ITCR   = UART0_BASE + 0x80,
    UART0_ITIP   = UART0_BASE + 0x84,
    UART0_ITOP   = UART0_BASE + 0x88,
    UART0_TDR    = UART0_BASE + 0x8C,
};

static union uart_flags read_flags(void)
{
    union uart_flags flags;
    
    flags.as_int = mmio_read(UART0_FR);
    return flags;
}

void uart_init(void)
{
    union uart_control control;
    
    // Disable UART0.
    memset(&control, 0, sizeof control);
    mmio_write(UART0_CR, control.as_int);

    // Setup the GPIO pin 14 && 15.
    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    mmio_write(GPPUD, 0x00000000);
    delay(150);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    mmio_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    mmio_write(GPPUDCLK0, 0x00000000);

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
    control.uart_enabled = 1;
    control.transmit_enabled = 1;
    control.receive_enabled = 1;
    mmio_write(UART0_CR, control.as_int);
}

void uart_putc(char c)
{
    // Wait for UART to become ready to transmit.
    union uart_flags flags;

    do
    {
        flags = read_flags();
    }
    while (flags.transmit_queue_full);
    mmio_write(UART0_DR, c);
}

unsigned char uart_getc(void)
{
    // Wait for UART to have received something.
    union uart_flags flags;
    
    do
    {
        flags = read_flags();
    }
    while (flags.recieve_queue_empty);
    return mmio_read(UART0_DR);
}
