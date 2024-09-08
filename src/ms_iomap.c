#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include "ms_iomap.h"
#include "vdp/ms_vdp.h"

#define THIS ms_iomap_t

static THIS* _shared = NULL;

THIS* ms_iomap_shared_instance() {
	_shared = (THIS*)new_malloc(sizeof(ms_iomap_t));
	if (_shared == NULL) {
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}
	int i;
	for(i=0;i<256;i++) {
		_shared->ioport_table[i] = NULL;
	}
	return _shared;
}

void ms_iomap_shared_deinit() {
	if (_shared == NULL) {
		return;
	}
	// �V���O���g���̏ꍇ�� deinit�� free����
	new_free(_shared);
	_shared = NULL;
}

void ms_iomap_attach_ioport(ms_iomap_t* instance, uint8_t port, ms_ioport_t* ioport) {
	instance->ioport_table[port] = ioport;
}

void ms_iomap_detach_ioport(ms_iomap_t* instance, uint8_t port) {
	instance->ioport_table[port] = NULL;
}

void ms_iomap_out(uint8_t port, uint8_t data) {
	ms_ioport_t* ioport = _shared->ioport_table[port];
	if (ioport == NULL) {
		MS_LOG(MS_LOG_DEBUG, "���Ή��|�[�g�ɏ������݂܂����B0x%02x\n", port);
		return;
	}
	ioport->write(ioport, port, data);
}

uint8_t ms_iomap_in(uint8_t port) {
	ms_ioport_t* ioport = _shared->ioport_table[port];
	if (ioport == NULL) {
		MS_LOG(MS_LOG_DEBUG, "���Ή��|�[�g����ǂݍ��݂܂����B0x%02x\n", port);
		return 0xff;
	}
	return ioport->read(ioport, port);
}