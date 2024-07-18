CROSS = m68k-xelf-
CC = $(CROSS)gcc
#AS = $(CROSS)as
AS = run68 /Users/ohnaka/work/XEiJ/HFS/HAS/HAS060.X
LD = $(CROSS)gcc

CFLAGS = -g -std=gnu90 -c -m68000 -O2 -finput-charset=CP932
CFLAGS_DEBUG = -g -std=gnu90 -c -m68000 -finput-charset=CP932
LDFLAGS = -lm -lbas -liocs -ldos

#GCC_OPTS = -c -O -g -finput-charset=CP932

#LD = m68k-xelf-ld.x
#LD_OPTS = -L /Users/ohnaka/work/XEiJ/HFS/XGCC/LIB/

ASFLAGS = -i /Users/ohnaka/work/XEiJ/HFS/XGCC/INCLUDE/ -w0
ASFLAGS_DEBUG = -d -i /Users/ohnaka/work/XEiJ/HFS/XGCC/INCLUDE/ -w0

SRC_DIR = src
BUILD_DIR = build
EXE_DIR = exe

all: copy_to_target_all

debug: copy_to_target_debug

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

copy_to_target_all: copy_to_target copy_to_target_debug

copy_to_target: ${BUILD_DIR} ${BUILD_DIR}/ms.x
	cp ${BUILD_DIR}/ms.x ${EXE_DIR}/
	cp ${BUILD_DIR}/ms.x.elf ${EXE_DIR}/
	cp ${BUILD_DIR}/ms.x /Users/ohnaka/work/XEiJ/HFS/MS.X/

copy_to_target_debug: ${BUILD_DIR} ${BUILD_DIR}/ms_debug.x
	cp ${BUILD_DIR}/ms_debug.x ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_debug.x.elf ${EXE_DIR}/
	cp ${BUILD_DIR}/ms_debug.x /Users/ohnaka/work/XEiJ/HFS/MS.X/

${BUILD_DIR}/ms.x: $(BUILD_DIR)/ms.o $(BUILD_DIR)/ms_R800_mac_30.o $(BUILD_DIR)/ms_vdp_mac.o $(BUILD_DIR)/ms_sysvalue.o $(BUILD_DIR)/ms_sub_mac.o $(BUILD_DIR)/ms_IO_PORT.o $(BUILD_DIR)/ms_PSG_mac.o $(BUILD_DIR)/ms_readcart.o #$(BUILD_DIR)/ms_debugger_mac.o
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/ms_debug.x: $(BUILD_DIR)/ms_debug.o $(BUILD_DIR)/ms_R800_mac_30_debug.o $(BUILD_DIR)/ms_vdp_mac_debug.o $(BUILD_DIR)/ms_sysvalue_debug.o $(BUILD_DIR)/ms_sub_mac_debug.o $(BUILD_DIR)/ms_IO_PORT_debug.o $(BUILD_DIR)/ms_PSG_mac_debug.o $(BUILD_DIR)/ms_readcart_debug.o #$(BUILD_DIR)/ms_debugger_mac_debug.o
	$(LD) $(LDFLAGS) -o $@ $^

${BUILD_DIR}/%_debug.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS_DEBUG) $< -o $@

${BUILD_DIR}/%_debug.o: $(SRC_DIR)/%.has
	$(AS) $(ASFLAGS_DEBUG) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

${BUILD_DIR}/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

${BUILD_DIR}/%.o: $(SRC_DIR)/%.has
	$(AS) $(ASFLAGS) $< -o $@.tmp
	x68k2elf.py $@.tmp $@
	rm $@.tmp

clean:
	rm -rf $(BUILD_DIR)
