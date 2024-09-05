#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_kanjirom12.h"

// Singleton instance
ms_kanjirom12_t* ms_kanjirom12_shared = NULL;
uint8_t ms_kanjirom12shared_initialized = 0;

ms_kanjirom12_t* ms_kanjirom12_alloc() {
	if (ms_kanjirom12_shared != NULL) {
		return ms_kanjirom12_shared;
	}
	if( (ms_kanjirom12_shared = (ms_kanjirom12_t*)new_malloc(sizeof(ms_kanjirom12_t))) == NULL) {
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}
	return ms_kanjirom12_shared;
}

void ms_kanjirom12_init(ms_kanjirom12_t* instance, char* rom_path) {
	if (instance == NULL || ms_kanjirom12shared_initialized) {
		return;
	}
	instance->rom_path = rom_path;
	instance->rom_data = NULL;
	// rom_path �̃t�@�C����ǂݍ���
	int fh = open(rom_path, O_RDONLY | O_BINARY);
	if (fh == -1) {
		printf("�t�@�C�����J���܂���. %s\n", rom_path);
		return;
	}
	int length = filelength(fh);
	if (length == -1) {
		printf("�t�@�C���̒������擾�ł��܂���B\n");
		close(fh);
		return;
	}
	instance->rom_data = (uint8_t*)new_malloc(length);
	if (instance->rom_data == NULL) {
		printf("���������m�ۂł��܂���B\n");
		close(fh);
		return;
	}
	read(fh, instance->rom_data, length);
	close(fh);
	instance->rom_size = length;

	// ��ꐅ���A�h���X
	instance->addr1 = 0x00000;
	// ��񐅏��A�h���X
	instance->addr2 = 0x20000;

	ms_kanjirom12shared_initialized = 1;
}

void ms_kanjirom12_deinit(ms_kanjirom12_t* instance) {
	if (instance->rom_data != NULL) {
		new_free(instance->rom_data);
	}
}


// I/O port


// ��ꐅ��

/**
 * @brief ��ꐅ��ROM�̉��ʃA�h���X6bit (addr1��bit10-bit5) ���Z�b�g���܂�
 * ����������1������16x16�h�b�g�ŁA1����������32�o�C�g�ł��B
 * ���̂��߁Aaddr1�̉���6bit��0x20���Ƃɐ؂�ւ��܂��B
 * ��ʃA�h���X�Ɖ��ʃA�h���X�͂��ꂼ��JIS�����R�[�h�̃R�[�h�\�̏c��(��Ɣԍ�)�ɑΉ����Ă��āA
 * 6bit�����蓖�Ă��Ă��܂��B
 * @param data 
 */
void write_kanji_D8(uint8_t port, uint8_t data) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	ms_kanjirom12_shared->addr1 = (ms_kanjirom12_shared->addr1 & 0x1f800) | (((uint32_t)data & 0x3f) << 5);
}

void write_kanji_D9(uint8_t port, uint8_t data) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	ms_kanjirom12_shared->addr1 = (ms_kanjirom12_shared->addr1 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

uint8_t read_kanji_D9(uint8_t port) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	if(ms_kanjirom12_shared->addr1 >= ms_kanjirom12_shared->rom_size) {
		return 0xff;
	}
	uint8_t ret = ms_kanjirom12_shared->rom_data[ms_kanjirom12_shared->addr1];
	// ��ʉ��ʃA�h���X�̓C���N�������g�����A32�o�C�g���ŃC���N�������g����
	ms_kanjirom12_shared->addr1 = (ms_kanjirom12_shared->addr1 & 0x1ffe0) | ((ms_kanjirom12_shared->addr1 + 1) & 0x1f);
	return ret;
}


// ��񐅏�
void write_kanji_DA(uint8_t port, uint8_t data) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	ms_kanjirom12_shared->addr2 = (ms_kanjirom12_shared->addr2 & 0x3f800) | (((uint32_t)data & 0x3f) << 5);
}

void write_kanji_DB(uint8_t port, uint8_t data) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	ms_kanjirom12_shared->addr2 = (ms_kanjirom12_shared->addr2 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

uint8_t read_kanji_DB(uint8_t port) {
	if(ms_kanjirom12_shared == NULL || !ms_kanjirom12shared_initialized) {
		return;
	}
	if(ms_kanjirom12_shared->addr2 >= ms_kanjirom12_shared->rom_size) {
		return 0xff;
	}
	uint8_t ret = ms_kanjirom12_shared->rom_data[ms_kanjirom12_shared->addr2];
	// ��ʉ��ʃA�h���X�̓C���N�������g�����A32�o�C�g���ŃC���N�������g����
	ms_kanjirom12_shared->addr2 = (ms_kanjirom12_shared->addr2 & 0x3f800) | ((ms_kanjirom12_shared->addr2 + 1) & 0x1f);
	return ret;
}