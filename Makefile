TOOLCHAIN = ../gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi
CC = $(TOOLCHAIN)-gcc
GDB = $(TOOLCHAIN)-gdb
OBJDUMP = $(TOOLCHAIN)-objdump
OBJCOPY = $(TOOLCHAIN)-objcopy
CFLAGS = -mcpu=cortex-a7 -ffreestanding -g -Wall -Wextra -Wno-unused-value -mgeneral-regs-only
#CFLAGS += -DCOPY_ATAGS
LDFLAGS = -nostdlib -nostartfiles -lgcc
LIB_DIR = lib
SRCS = $(wildcard *.c)
ASM_SRCS = $(wildcard *.S)
LIB_SRCS = $(wildcard $(LIB_DIR)/*.c)
ASM_LIB_SRCS = $(wildcard $(LIB_DIR)/*.S)
HDRS = $(wildcard *.h)
HDRS += $(wildcard $(LIB_DIR)/*.h)
OBJS = $(patsubst %.c, %.o, $(SRCS))
OBJS += $(patsubst %.S, %.o, $(ASM_SRCS))
OBJS += $(patsubst $(LIB_DIR)/%.c, $(LIB_DIR)/%.o, $(LIB_SRCS))
OBJS += $(patsubst $(LIB_DIR)/%.S, $(LIB_DIR)/%.o, $(ASM_LIB_SRCS))
INCS = -I. -I$(LIB_DIR)
IMG_NAME=kernel7

build: $(OBJS) $(HDRS)
	$(CC) -T linker.ld -o $(IMG_NAME).elf $(OBJS) $(LDFLAGS)
	$(OBJCOPY) $(IMG_NAME).elf -O binary $(IMG_NAME).img
	$(OBJDUMP) -d -S $(IMG_NAME).elf > $(IMG_NAME).asm

$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

$(LIB_DIR)/%.o: $(LIB_DIR)/%.S
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@ 

%.o: %.S
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
	rm *.img *.elf *.asm *.o *.bin $(LIB_DIR)/*o 

run: build
	qemu-system-arm -m 128 -no-reboot -machine raspi2 -serial null -serial stdio -kernel kernel7.elf

dbg:
	$(GDB) kernel7.elf

dbgrun: build gdbinit
	qemu-system-arm -m 128 -no-reboot -machine raspi2 -serial null -serial stdio -kernel kernel7.elf -S -s

gdbinit:
	echo "target remote localhost:1234" > .gdbinit
