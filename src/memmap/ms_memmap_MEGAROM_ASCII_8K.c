#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_MEGAROM_ASCII_8K.h"

#define THIS ms_memmap_driver_MEGAROM_ASCII_8K_t

static char* driver_name = "MEGAROM_ASCII_8K";
uint8_t dummy_buffer[8192];

void _select_bank_ascii_8K(THIS* d, int page, int bank);
static void _did_attach(ms_memmap_driver_t* driver);
static int _will_detach(ms_memmap_driver_t* driver);

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num);

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr);
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data);
static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr);
static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data);

/*
	�m�ۃ��[�`��
 */
THIS* ms_memmap_MEGAROM_ASCII_8K_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	���������[�`��
 */
void ms_memmap_MEGAROM_ASCII_8K_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, uint32_t length) {
	if (instance == NULL) {
		return;
	}
	int i;
	for(i = 0; i < 8192; i++) {
		dummy_buffer[i] = 0xff;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// �v���p�e�B�⃁�\�b�h�̓o�^
	instance->base.type = ROM_TYPE_MEGAROM_ASCII_8K;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_MEGAROM_ASCII_8K_deinit; �I�[�o�[���C�h�s�v
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	//
	instance->base.buffer = (uint8_t*)buffer;
	instance->num_segments = length / 0x2000;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		instance->base.page8k_pointers[page8k] = NULL;
	}

	for(page8k = 2; page8k < 6; page8k++) {
		_select_bank_ascii_8K(instance, page8k, 0);	// ASCII 8K���K�����̏ꍇ�A�����l��0
	}
	return;
}

static void _did_attach(ms_memmap_driver_t* driver) {
}

static int _will_detach(ms_memmap_driver_t* driver) {
	return 0;
}

static void _did_update_memory_mapper(ms_memmap_driver_t* driver, int slot, uint8_t segment_num) {
}

void _select_bank_ascii_8K(THIS* d, int page8k, int segment) {
	if ( segment >= d->num_segments) {
		printf("MEGAROM_ASCII_8K: segment out of range: %d\n", segment);
		d->base.page8k_pointers[page8k] = dummy_buffer;
		d->selected_segment[page8k] = segment;
	} else {
		d->base.page8k_pointers[page8k] = d->base.buffer + (segment * 0x2000);
		d->selected_segment[page8k] = segment;
	}

	// �؂�ւ����N���������Ƃ� memmap �ɒʒm
	d->base.memmap->update_page_pointer( d->base.memmap, (ms_memmap_driver_t*)d, page8k);
	return;
}


static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = addr >> 13;
	if( page8k < 2 || page8k > 5) {
		printf("MEGAROM_ASCII_8K: read out of range: %04x\n", addr);
		return 0xff;
	}
	int segment = d->selected_segment[page8k];
	if (segment >= d->num_segments) {
		printf("MEGAROM_ASCII_8K: segment out of range: %d\n", segment);
		return 0xff;
	}
	int long_addr = (addr & 0x1fff) + (0x2000 * segment);
	uint8_t ret = driver->buffer[long_addr];
	//printf("MEGAROM_ASCII_8K: read %04x[%06x] -> %02x\n", addr, long_addr, ret);
	return ret;
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	return _read8(driver, addr) | (_read8(driver, addr + 1) << 8);
}

/*
	ASCII 8K���K�����̐؂�ւ�����
	https://www.msx.org/wiki/MegaROM_Mappers#ASCII_8K

	* 4000h~5FFFh (mirror: C000h~DFFFh)
		* �؂�ւ��A�h���X:	6000h (mirrors: 6001h~67FFh)
		* �����Z�O�����g	0
	* 6000h~7FFFh (mirror: E000h~FFFFh)
		* �؂�ւ��A�h���X	6800h (mirrors: 6801h~6FFFh)
		* �����Z�O�����g	0
	* 8000h~9FFFh (mirror: 0000h~1FFFh)
		* �؂�ւ��A�h���X	7000h (mirrors: 7001h~77FFh)
		* �����Z�O�����g	0
	* A000h~BFFFh (mirror: 2000h~3FFFh)
		* �؂�ւ��A�h���X	7800h (mirrors: 7801h~7FFFh)
		* �����Z�O�����g	0
 */
static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	THIS* d = (THIS*)driver;
	// �o���N�؂�ւ�����
	int page8k = -1;
	int area = addr >> 11;
	switch(area) {
		case 0x6*2+0:
			page8k = 2;
			break;
		case 0x6*2+1:
			page8k = 3;
			break;
		case 0x7*2+0:
			page8k = 4;
			break;
		case 0x7*2+1:
			page8k = 5;
			break;
	}
	if (page8k != -1) {
		_select_bank_ascii_8K(d, page8k, data);
	}
	return;
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	THIS* d = (THIS*)driver;
	_write8(driver, addr + 0, data & 0xff);
	_write8(driver, addr + 1, data >> 8);
	return;
}

