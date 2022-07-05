#ifndef __GPIO_H
#define __GPIO_H

#include "peripheral.h"

#define GPIO_BASE (PERIPHERAL_BASE + 0x200000)
#define GPFSEL0   (GPIO_BASE + 0x00)
#define GPFSEL1   (GPIO_BASE + 0x04)
#define GPFSEL2   (GPIO_BASE + 0x08)
#define GPFSEL3   (GPIO_BASE + 0x0C)
#define GPFSEL4   (GPIO_BASE + 0x10)
#define GPFSEL5   (GPIO_BASE + 0x14)
#define GPSET0    (GPIO_BASE + 0x1C)
#define GPSET1    (GPIO_BASE + 0x20)
#define GPCLR0    (GPIO_BASE + 0x28)
#define GPCLR1    (GPIO_BASE + 0x2C)
#define GPPUD     (GPIO_BASE + 0x94)
#define GPPUDCLK0 (GPIO_BASE + 0x98)
#define GPPUDCLK1 (GPIO_BASE + 0x9C)
#endif
