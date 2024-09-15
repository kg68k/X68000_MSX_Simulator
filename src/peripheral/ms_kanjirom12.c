#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_kanjirom12.h"

#define THIS ms_kanjirom12_t

static void write_kanji_D8(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t read_kanji_D8(ms_ioport_t* ioport, uint8_t port);
static void write_kanji_D9(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t read_kanji_D9(ms_ioport_t* ioport, uint8_t port);

THIS* ms_kanjirom12_alloc() {
	THIS* instance = NULL;
	if( (instance = (ms_kanjirom12_t*)new_malloc(sizeof(ms_kanjirom12_t))) == NULL) {
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}
	return instance;
}

void ms_kanjirom12_init(ms_kanjirom12_t* instance, ms_iomap_t* iomap, char* rom_path) {
	if (instance == NULL) {
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

	// I/O port �A�N�Z�X���
	instance->io_port_D8.instance = instance;
	instance->io_port_D8.read = read_kanji_D8;
	instance->io_port_D8.write = write_kanji_D8;
	ms_iomap_attach_ioport(iomap, 0xd8, &instance->io_port_D8);

	instance->io_port_D9.instance = instance;
	instance->io_port_D9.read = read_kanji_D9;
	instance->io_port_D9.write = write_kanji_D9;
	ms_iomap_attach_ioport(iomap, 0xd9, &instance->io_port_D9);
}

void ms_kanjirom12_deinit(ms_kanjirom12_t* instance, ms_iomap_t* iomap) {
	if (instance->rom_data != NULL) {
		new_free(instance->rom_data);
	}
	ms_iomap_detach_ioport(iomap, 0xd8);
	ms_iomap_detach_ioport(iomap, 0xd9);
}


// I/O port


/*
 * MSX�̊���ROM�͑傫���A��ꐅ���Ƒ�񐅏��ɕ�����Ă��܂��B
 * ��ꐅ���̓|�[�g0xD8,D9�ŃA�N�Z�X���A��񐅏��̓|�[�g0xDA,DB�ŃA�N�Z�X���܂��B
 * ���̎��Ɏg�p����A�h���X�́A�u�����ԍ��v�ƌĂ΂����̂ŁA�ȉ��̌v�Z���ŋ��߂��܂��B
 * 
 * # �����ԍ�
 * ����ROM�̊����ԍ��́AJIS�̋�_�R�[�h���̂��̂ł͂Ȃ����߁A�ϊ����K�v�ł��B
 * MSX Datapack Volume 1 P.306�̕ϊ��������p���܂��B
 * * ��ꐅ��: 1��-15��܂� (�񊿎��̈�B���ۂ�10��ȍ~�͑�ꐅ���̊����ԍ��Ɣ��̂ŁA8��܂ł������^����Ă��Ȃ��Ǝv����)
 * 		* ��ԍ� * 96 + �_�ԍ�
 * * ��ꐅ��: 16��-47��܂� (�����̈�)
 * 		* ��ԍ� * 96 + �_�ԍ� - 512
 * * ��񐅏�: 48��-84��܂�
 * 		* (��ԍ� - 48) * 96 + �_�ԍ�
 * ����ɂ���ďo�Ă��������ԍ����u���6bit�v�u����6bit�v�ɕ����āA�|�[�g�ɏ������ނ��Ƃ�
 * ����ROM���犿����ǂݏo�����Ƃ��ł��܂��B
 * 
 * # ROM�T�C�Y
 * ��ꐅ����ROM�́A47��94�_���Ō�ł��B���̊����̊����ԍ��́A47*96 + 94 - 512 = 4094�ł��B
 * ����1������32�o�C�g���K�v�Ȃ̂ŁA4094 * 32 + 32 = 131040�o�C�g = 0x1ffe0�o�C�g�ł��B
 * 
 * ��񐅏��� 0x20000�o�C�g����n�܂�܂��B84��94�_���Ō�ŁA�����ԍ��́A(84-48) * 96 + 94 = 3550�ł��B
 * ����1������32�o�C�g���K�v�Ȃ̂ŁA3550 * 32 + 32 = 113632�o�C�g = 0x1bfe0�o�C�g�ł��B
 * 
 * ���̂悤�ɁA���ꂼ��128KB�A���v��256KB��ROM���K�v�ł��B
 * 
 * # �p�^�[���̊i�[��
 * MSX�̊���ROM�́A1������16x16�h�b�g�ŁA1����������32�o�C�g�ł��B
 * 1�����́A��8bit�ƉE8bit�ɕ�����Ċi�[����Ă��āA���я��́A
 * * ����
 * * �E��
 * * ����
 * * �E��
 * �̏��Ɋi�[����Ă��܂��B
 */


// ��ꐅ��

/**
 * @brief ��ꐅ��ROM�̊����ԍ�(�O�q)�̉��ʃA�h���X6bit (addr1��bit10-bit5) ���Z�b�g���܂��B
 * ����������1������16x16�h�b�g�ŁA1����������32�o�C�g�ł��B
 * ���̂��߁Aaddr1�̉���6bit��0x20���Ƃɐ؂�ւ��܂��B
 * 
 * @param data 
 */
static void write_kanji_D8(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr1 = (instance->addr1 & 0x1f800) | (((uint32_t)data & 0x3f) << 5);
}

/**
 * @brief ��ꐅ��ROM�̊����ԍ�(�O�q)�̏�ʃA�h���X6bit (addr1��bit16-bit11) ���Z�b�g���܂��B
 * 
 * @param data 
 */
static void write_kanji_D9(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr1 = (instance->addr1 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

static uint8_t read_kanji_D8(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}

static uint8_t read_kanji_D9(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	if(instance->addr1 >= instance->rom_size) {
		return 0xff;
	}
	uint8_t ret = instance->rom_data[instance->addr1];
	// ��ʉ��ʃA�h���X�̓C���N�������g�����A32�o�C�g���ŃC���N�������g����
	instance->addr1 = (instance->addr1 & 0x1ffe0) | ((instance->addr1 + 1) & 0x1f);
	return ret;
}


// ��񐅏�
static void write_kanji_DA(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr2 = (instance->addr2 & 0x3f800) | (((uint32_t)data & 0x3f) << 5);
}

static void write_kanji_DB(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->addr2 = (instance->addr2 & 0x007e0) | (((uint32_t)data & 0x3f) << 11);
}

static uint8_t read_kanji_DA(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}

static uint8_t read_kanji_DB(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	if(instance->addr2 >= instance->rom_size) {
		return 0xff;
	}
	uint8_t ret = instance->rom_data[instance->addr2];
	// ��ʉ��ʃA�h���X�̓C���N�������g�����A32�o�C�g���ŃC���N�������g����
	instance->addr2 = (instance->addr2 & 0x3ffe0) | ((instance->addr2 + 1) & 0x1f);
	return ret;
}