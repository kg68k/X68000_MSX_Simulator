#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>

#include "ms_rtc.h"

#define THIS ms_rtc_t

#define RTC_X68K ((volatile uint16_t *)0xe8a000) // RTC���W�X�^

uint8_t read_rtc_datetime(uint8_t regnum);
void write_rtc_datetime(uint8_t regnum, uint8_t data);

// Singleton instance
static THIS* _shared = NULL;

THIS* ms_rtc_shared_instance() {
	if (_shared != NULL) {
		return _shared;
	}
	if( (_shared = (THIS*)new_malloc(sizeof(THIS))) == NULL) {
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}

	_shared->regnum = 0;
	_shared->r13 = 0;
	_shared->r14 = 0;
	_shared->r15 = 0;

	int i;
	for (i = 0; i < 13; i++) {
		_shared->block2[i] = 0;
		_shared->block3[i] = 0;
	}

	// MSX�Ńo�b�e���o�b�N�A�b�v����Ă���ݒ�l���f�t�H���g�l�ɂ��Ă���
	// TODO: �t�@�C���ɉi�����ł���悤�ɂ���
	_shared->block2[0] = 10;		// �ۑ��l���L���ȏꍇ��10�ɂȂ�
	_shared->block2[1] = 0x00;	// X-Adjust
	_shared->block2[2] = 0x00;	// Y-Adjust
	_shared->block2[3] = 0x01;	// SCREEN MODE
	_shared->block2[4] = 0x00;	// SCREEN WIDTH (����4bit)
	_shared->block2[5] = 0x02;	// SCREEN WIDTH (���3bit)
	_shared->block2[6] = 0x0f;	// TEXT COLOR
	_shared->block2[7] = 0x04;	// BACKGROUND COLOR
	_shared->block2[8] = 0x07;	// BORDER COLOR
	_shared->block2[9] = 0x01;	// b3:�{�[���[�g, b2:�v�����^���, b1:�L�[�N���b�N, b0:�L�[ON/OFF
	_shared->block2[10] = 0x00;	// b3-2:BEEP timbre, b1-0:BEEP volume
	_shared->block2[11] = 0x00;	// b1-0:�N�����S�̐F
	_shared->block2[12] = 0x00;	// �G���A�R�[�h(0:���{, 1:�č�, etc)

	return _shared;
}


void ms_rtc_shared_deinit() {
	// �V���O���g���̏ꍇ������O�I�� deinit�� free����
	if (_shared == NULL) {
		return;
	}
	new_free(_shared);
	_shared = NULL;
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

uint8_t read_rtc_B4(uint8_t port) {
	if(_shared == NULL) {
		return;
	}
	// ���W�X�^�ԍ���Ԃ�
	return _shared->regnum;
}

void write_rtc_B4(uint8_t port, uint8_t data) {
	if(_shared == NULL) {
		return;
	}
	// ���W�X�^�ԍ����Z�b�g
	_shared->regnum = data;
}

uint8_t read_rtc_B5(uint8_t port) {
	if(_shared == NULL) {
		return;
	}
	// ���W�X�^�̓ǂݏo��
	switch(_shared->regnum) {
	case 13:
		// Mode register
		// b3: Timer Enable
		// b2: Alarm Enable
		// b1-2: Block number
		return _shared->r13;
	case 14:
		// Test register
		return _shared->r14;
	case 15:
		// Reset controller
		return _shared->r15;
	default:
		// ���W�X�^0-12�̓u���b�N�ɂ���ĈقȂ�
		switch(_shared->r13 & 0x03) {
		case 0:
			return read_rtc_datetime(_shared->regnum);
		case 1:
			return 0xff;
		case 2:
			return _shared->block2[_shared->regnum];
		case 3:
			return _shared->block3[_shared->regnum];
		default:
			break;
		}
	}
	return 0xff;
}

void write_rtc_B5(uint8_t port, uint8_t data) {
	if(_shared == NULL) {
		return;
	}
	// ���W�X�^�̏�������
	switch(_shared->regnum) {
	case 13:
		_shared->r13 = data;
		break;
	case 14:
		_shared->r14 = data;
		break;
	case 15:
		_shared->r15 = data;
		break;
	default:
		// ���W�X�^0-12�̓u���b�N�ɂ���ĈقȂ�
		switch(_shared->r13 & 0x03) {
		case 0:
			write_rtc_datetime(_shared->regnum, data);
			break;
		case 1:
			break;
		case 2:
			_shared->block2[_shared->regnum] = data;
			break;
		case 3:
			_shared->block3[_shared->regnum] = data;
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
