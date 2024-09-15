#ifndef MS_KANJIROM_ALT_H
#define MS_KANJIROM_ALT_H

#include <stdint.h>
#include "../ms_iomap.h"

typedef struct ms_kanjirom_alt {
	char* rom_path;

	// ��ꐅ�������ԍ�
	uint16_t k1_num;
	// ��ꐅ�� �s�ԍ�(����A�E��A�����A�E���̏���0-31)
	uint8_t k1_line;

	// ��񐅏������ԍ�
	uint16_t k2_num;
	// ��ꐅ�� �s�ԍ�(����A�E��A�����A�E���̏���0-31)
	uint8_t k2_line;

	// I/O port �A�N�Z�X���
	ms_ioport_t io_port_D8;
	ms_ioport_t io_port_D9;
} ms_kanjirom_alt_t;

ms_kanjirom_alt_t* ms_kanjirom_alt_alloc();
void ms_kanjirom_alt_init(ms_kanjirom_alt_t* instance, ms_iomap_t* iomap);
void ms_kanjirom_alt_deinit(ms_kanjirom_alt_t* instance, ms_iomap_t* iomap);

#endif