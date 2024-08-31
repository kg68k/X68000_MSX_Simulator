#ifndef MS_DISK_MEDIA_H
#define MS_DISK_MEDIA_H

#include <stdint.h>
#include "ms_disk.h"

typedef struct ms_disk_media ms_disk_media_t;

/**
 * @brief �~�Տ�̃��f�B�A(Disk Media)��\���\���̂ł��B
 * 
 * ms_disk_rawtrack_t �ŕ\�����~�Տ�̃g���b�N���g���ăA�N�Z�X���܂��B�g���b�N�P�ʂȂ̂ŁA
 * ��������Z�N�^�ɕ������鏈���̓R���g���[���[ (ms_disk_controller_t) �̐Ӗ��ł��B
 * 
 */
typedef struct ms_disk_media {
	// methods
	void (*deinit)(ms_disk_media_t* media);
	void (*read_track)(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);
	void (*write_track)(ms_disk_media_t* media, uint32_t track_no, uint8_t side, ms_disk_raw_track_t* raw_track);

} ms_disk_media_t;

ms_disk_media_t* ms_disk_media_alloc();
void ms_disk_media_init(ms_disk_media_t* instance);
void ms_disk_media_deinit(ms_disk_media_t* instance);

#endif