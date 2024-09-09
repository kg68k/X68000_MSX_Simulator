#ifndef MS_DISK_DRIVE_FLOPPY_H
#define MS_DISK_DRIVE_FLOPPY_H

#include <stdint.h>
#include "ms_disk.h"
#include "ms_disk_drive.h"
#include "ms_disk_container.h"

typedef struct ms_disk_drive_floppy ms_disk_drive_floppy_t;

typedef struct ms_disk_drive_floppy {
	ms_disk_drive_t base;

	// methods
	void (*set_motor)(ms_disk_drive_floppy_t* d, uint8_t motoron);
	void (*set_side)(ms_disk_drive_floppy_t* d, uint8_t side);
	void (*seek)(ms_disk_drive_floppy_t* d, uint32_t track_no);
	uint8_t (*is_disk_inserted)(ms_disk_drive_floppy_t* d);

	/**
	 * @brief ���݂̃w�b�h�ʒu����A���̃Z�N�^���擾���܂�
	 * 
	 * �����ƕ����I�ȃt���b�s�[�f�B�X�N�h���C�u���G�~�����[�V��������ꍇ�́A�o�ߎ���(CPU����)����
	 * �f�B�X�N�̉�]�ʒu���v�Z���āA���w�b�h���h���Ă���ʒu�̃Z�N�^���擾����K�v������܂����A
	 * �ЂƂ܂��ȈՓI�ɁA�V�[�N���ꂽ�g���b�N�̐擪���珇�ɃZ�N�^���擾����悤�ɂ��܂��B
	 */
	uint8_t (*get_next_sector)(ms_disk_drive_floppy_t* d, ms_disk_sector_t* sector_buffer);

	/**
	 * @brief ���݂̃w�b�h�ʒu�̃g���b�N�ɃZ�N�^���������݂܂�
	 * 
	 */
	uint8_t (*write_sector)(ms_disk_drive_floppy_t* d, ms_disk_sector_t* sector_buffer);

	// properties

	/* �f�B�X�N�R���e�i�B null�ɂ���ƃ_�~�[�h���C�u(�@�\���Ȃ��h���C�u)�Ƃ��ĐU�镑�� */
	ms_disk_container_t* container;

	uint8_t is_track00;
	uint8_t is_double_sided;
	uint8_t is_write_protected;

	// private properties
	uint8_t _present_cylinder_number;	//PCN
	uint8_t _present_side_number;
	uint8_t _present_sector_number;
	uint8_t	_track_buffer_ready;
	ms_disk_raw_track_t _track_buffer;

} ms_disk_drive_floppy_t;

ms_disk_drive_floppy_t* ms_disk_drive_floppy_alloc();
void ms_disk_drive_floppy_init(ms_disk_drive_floppy_t* instance, ms_disk_container_t* container);
void ms_disk_drive_floppy_deinit(ms_disk_drive_floppy_t* instance);

#endif