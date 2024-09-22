#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <math.h>

#include "ms_psg.h"

int ms_psg_init_mac(uint8_t* psg2octnote, uint8_t* psg2kf);
void ms_psg_deinit_mac(void);

uint8_t r_port_A0();
uint8_t r_port_A2();
void w_port_A0(uint8_t data);
void w_port_A1(uint8_t data);

static void _write_psg_A0(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_psg_A0(ms_ioport_t* ioport, uint8_t port);
static void _write_psg_A1(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_psg_A1(ms_ioport_t* ioport, uint8_t port);
static void _write_psg_A2(ms_ioport_t* ioport, uint8_t port, uint8_t data);
static uint8_t _read_psg_A2(ms_ioport_t* ioport, uint8_t port);

// Singleton instance
static ms_psg_t* _shared = NULL;
static initialized = 0;

ms_psg_t* ms_psg_shared_instance() {
	if( _shared != NULL) {
		return _shared;
	}
	if ( (_shared = (ms_psg_t*)new_malloc(sizeof(ms_psg_t))) == NULL)
	{
		MS_LOG(MS_LOG_INFO,"���������m�ۂł��܂���\n");
		return NULL;
	}
	return _shared;
}

void ms_psg_shared_init(ms_iomap_t* iomap) {
	if (initialized) {
		return;
	}
	initialized = 1;

	// I/O port �A�N�Z�X���
	_shared->io_port_A0.instance = _shared;
	_shared->io_port_A0.read = _read_psg_A0;
	_shared->io_port_A0.write = _write_psg_A0;
	ms_iomap_attach_ioport(iomap, 0xa0, &_shared->io_port_A0);

	_shared->io_port_A1.instance = _shared;
	_shared->io_port_A1.read = _read_psg_A1;
	_shared->io_port_A1.write = _write_psg_A1;
	ms_iomap_attach_ioport(iomap, 0xa1, &_shared->io_port_A1);

	_shared->io_port_A2.instance = _shared;
	_shared->io_port_A2.read = _read_psg_A2;
	_shared->io_port_A2.write = _write_psg_A2;
	ms_iomap_attach_ioport(iomap, 0xa2, &_shared->io_port_A2);

	// PSG�̕����p�����[�^n����OPM��oct,note,kf�ɕϊ�����e�[�u��
	// PSG�̕����p�����[�^��n�Ƃ���ƁAPSG�̎��g���́AFpsg = 3579545 / (32 * n) [Hz]�ƂȂ�
	// OPM�̎��g���̓I�N�^�[�u(oct)�A�m�[�g(note)�A�L�[�t���N�V����(kf)�ŕ\�����B
	// OPM�� 3.579545MHz�ŋ쓮����Ă���ꍇ�Aoct=4, note=8, kf=0 �� 440Hz��A�����o��B
	// ���ŕ\���� Fopm = 440 * 2^(oct-4 + (n-8)/12 + kf/12/64) [Hz]
	// �ł���΁AX68000��OPM�� 4MHz�ŋ쓮����Ă��邽�߁A
	// Fopm = 440 * 2^(oct-4 + (n-8)/12 + kf/12/64) * 4/3.579545 [Hz]
	// �ƂȂ�B
	// �ȉ��APSG�̕����p�����[�^n����OPM��oct,note,kf�ɕϊ�����e�[�u�����쐬����B
	// Fopm = Fpsg �ƂȂ�悤�ɁAoct,note,kf�����߂�K�v������̂ŁA�܂��͗��҂̓����𗧂Ă�ƁA
	// 440 * 2^(oct-4 + (note-8)/12 + kf/12/64) * 4/3.579545 = 3579545 / (32 * n)
	// �ƂȂ�B����𐮗�����ƁA
	// 2^(oct-4 + (note-8)/12 + kf/12/64) = 3579545 / (32 * n) * 3.579545 / 4 / 440
	// 2^(oct-4 + (note-8)/12 + kf/12/64) = 227.50607967018821 / n
	// oct-4 + (note-8)/12 + kf/12/64 = log2(227.50607967018821 / n)
	// �ƂȂ�B
	// O = (oct-4)
	// N = (note-8)
	// K = kf
	// �Ƃ���ƁA
	// O*12*64 + N*64 + K = log2(227.50607967018821 / n) * 12 * 64
	// �ƂȂ�B��������A
	// O��oct��N��note��K�̏��ɋ��߂Ă����B
	// �܂��AN=0, K=0�Ƃ��āA�E�ӂ𒴂��Ȃ��ő�� oct�����߂�B(O�͕��ɂȂ邪�Aoct��0�ȏ�)
	// O�����܂�ƁA
	// N*64 + K = (log2(227.50607967018821 / n) - O ) * 12 * 64
	// N*64 + K = (log2(227.50607967018821 / n) + 4 - oct) * 12 * 64
	// �ƂȂ�̂ŁA���́AK=0�Ƃ��āA�E�ӂ𒴂��Ȃ��ő�� note�����߂�B(N�͕��ɂȂ邪�Anote��0�ȏ�)
	// (note-8)*64 = (log2(227.50607967018821 / n) + 4 - oct) * 12 * 64
	// note-8 = (log2(227.50607967018821 / n) + 4 - oct) * 12
	// note = (log2(227.50607967018821 / n) + 4 - oct) * 12 + 8
	// oct, note���m�肷��ƁAK��
	// K = (log2(227.50607967018821 / n) + 4 - oct - (note-8)/12 ) * 12 * 64
	// K = ((log2(227.50607967018821 / n) + 4 - oct) * 12 - note+8 ) * 64
	// �ƂȂ�̂ŁA�E�ӂ��l�̌ܓ����Đ���������K�����߂�B
	// �Ō�ɁAnote��12�ȏ�ɂȂ�P�[�X������̂ŁAnote >= 12 �̏ꍇ�� O��1���₵�āAnote��12���炷�B
	// ����ŁAPSG�̕����p�����[�^n����OPM��oct,note,kf�ɕϊ�����e�[�u�����쐬�ł���B

	uint8_t n2value[] = {0,1,2,4,5,6,8,9,0xa,0xc,0xd,0xe};
	int n;
	_shared->psg2octnote[0] = 7<<4 | n2value[11];
	for(n=1;n<4096;n++) {
		double log2n = log2(227.50607967018821 / n);
		int oct  = (int)(  log2n + 4);
		int note = (int)( (log2n + 4 - oct) * 12 + 8);
		int K    = (int)(((log2n + 4 - oct) * 12 - note + 8) * 64 + 0.5);
		if (K > 63) {
			// �덷�� 64 �ɂȂ�ꍇ������̂ŁA����
			K = 63;
		}
		if (note >= 12) {
			oct++;
			note -= 12;
		}
		if ( oct < 0 ) {
			oct = 0;
			note = 0;
			K = 0;
		} else if ( oct > 7) {
			oct = 7;
			note = 11;
			K = 63;
		}
		_shared->psg2octnote[n] = (oct<<4) | n2value[note];
		_shared->psg2kf[n] = K;
		MS_LOG(MS_LOG_TRACE, "n=%d, Oct=%d, Note=%d, Kf=%d, octnote=%02x, kf=%02x\n", n, oct, note, K, _shared->psg2octnote[n], _shared->psg2kf[n]);
	}

	// �A�Z���u�����[�`���̏�����
	ms_psg_init_mac(_shared->psg2octnote, _shared->psg2kf);
}

void ms_psg_shared_deinit(ms_iomap_t* iomap) {
	if (_shared == NULL) {
		return;
	}
	ms_iomap_detach_ioport(iomap, 0xa0);
	ms_iomap_detach_ioport(iomap, 0xa1);
	ms_iomap_detach_ioport(iomap, 0xa2);

	// �A�Z���u�����[�`���̏I������
	ms_psg_deinit_mac();

	// �V���O���g���̏ꍇ�� deinit�� free����
	new_free(_shared);
	_shared = NULL;
}

static void _write_psg_A0(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	w_port_A0(data);
}

static uint8_t _read_psg_A0(ms_ioport_t* ioport, uint8_t port) {
	return r_port_A0();
}

static void _write_psg_A1(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
	w_port_A1(data);
}

static uint8_t _read_psg_A1(ms_ioport_t* ioport, uint8_t port) {
	return 0xff;
}

static void _write_psg_A2(ms_ioport_t* ioport, uint8_t port, uint8_t data) {
}

static uint8_t _read_psg_A2(ms_ioport_t* ioport, uint8_t port) {
	return r_port_A2();
}
