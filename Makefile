TOOLCHAIN = ../gcc-arm-none-eabi-10-2020-q4-major/bin/arm-none-eabi
CC = $(TOOLCHAIN)-gcc
GDB = $(TOOLCHAIN)-gdb
OBJDUMP = $(TOOLCHAIN)-objdump
CFLAGS = -mcpu=cortex-a7 -ffreestanding -g -Wall -Wextra
CFLAGS += -DUSE_UART0
LDFLAGS = -nostdlib -nostartfiles -lgcc
LIB_DIR = lib
SRCS = $(wildcard *.c)
ASM_SRCS = $(wildcard *.S)
LIB_SRCS = $(wildcard $(LIB_DIR)/*.c)
HDRS = $(wildcard *.h)
HDRS += $(wildcard $(LIB_DIR)/*.h)
OBJS = $(patsubst %.c, %.o, $(SRCS))
OBJS += $(patsubst $(LIB_DIR)/%.c, $(LIB_DIR)/%.o, $(LIB_SRCS))
OBJS += $(patsubst %.S, %.o, $(ASM_SRCS))
INCS = -I. -I$(LIB_DIR)
IMG_NAME=kernel

build: $(OBJS) $(HDRS)
	$(CC) -T linker.ld -o $(IMG_NAME).elf $(OBJS) $(LDFLAGS)

$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@ 

%.o: %.S
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
	rm *.elf *.o $(LIB_DIR)/*o 

run: build
	qemu-system-arm -m 128 -no-reboot -machine raspi2 -serial stdio -kernel kernel.elf

objdump:
	$(OBJDUMP) -d -S kernel.elf

dbg:
	$(GDB) kernel.elf

dbgrun: build gdbinit
	qemu-system-arm -m 128 -no-reboot -machine raspi2 -serial stdio -kernel kernel.elf -S -s

gdbinit:
	echo "target remote localhost:1234" > .gdbinit
	echo "break kernel_main" >> .gdbinit
