#ifndef MS_DISK_MEDIA_SECTORBASE_H
#define MS_DISK_MEDIA_SECTORBASE_H

#include <stdint.h>
#include "ms_disk.h"

typedef struct ms_disk_media_sectorbase ms_disk_media_sectorbase_t;

/**
 * @brief �~�Տ�̃��f�B�A(Disk Media)���Z�N�^�[�x�[�X�Ŏ�������\���̂ł��B
 * 
 * �g���b�N�̃M���b�v����CRC�G���[�Ȃǂ̐��̏������킸�ɁA�Z�N�^�[�f�[�^�݂̂Ń��f�B�A�������ꍇ�Ɏg�p���܂��B
 * .DSK�t�H�[�}�b�g�̃C���[�W���������߂̃T�u�\����(ms_disk_media_dskformat_t) �Ȃǂ����̑�\��ł��B
 * 
 */
typedef struct ms_disk_media_sectorbase {
	ms_disk_media_t base;
	// methods

} ms_disk_media_sectorbase_t;


ms_disk_media_sectorbase_t* ms_disk_media_sectorbase_alloc();
void ms_disk_media_sectorbase_init(ms_disk_media_sectorbase_t* instance);
void ms_disk_media_sectorbase_deinit(ms_disk_media_sectorbase_t* instance);

#endif