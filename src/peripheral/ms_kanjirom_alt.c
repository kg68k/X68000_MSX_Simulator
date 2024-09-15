/*
	X68000��CGROM�̊������g�p����MSX�̊���ROM���G�~�����[�g������̂ł�
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_kanjirom_alt.h"

#define THIS ms_kanjirom_alt_t

#define CGROM_KANJI_KIGO	((uint16_t*)0xf00000)
#define CGROM_KANJI_LEVEL1	((uint16_t*)0xf05e00)
#define CGROM_KANJI_LEVEL2	((uint16_t*)0xf1d600)

static void _write_kanji_D8(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_kanji_D8(ms_ioport_t* ioport, uint8_t port);
static void _write_kanji_D9(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_kanji_D9(ms_ioport_t* ioport, uint8_t port);

THIS* ms_kanjirom_alt_alloc() {
	THIS* instance = NULL;
	if( (instance = (ms_kanjirom_alt_t*)new_malloc(sizeof(ms_kanjirom_alt_t))) == NULL) {
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}
	return instance;
}

void ms_kanjirom_alt_init(ms_kanjirom_alt_t* instance, ms_iomap_t* iomap) {
	if (instance == NULL) {
		return;
	}

	// ��ꐅ���A�h���X
	instance->k1_num = 0;
	instance->k1_line = 0;
	// ��񐅏��A�h���X
	instance->k2_num = 0;
	instance->k2_line = 0;

	// I/O port �A�N�Z�X���
	instance->io_port_D8.instance = instance;
	instance->io_port_D8.read = _read_kanji_D8;
	instance->io_port_D8.write = _write_kanji_D8;
	ms_iomap_attach_ioport(iomap, 0xd8, &instance->io_port_D8);

	instance->io_port_D9.instance = instance;
	instance->io_port_D9.read = _read_kanji_D9;
	instance->io_port_D9.write = _write_kanji_D9;
	ms_iomap_attach_ioport(iomap, 0xd9, &instance->io_port_D9);
}

void ms_kanjirom_alt_deinit(ms_kanjirom_alt_t* instance, ms_iomap_t* iomap) {
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
 * 
 * # X68000��CGROM����̕ϊ�
 * X68000��CGROM�ɂ́A�ȉ��̂悤��3�̗̈�ɕ�����ăt�H���g�f�[�^���i�[����Ă��܂��B
 * Inside X68000 P.219�̕\���Q�l�ɂ��܂��B
 * 
 * * 0xf00000-0xf05dff: ��ꐅ��-�񊿎��̈�	(1��-8��)
 * 		* JIS�R�[�h���: $21-$28 (1��-8��)
 * 		* JIS�R�[�h����: $21-$7e (1�_-94�_)
 * * 0xf05e00-0xf1d5ff: ��ꐅ��-�����̈� 	(16��-47��)
 * 		* JIS�R�[�h���: $30-$4f (16��-47��)
 * 		* JIS�R�[�h����: $21-$7e
 * * 0xf1d600-0xf388bf: ��񐅏�-�����̈�	(48��-84��)
 * 		* JIS�R�[�h���: $50-$74 (48��-84��)
 * 		* JIS�R�[�h����: $21-$7e
 * 
 * # MSX�̊���ROM�̃`�F�b�N�T��
 * MSX�̊���ROM�́A����������ROM���ǂ������`�F�b�N���邽�߂ɁA�ȉ��̃`�F�b�N�T�����g���܂��B
 * * ��ꐅ��
 *  ��ꐅ����ROM�� JIS�R�[�h 2140H (1��32�_)�̍����8x8�̈悪�A�ȉ��̃p�^�[���ɂȂ��Ă��邩���`�F�b�N���܂��B
 * 
 *	00000000
 *	01000000
 *	00100000
 *	00010000
 *	00001000
 *	00000100
 *	00000010
 *	00000001
 *
 * * ��񐅏�
 * ��񐅏���ROM�� JIS�R�[�h 737EH (83��94�_)�̍����8x8�̈�̃`�F�N�T���� 95H �ɂȂ��Ă��邩���`�F�b�N���܂��B
 * ��:
 *  01 02 0c 37 c0 3b 2a 2a
 * 
 */

// ��ꐅ��

