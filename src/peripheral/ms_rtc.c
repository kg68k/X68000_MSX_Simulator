#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_rtc.h"

#define THIS ms_rtc_t

// I/O port
static void write_rtc_B4(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t read_rtc_B4(ms_ioport_t* ioport, uint8_t port);
static void write_rtc_B5(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t read_rtc_B5(ms_ioport_t* ioport, uint8_t port);

#define RTC_X68K ((volatile uint16_t *)0xe8a000) // RTC���W�X�^

uint8_t read_rtc_datetime(uint8_t regnum);
void write_rtc_datetime(uint8_t regnum, uint8_t data);

THIS* ms_rtc_alloc() {
	THIS* instance = NULL;
	if( (instance = (ms_rtc_t*)new_malloc(sizeof(ms_rtc_t))) == NULL) {
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}
	return instance;
}

void ms_rtc_init(ms_rtc_t* instance, ms_iomap_t* iomap) {
	if (instance == NULL) {
		return;
	}

	instance->regnum = 0;
	instance->r13 = 0;
	instance->r14 = 0;
	instance->r15 = 0;

	int i;
	for (i = 0; i < 13; i++) {
		instance->block2[i] = 0;
		instance->block3[i] = 0;
	}

	// MSX�Ńo�b�e���o�b�N�A�b�v����Ă���ݒ�l���f�t�H���g�l�ɂ��Ă���
	// TODO: �t�@�C���ɉi�����ł���悤�ɂ���
	instance->block2[0] = 10;		// �ۑ��l���L���ȏꍇ��10�ɂȂ�
	instance->block2[1] = 0x00;	// X-Adjust
	instance->block2[2] = 0x00;	// Y-Adjust
	instance->block2[3] = 0x01;	// SCREEN MODE
	instance->block2[4] = 0x00;	// SCREEN WIDTH (����4bit)
	instance->block2[5] = 0x02;	// SCREEN WIDTH (���3bit)
	instance->block2[6] = 0x0f;	// TEXT COLOR
	instance->block2[7] = 0x04;	// BACKGROUND COLOR
	instance->block2[8] = 0x07;	// BORDER COLOR
	instance->block2[9] = 0x01;	// b3:�{�[���[�g, b2:�v�����^���, b1:�L�[�N���b�N, b0:�L�[ON/OFF
	instance->block2[10] = 0x00;	// b3-2:BEEP timbre, b1-0:BEEP volume
	instance->block2[11] = 0x00;	// b1-0:�N�����S�̐F
	instance->block2[12] = 0x00;	// �G���A�R�[�h(0:���{, 1:�č�, etc)

	// I/O port �A�N�Z�X���
	instance->io_port_B4.instance = instance;
	instance->io_port_B4.read = read_rtc_B4;
	instance->io_port_B4.write = write_rtc_B4;
	ms_iomap_attach_ioport(iomap, 0xb4, &instance->io_port_B4);

	instance->io_port_B5.instance = instance;
	instance->io_port_B5.read = read_rtc_B5;
	instance->io_port_B5.write = write_rtc_B5;
	ms_iomap_attach_ioport(iomap, 0xb5, &instance->io_port_B5);
}


void ms_rtc_deinit(ms_rtc_t* instance, ms_iomap_t* iomap) {
	if (instance == NULL) {
		return;
	}
	ms_iomap_detach_ioport(iomap, 0xb4);
	ms_iomap_detach_ioport(iomap, 0xb5);
}


// I/O port

// 
// �q������ �s������ �b��������
// 
//  Port B4: ���W�X�^�ԍ��w��
//  Port B5: ���W�X�^�̓ǂݏ���
// 
// [MSX��RTC���W�X�^�� RICOH RP-5C01](https://www.msx.org/wiki/Ricoh_RP-5C01)�ŁA
// 4�̃u���b�N��15���̃��W�X�^������(�e���W�X�^��4bit��)�B
// �������A���W�X�^13-15�͂ǂ̃u���b�N�ł��������̂������Ă���̂ŁA�����W�X�^���� 3+12*4=51�B
// 
// * �u���b�N����: 13-15
//    * ���W�X�^13: ���[�h���W�X�^
// * �u���b�N0: 0-12
//    * ���v�̔N���������b���i�[����Ă���
// * �u���b�N1: 0-12
//    * �A���[���������i�[����Ă��邪�AMSX�ł͎g�p���Ă��Ȃ�
// * �u���b�N2: 0-12
//    * MSX�̐ݒ�l(��ʂ̐F��SET ADJUST�̒l�Ȃ�)���i�[����Ă���

static uint8_t read_rtc_B4(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	// ���W�X�^�ԍ���Ԃ�
	return instance->regnum;
}

static void write_rtc_B4(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	// ���W�X�^�ԍ����Z�b�g
	instance->regnum = data;
}

static uint8_t read_rtc_B5(ms_ioport_t* ioport, uint8_t port) {
	THIS* instance = (THIS*)ioport->instance;
	// ���W�X�^�̓ǂݏo��
	switch(instance->regnum) {
	case 13:
		// Mode register
		// b3: Timer Enable
		// b2: Alarm Enable
		// b1-2: Block number
		return instance->r13;
	case 14:
		// Test register
		return instance->r14;
	case 15:
		// Reset controller
		return instance->r15;
	default:
		// ���W�X�^0-12�̓u���b�N�ɂ���ĈقȂ�
		switch(instance->r13 & 0x03) {
		case 0:
			return read_rtc_datetime(instance->regnum);
		case 1:
			return 0xff;
		case 2:
			return instance->block2[instance->regnum];
		case 3:
			return instance->block3[instance->regnum];
		default:
			break;
		}
	}
	return 0xff;
}

static void write_rtc_B5(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	THIS* instance = (THIS*)ioport->instance;
	// ���W�X�^�̏�������
	switch(instance->regnum) {
	case 13:
		instance->r13 = data;
		break;
	case 14:
		instance->r14 = data;
		break;
	case 15:
		instance->r15 = data;
		break;
	default:
		// ���W�X�^0-12�̓u���b�N�ɂ���ĈقȂ�
		switch(instance->r13 & 0x03) {
		case 0:
			write_rtc_datetime(instance->regnum, data);
			break;
		case 1:
			break;
		case 2:
			instance->block2[instance->regnum] = data;
			break;
		case 3:
			instance->block3[instance->regnum] = data;
			break;
		default:
			break;
		}
	}
}

/**
 * @brief RTC�̔N���������b��ǂݏo���܂�
 * 
 * �w�肳�ꂽ���W�X�^�̒l�ɑ�������l��X68000��RTC����N���������b��ǂݏo���A
 * ���̒l��Ԃ��܂��B
 * 
 * @param regnum MSX��RTC�̃��W�X�^�ԍ�(0-12) (�u���b�N0�O��)
 * @return uint8_t �ǂݏo�����l
 */
uint8_t read_rtc_datetime(uint8_t regnum) {
	// X68000��RTC�ƃ��W�X�^�̕��т͓��������AX68000���̓��[�h�A�N�Z�X���K�v
	// �����AC�̃|�C���^�� uint16_t �^�ɂ��Ă���̂ŁA���̂܂ܔԍ���n���Γǂݏo����
	return RTC_X68K[regnum] & 0x0f;	 // ����4bit�����L��
}

void write_rtc_datetime(uint8_t regnum, uint8_t data) {
	// ���t�̏������݂͊�Ȃ��̂Ŏ������Ȃ�
}
