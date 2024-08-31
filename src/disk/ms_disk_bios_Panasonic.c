#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "../memmap/ms_memmap.h"
#include "ms_disk_bios_Panasonic.h"

char* driver_name_DISKBIOS_PANASONIC = "DISKBIOS_PANASONIC";

/*
	�m�ۃ��[�`��
 */
ms_memmap_driver_DISKBIOS_PANASONIC_t* ms_disk_bios_Panasonic_alloc() {
	return (ms_memmap_driver_DISKBIOS_PANASONIC_t*)new_malloc(sizeof(ms_memmap_driver_DISKBIOS_PANASONIC_t));
}

/*
	���������[�`��
 */
void ms_disk_bios_Panasonic_init(ms_memmap_driver_DISKBIOS_PANASONIC_t* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length, ms_disk_container_t* container) {
	if (instance == NULL) {
		return;
	}
	ms_memmap_driver_init(&instance->base, memmap, buffer);

	instance->base.type = ROM_TYPE_DOS_ROM;
	instance->base.name = driver_name_DISKBIOS_PANASONIC;
	//instance->base.deinit = ms_memmap_DISKBIOS_PANASONIC_deinit; �I�[�o�[���C�h�s�v
	instance->base.did_attach = ms_memmap_did_attach_DISKBIOS_PANASONIC;
	instance->base.will_detach = ms_memmap_will_detach_DISKBIOS_PANASONIC;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_DISKBIOS_PANASONIC;
	instance->base.read8 = ms_memmap_read8_DISKBIOS_PANASONIC;
	instance->base.read16 = ms_memmap_read16_DISKBIOS_PANASONIC;
	instance->base.write8 = ms_memmap_write8_DISKBIOS_PANASONIC;
	instance->base.write16 = ms_memmap_write16_DISKBIOS_PANASONIC;

	instance->base.buffer = buffer;
	instance->length = length;

	int page8k = 0;
	for(; page8k < 2; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}
	for(; page8k < 4; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->base.buffer + ((page8k-2) * 0x2000);
	}
	for(; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	// FDC�̏�����
	ms_disk_controller_TC8566A_init(&instance->fdc, container);

	return;
}

void ms_memmap_did_attach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver) {
}

int ms_memmap_will_detach_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver) {
	return 0;
}

void ms_memmap_did_update_memory_mapper_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, int page, uint8_t segment_num) {
}


/**
 * @brief 
 * 	Address	R/W	Feature
 *	0x3FF8	W	���W�X�^2 ���X�V (write only)
 *	0x3FF9	W	���W�X�^3 ���X�V (write only)
 *	0x3FFA	R/W	���W�X�^4 ���Q�ƁE�X�V
 *	0x3FFB	R/W	���W�X�^5 ���Q�ƁE�X�V
 */
uint8_t ms_memmap_read8_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_DISKBIOS_PANASONIC_t* d = (ms_memmap_driver_DISKBIOS_PANASONIC_t*)driver;
	addr &= 0x3fff;
	if ((addr & 0x3ff0) == 0x3ff0) {
		// Memory mapped DISK I/O
		switch(addr) {
		case 0x3ff8:
			// ���W�X�^2�� write only
			return 0xff;
		case 0x3ff9:
			// ���W�X�^3�� write only
			return 0xff;
		case 0x3ffa:
			// ���W�X�^4 ���Q��
			return _TC8556AF_reg4_read(d);
		case 0x3ffb:
			// ���W�X�^5 ���Q��
			return _TC8556AF_reg5_read(d);
		default:
			return 0xff;
		}
	} else {
		uint8_t ret = driver->buffer[addr];
		return ret;
	}
}

/**
 * @brief 
 * 	Address	R/W	Feature
 *	0x3FF8	W	���W�X�^2 ���X�V (write only)
 *	0x3FF9	W	���W�X�^3 ���X�V (write only)
 *	0x3FFA	R/W	���W�X�^4 ���Q�ƁE�X�V
 *	0x3FFB	R/W	���W�X�^5 ���Q�ƁE�X�V
 */
void ms_memmap_write8_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	ms_memmap_driver_DISKBIOS_PANASONIC_t* d = (ms_memmap_driver_DISKBIOS_PANASONIC_t*)driver;
	int local_addr = addr & 0x3fff;
	if ( local_addr >= 0x3ff0) {
		// Memory mapped DISK I/O
		switch(local_addr) {
		case 0x3ff8:
			// ���W�X�^2 ���X�V
			_TC8556AF_reg2_write(d, data);
			break;
		case 0x3ff9:
			// ���W�X�^3 ���X�V
			_TC8556AF_reg3_write(d, data);
			break;
		case 0x3ffa:
			// ���W�X�^4 ���X�V
			_TC8556AF_reg4_write(d, data);
			break;
		case 0x3ffb:
			// ���W�X�^5 ���X�V
			_TC8556AF_reg5_write(d, data);
			break;
		}
	}
}

uint16_t ms_memmap_read16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr) {
	// addr �̓y�[�W���E���܂����Ȃ��悤�ɂȂ��Ă���̂ŋC�ɂ���OK
	return ms_memmap_read8_DISKBIOS_PANASONIC(driver, addr) | (ms_memmap_read8_DISKBIOS_PANASONIC(driver, addr + 1) << 8);
}

void ms_memmap_write16_DISKBIOS_PANASONIC(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	ms_memmap_write8_DISKBIOS_PANASONIC(driver, addr, data & 0xff);
	ms_memmap_write8_DISKBIOS_PANASONIC(driver, addr + 1, data >> 8);
}
