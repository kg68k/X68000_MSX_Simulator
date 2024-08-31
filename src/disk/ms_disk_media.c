#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_media.h"


/*
	�m�ۃ��[�`��
 */
ms_disk_media_t* ms_disk_media_alloc() {
	return (ms_disk_media_t*)new_malloc(sizeof(ms_disk_media_t));
}

/*
	���������[�`��
 */
void ms_disk_media_init(ms_disk_media_t* instance) {
	if (instance == NULL) {
		return;
	}
	// ���\�b�h�̓o�^
	instance->deinit = ms_disk_media_deinit;
	return;
}

void ms_disk_media_deinit(ms_disk_media_t* instance) {
}