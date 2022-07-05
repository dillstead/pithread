#include "peripheral.h"
#include "mmio.h"
#include "rand.h"

// There appears to be an undocumented random register on the BCM2835:
//   https://forums.raspberrypi.com/viewtopic.php?t=196015
#define RAND_BASE     (PERIPHERAL_BASE + 0x104000)
// The offsets for reach register for the RNG.
#define RAND_CTRL     (RAND_BASE + 0x00)
#define RAND_STATUS   (RAND_BASE + 0x04)
#define RAND_DATA     (RAND_BASE + 0x08)
#define RAND_INT_MASK (RAND_BASE + 0x10)

void rand_init(void)
{
    mmio_write(RAND_STATUS, 0x40000);
    // Mask interrupt
    mmio_write(RAND_INT_MASK, mmio_read(RAND_INT_MASK) | 1);
    // Enable
    mmio_write(RAND_CTRL, mmio_read(RAND_CTRL) | 1);
}
    
/**
 * Return a random number between [min..max]
 */
unsigned int rand(unsigned int min, unsigned int max)
{
    // May need to wait for entropy: bits 24-31 store how many words are
    // available for reading; require at least one
    while (!(mmio_read(RAND_STATUS) >> 24));
    return mmio_read(RAND_DATA) % (max - min + 1) + min;
}
