#ifndef MS_DISK_H
#define MS_DISK_H

#include <stdint.h>

/**
 * @brief �t���b�s�[�f�B�X�N�̐��̃g���b�N�f�[�^�������\���̂ł��B
 * 2DD 720kB�̏ꍇ�A250kB/Sec �� 300rpm�ƂȂ�A1�g���b�N������ 6250�o�C�g�ƂȂ�܂��B
 * IBM�t�H�[�}�b�g�̏ꍇ�A6250�o�C�g�̓���́A�g���b�N�̐擪���珇�ɁA�ȉ��̂悤�ɂȂ��Ă��܂��B
 * * �v���A���u��
 * * �Z�N�^1
 * 		* ID�t�B�[���h
 * 		* (�M���b�v)
 * 		* �f�[�^�t�B�[���h�i512�o�C�g)
 * 		* CRC
 * 		* (�M���b�v)
 * * �Z�N�^2
 * 		* ID�t�B�[���h
 * 		* (�M���b�v)
 * 		* �f�[�^�t�B�[���h (512�o�C�g)
 * 		* CRC
 * 		* (�M���b�v)
 * * �Z�N�^3-9
 * 		* ���l
 * * �|�X�g�A���u��
 * 
 * ���f�B�A�N���X(ms_disk_media_t)�Ƃ́A����raw track�Ńf�[�^������肵�܂��B
 * raw track���Z�N�^�[���ɕϊ����ăA�N�Z�X����̂́A�R���g���[���[�̐Ӗ��ł��B
 * 
 * �Ȃ��A���f�B�A�N���X�̎����ɂ́A�Z�N�^�x�[�X�ŃC���[�W������ ms_disk_media_disk_t�^������܂����A
 * ���̏ꍇ�ł��A��x raw track �̏�Ԃɂ������̂��ēx controller�ŃZ�N�^�ɖ߂����ƂɂȂ�܂��B
 * �����Ƃ��Ă͏璷�ɂȂ�܂����A�����I�ɃR�s�[�v���e�N�g�̍Č��Ȃǂœ���t�H�[�}�b�g��������悤�ɂ��邽�߂ɂ�
 * ���̂悤�ȃC���^�[�t�F�[�X���K�v�ł��B
 */
typedef struct ms_disk_raw_track {
	uint8_t data[6250];
} ms_disk_raw_track_t;

typedef struct ms_disk_sector {
	uint8_t track;
	uint8_t head;		// FDD�̏ꍇ�� side
	uint8_t sector;
	uint8_t deleted;
	uint8_t data[512];
	uint16_t crc;
	uint16_t crc_expected;
} ms_disk_sector_t;

#endif