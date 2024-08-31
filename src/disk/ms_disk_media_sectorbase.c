#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk.h"
#include "ms_disk_media_sectorbase.h"


/*
	�m�ۃ��[�`��
 */
ms_disk_media_sectorbase_t* ms_disk_media_sectorbase_alloc() {
	return (ms_disk_media_sectorbase_t*)new_malloc(sizeof(ms_disk_media_sectorbase_t));
}

/*
	���������[�`��
 */
void ms_disk_media_sectorbase_init(ms_disk_media_sectorbase_t* instance) {
	if (instance == NULL) {
		return;
	}
	// base�N���X�̏�����
	ms_disk_media_init(&instance->base);
	// ���\�b�h�̓o�^
	instance->base.deinit = (void (*)(ms_disk_media_t*))ms_disk_media_sectorbase_deinit; //override
	return;
}

void ms_disk_media_sectorbase_deinit(ms_disk_media_sectorbase_t* instance) {
	ms_disk_media_deinit(&instance->base);
}