/**
 * @brief ��ꐅ��ROM�̊����ԍ�(�O�q)�̉��ʃA�h���X6bit (addr1��bit10-bit5) ���Z�b�g���܂��B
 * ����������1������16x16�h�b�g�ŁA1����������32�o�C�g�ł��B
 * ���̂��߁Aaddr1�̉���6bit��0x20���Ƃɐ؂�ւ��܂��B
 * 
 * @param data 
 */
static void _write_kanji_D8(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->k1_num = (instance->k1_num & 0xffc0) | (data & 0x3f);	// ����6bit���X�V
	instance->k1_line = 0;
}

/**
 * @brief ��ꐅ��ROM�̊����ԍ�(�O�q)�̏�ʃA�h���X6bit (addr1��bit16-bit11) ���Z�b�g���܂��B
 * 
 * @param data 
 */
static void _write_kanji_D9(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->k1_num =  (((uint16_t)data & 0x3f) << 6) | (instance->k1_num & 0x003f);	// ���6bit���X�V
	instance->k1_line = 0;
}

static uint8_t _read_kanji_D8(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}

static uint8_t _read_kanji_D9(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	int knum = instance->k1_num;
	int line = instance->k1_line;
	int ku;
	int ten;
	uint16_t pattern;
	if( knum < 16*96-512 ) {
		// �񊿎��̈�
		ku = knum / 96 - 1;
		ten = knum % 96 - 1;
		pattern = CGROM_KANJI_KIGO[(ku * 94 + ten)*16+(line%8)+(line/16)*8];
		if ( ku = 0 && ten == 31 && (line < 8)) {
			// 1��32�_�̍����8x8�̈�
			uint8_t check[8] = {
				0b00000000,
				0b01000000,
				0b00100000,
				0b00010000,
				0b00001000,
				0b00000100,
				0b00000010,
				0b00000001
			};
			pattern = (check[line] << 8) | (pattern & 0xff);
		}
	} else {
		// �����̈�
		knum += 512;
		ku = knum / 96 - 16;
		ten = knum % 96 - 1;
		pattern = CGROM_KANJI_LEVEL1[(ku * 94 + ten)*16+(line%8)+(line/16)*8];
	}
	uint8_t ret = ((line & 8) == 0) ? (pattern >> 8) & 0xff : (pattern >> 0) & 0xff;
	// ���C���ԍ����C���N�������g
	instance->k1_line = (line + 1) % 32;
	return ret;
}


// ��񐅏�
static void _write_kanji_DA(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->k2_num = (instance->k2_num & 0xffc0) | (data & 0x3f);	// ����6bit���X�V
	instance->k2_line = 0;
}

static void _write_kanji_DB(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	instance->k2_num =  ((uint16_t)data & 0x3f) << 6 | (instance->k2_num & 0x003f);	// ���6bit���X�V
	instance->k2_line = 0;
}

static uint8_t _read_kanji_DA(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}


static uint8_t _read_kanji_DB(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	int knum = instance->k2_num;
	int line = instance->k2_line;
	int ku;
	int ten;
	uint16_t pattern;

	ku = knum / 96;
	ten = knum % 96 - 1;
	pattern = CGROM_KANJI_LEVEL2[(ku * 94 + ten)*16+(line%8)+(line/16)*8];
	if ( (ku == 35) && (ten == 93) && (line < 8)) {
		// 83��94�_�̍����8x8�̈�
		// �� = 83-48 = 35
		// �_ = 0x7e - 0x20 = 0x5e = 94
		// �����ԍ� = 35 * 96 + 94 = 3454 = 0xd7e
		//   ���6bit = 0x35
		//   ����6bit = 0x3e
		//
		// ku  = 3454 / 96 - 48 = 35
		// ten = 3454 % 96 -  1 = 93
		uint8_t check[8] = {
			0x01, 0x02, 0x0c, 0x37, 0xc0, 0x3b, 0x2a, 0x2a
		};
		pattern = (check[line] << 8) | (pattern & 0xff);
	}
	uint8_t ret = ((line & 8) == 0) ? (pattern >> 8) & 0xff : (pattern >> 0) & 0xff;
	// ���C���ԍ����C���N�������g
	instance->k2_line = (line + 1) % 32;
	return ret;
}


