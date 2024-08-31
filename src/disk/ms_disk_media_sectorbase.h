#ifndef MS_DISK_MEDIA_SECTORBASE_H
#define MS_DISK_MEDIA_SECTORBASE_H

#include <stdint.h>
#include "ms_disk.h"

typedef struct ms_disk_media_sectorbase ms_disk_media_sectorbase_t;

// 512�o�C�g�̔z��� ms_sector_t �Ƃ��Ē�` (C�̒�`�̏�����������Ȃ̂Œ���)
typedef uint8_t ms_sector_t[512];

/**
 * @brief �~�Տ�̃��f�B�A(Disk Media)���Z�N�^�[�x�[�X�Ŏ�������\���̂ł��B
 * 
 * �g���b�N�̃M���b�v����CRC�G���[�Ȃǂ̐��̏������킸�ɁA�Z�N�^�[�f�[�^�݂̂Ń��f�B�A�������ꍇ�Ɏg�p���܂��B
 * .DSK�t�H�[�}�b�g�̃C���[�W���������߂̃T�u�\����(ms_disk_media_dskformat_t) �Ȃǂ����̑�\��ł��B
 * 
 */
typedef struct ms_disk_media_sectorbase {
	ms_disk_media_t base;
	// virtual methods
	void (*read_sector)(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);
	void (*write_sector)(ms_disk_media_t* instance, uint32_t sector_id, ms_sector_t* sector);
	// properties
	uint16_t sectors_per_track;
	uint16_t heads;
	uint16_t tracks;
} ms_disk_media_sectorbase_t;


ms_disk_media_sectorbase_t* ms_disk_media_sectorbase_alloc();
void ms_disk_media_sectorbase_init(ms_disk_media_sectorbase_t* instance);
void ms_disk_media_sectorbase_deinit(ms_disk_media_sectorbase_t* instance);


void ms_disk_media_sectorbase_read_track(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
void ms_disk_media_sectorbase_write_track(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);

#endif