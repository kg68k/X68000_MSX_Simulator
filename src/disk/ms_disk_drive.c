#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_drive.h"

/*
	�m�ۃ��[�`��
 */
ms_disk_drive_t* ms_disk_drive_alloc(char* file_path) {
	return (ms_disk_drive_t*)new_malloc(sizeof(ms_disk_drive_t));
}

/*
	���������[�`��
 */
void ms_disk_drive_init(ms_disk_drive_t* instance) {
	if (instance == NULL) {
		return;
	}
	// ���\�b�h�̓o�^
	instance->deinit = ms_disk_drive_deinit;
	return;
}

void ms_disk_drive_deinit(ms_disk_drive_t* instance) {
}