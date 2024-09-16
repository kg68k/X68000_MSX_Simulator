/*
	16K�o�C�g/32K�o�C�g��ROM���~���[�����O���Ĕz�u����h���C�o

	�ʏ�A16K�o�C�g��ROM�̓y�[�W1�ɔz�u����A32K�o�C�g��ROM�̓y�[�W1�ƃy�[�W2�ɔz�u����܂��B
	�����A������ROM�J�[�g���b�W�́A���ꂼ��̓��e�����̃y�[�W�ɂ��~���[����Č�����悤�ɂȂ��Ă��܂��B
	* 16K�o�C�gROM�̏ꍇ
		* �S�Ẵy�[�W�ɓ������e��������
	* 32K�o�C�gROM�̏ꍇ
		* �y�[�W0�ƃy�[�W2���������e
		* �y�[�W1�ƃy�[�W3���������e

	�{�h���C�o�́A�w�肳�ꂽ�o�b�t�@�T�C�Y�ɍ��킹�āA�����̃~���[�����O���s���܂��B
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"
#include "ms_memmap_MIRROREDROM.h"

#define THIS ms_memmap_driver_MIRROREDROM_t

static char* driver_name = "MIRROREDROM";

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
THIS* ms_memmap_MIRROREDROM_alloc() {
	return (THIS*)new_malloc(sizeof(THIS));
}

/*
	���������[�`��
 */
void ms_memmap_MIRROREDROM_init(THIS* instance, ms_memmap_t* memmap, uint8_t* buffer, int buffer_length) {
	if (instance == NULL) {
		return;
	}

	if (buffer_length != 0x4000 && buffer_length != 0x8000) {
		printf("MIRROREDROM�̃o�b�t�@�T�C�Y��16K�o�C�g�܂���32K�o�C�g�ł���K�v������܂�\n");
		return;
	}

	ms_memmap_driver_init(&instance->base, memmap, buffer);

	// �v���p�e�B�⃁�\�b�h�̓o�^
	instance->base.type = ROM_TYPE_MIRRORED_ROM;
	instance->base.name = driver_name;
	//instance->base.deinit = ms_memmap_MIRROREDROM_deinit; �I�[�o�[���C�h�s�v
	instance->base.did_attach = _did_attach;
	instance->base.will_detach = _will_detach;
	instance->base.did_update_memory_mapper = _did_update_memory_mapper;
	instance->base.read8 = _read8;
	instance->base.read16 = _read16;
	instance->base.write8 = _write8;
	instance->base.write16 = _write16;

	// �v���C�x�[�g�v���p�e�B
	instance->buffer_length = buffer_length;

	int page8k;
	for(page8k = 0; page8k < 8; page8k++) {
		if ( instance->buffer_length == 0x4000 ) {
			// 16K�o�C�gROM
			instance->base.page8k_pointers[page8k] = instance->base.buffer + (page8k%2)*0x2000;
		} else {
			// 32K�o�C�gROM
			int page = page8k / 2;
			if ( page % 2 == 0 ) {
				// �����y�[�W(0,2)
				instance->base.page8k_pointers[page8k] = instance->base.buffer + 0x4000 + (page8k%2)*0x2000;
			} else {
				// ��y�[�W(1,3)
				instance->base.page8k_pointers[page8k] = instance->base.buffer + 0x0000 + (page8k%2)*0x2000;
			}
		}
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

static uint8_t _read8(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = (addr >> 13) & 0x07;
	int local_addr = addr & 0x1fff;

	uint8_t ret = driver->page8k_pointers[page8k][local_addr];
	return ret;
}

static void _write8(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	// read only
}

static uint16_t _read16(ms_memmap_driver_t* driver, uint16_t addr) {
	THIS* d = (THIS*)driver;
	int page8k = (addr >> 13) & 0x07;
	int local_addr = addr & 0x1fff;

	// addr �̓y�[�W���E���܂����Ȃ��悤�ɂȂ��Ă���̂ŋC�ɂ���OK
	uint16_t ret = (driver->page8k_pointers[page8k][local_addr]) | (((uint16_t)driver->page8k_pointers[page8k][local_addr+1]) << 8);
	return ret;
}

static void _write16(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	// read only
}
