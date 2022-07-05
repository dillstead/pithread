#ifndef __AUX_H
#define __AUX_H

#include "peripheral.h"

#define AUX_BASE           (PERIPHERAL_BASE + 0x215000)
// The offsets for each register for the mini UART
#define AUX_ENABLES        (AUX_BASE + 0x04)
#define AUX_MU_IO_REG      (AUX_BASE + 0x40)
#define AUX_MU_IER_REG     (AUX_BASE + 0x44)
#define AUX_MU_IIR_REG     (AUX_BASE + 0x48)
#define AUX_MU_LCR_REG     (AUX_BASE + 0x4C)
#define AUX_MU_MCR_REG     (AUX_BASE + 0x50)
#define AUX_MU_LSR_REG     (AUX_BASE + 0x54)
#define AUX_MU_MSR_REG     (AUX_BASE + 0x58)
#define AUX_MU_SCRATCH_REG (AUX_BASE + 0x5C)
#define AUX_MU_CNTL_REG    (AUX_BASE + 0x60)
#define AUX_MU_STAT_REG    (AUX_BASE + 0x64)
#define AUX_MU_BAUD_REG    (AUX_BASE + 0x68)

#endif
