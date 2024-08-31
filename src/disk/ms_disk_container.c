#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_container.h"

/*
	�m�ۃ��[�`��
 */
ms_disk_container_t* ms_disk_container_init() {
	return (ms_disk_container_t*)new_malloc(sizeof(ms_disk_container_t));
}

/*
	���������[�`��
 */
void ms_disk_container_init(ms_disk_container_t* instance) {
	if (instance == NULL) {
		return;
	}
	// ���\�b�h�̓o�^
	instance->deinit = ms_disk_container_deinit;
	return instance;
}

void ms_disk_container_deinit(ms_disk_container_t* instance) {
}