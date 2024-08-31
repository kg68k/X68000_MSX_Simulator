#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_drive_floppy.h"

/*
	�m�ۃ��[�`��
 */
ms_disk_drive_floppy_t* ms_disk_drive_floppy_init() {
	return (ms_disk_drive_floppy_t*)new_malloc(sizeof(ms_disk_drive_floppy_t));
}

/*
	���������[�`��
 */
void ms_disk_drive_floppy_init(ms_disk_drive_floppy_t* instance) {
	if (instance == NULL) {
		return;
	}
	ms_disk_drive_init(&instance->base);
	// ���\�b�h�̓o�^
	instance->base.deinit = ms_disk_drive_floppy_deinit;
	return instance;
}

void ms_disk_drive_floppy_deinit(ms_disk_drive_floppy_t* instance) {
	ms_disk_drive_deinit(&instance->base);
}