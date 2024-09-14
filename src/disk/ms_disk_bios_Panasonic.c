#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "../ms.h"
#include "../memmap/ms_memmap.h"
#include "ms_disk_bios_Panasonic.h"

#define THIS ms_memmap_driver_DISKBIOS_PANASONIC_t

static char* driver_name = "DISKBIOS_PANASONIC";

/*
	�m�ۃ��[�`��
 */
THIS* ms_disk_bios_Panasonic_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	���������[�`��
 */
void ms_disk_bios_Panasonic_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, ms_disk_container_t* container) {
	if (instance == NULL) {
		return;
	}
	ms_memmap_driver_init(&instance->base, memmap, buffer);

	instance->base.type = ROM_TYPE_DOS_ROM;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_DISKBIOS_PANASONIC_deinit; �I�[�o�[���C�h�s�v
	instance->base.did_attach = ms_memmap_did_attach_DISKBIOS_PANASONIC;
	instance->base.will_detach = ms_memmap_will_detach_DISKBIOS_PANASONIC;
	instance->base.did_update_memory_mapper = ms_memmap_did_update_memory_mapper_DISKBIOS_PANASONIC;
	instance->base.read8 = ms_memmap_read8_DISKBIOS_PANASONIC;
	instance->base.read16 = ms_memmap_read16_DISKBIOS_PANASONIC;
	instance->base.write8 = ms_memmap_write8_DISKBIOS_PANASONIC;
	instance->base.write16 = ms_memmap_write16_DISKBIOS_PANASONIC;

	// FDC��ROM�̓X���b�g3-2�ɔz�u����Ă��邪�A���̃y�[�W�͒l0x00���ǂ߂�悤�ɂ��Ă���
	// �����̃y�[�W�ł�FDC�̃��W�X�^�����͓ǂݏ����ł���悤�ɂ��Ȃ���΂����Ȃ��炵��
	uint8_t* zero_buffer = (uint8_t*)new_malloc( 8*1024 );
	if(zero_buffer == NULL) {
		printf("���������m�ۂł��܂���B\n");
		return;
	}
	int i;
	for(i = 0; i<8*1024; i++) {
		zero_buffer[i] = 0x00;
	}
	instance->zero_buffer = zero_buffer;

	int page8k = 0;
	for(; page8k < 2; page8k++) {
		instance->base.page8k_pointers[page8k] = zero_buffer;
	}
	for(; page8k < 4; page8k++) {
		instance->base.page8k_pointers[page8k] = instance->base.buffer + ((page8k-2) * 0x2000);
	}
	for(; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = zero_buffer;
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
	THIS* d = (THIS*)driver;
	int addr_16k = addr & 0x3fff;
	if ((addr_16k & 0x3ff0) == 0x3ff0) {
		if((addr >> 14) != 0x2) {
			MS_LOG(MS_LOG_TRACE, "DISKBIOS_PANASONIC: detect page 0,2,3: %04x\n", addr);
		}
		// Memory mapped DISK I/O
		switch(addr_16k) {
		case 0x3ff8:
			// ���W�X�^2�� write only
			return 0xff;
		case 0x3ff9:
			// ���W�X�^3�� write only
			return 0xff;
		case 0x3ffa:
			// ���W�X�^4 ���Q��
			return d->fdc.read_reg4(&d->fdc);
		case 0x3ffb:
			// ���W�X�^5 ���Q��
			return d->fdc.read_reg5(&d->fdc);
		default:
			MS_LOG(MS_LOG_INFO, "DISKBIOS_PANASONIC: read8: unknown addr: %04x\n", addr_16k);
			return 0xff;
		}
	} else {
		int page8k = addr >> 13;
		int addr_8k = addr_16k & 0x1fff;
		uint8_t ret = driver->page8k_pointers[page8k][addr_8k];
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
	THIS* d = (THIS*)driver;
	int addr_16k = addr & 0x3fff;
	if ((addr_16k & 0x3ff0) == 0x3ff0) {
		if((addr >> 14) != 0x2) {
			MS_LOG(MS_LOG_TRACE, "DISKBIOS_PANASONIC: detect page 0,2,3: %04x\n", addr);
		}
		// Memory mapped DISK I/O
		switch(addr_16k) {
		case 0x3ff8:
			// ���W�X�^2 ���X�V
			d->fdc.write_reg2(&d->fdc, data);
			break;
		case 0x3ff9:
			// ���W�X�^3 ���X�V
			d->fdc.write_reg3(&d->fdc, data);
			break;
		case 0x3ffa:
			// ���W�X�^4 ���X�V
			d->fdc.write_reg4(&d->fdc, data);
			break;
		case 0x3ffb:
			// ���W�X�^5 ���X�V
			d->fdc.write_reg5(&d->fdc, data);
			break;
		default:
			MS_LOG(MS_LOG_INFO, "DISKBIOS_PANASONIC: write8: unknown addr: %04x\n", addr_16k);
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
