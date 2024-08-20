#ifndef _MS_MEMMAP_H_
#define _MS_MEMMAP_H_

#include <stdint.h>

#define ROM_TYPE_NOTHING		0
#define ROM_TYPE_MAPPER_RAM		1
#define ROM_TYPE_NORMAL_ROM		2
#define ROM_TYPE_MEGA_ROM_8		3
#define ROM_TYPE_MEGA_ROM_16	4
#define ROM_TYPE_MEGA_ROM_SCC	5
#define ROM_TYPE_SOUND_CARTRIG	6
#define ROM_TYPE_MEGA_ROM_PANA	7
#define ROM_TYPE_DOS_ROM		8
#define ROM_TYPE_PAC			9

#define MS_MEMMAP_HEADER_LENGTH 8
#define MS_MEMMAP_NUM_SEGMENTS 4

/*	�X���b�g���g������Ă��邩�ǂ���	*/
struct slot_ex_info_st {
	char slot_0;
	char slot_1;
	char slot_2;
	char slot_3;
};

struct one_slot_st {
	uint8_t *page_0;
	uint8_t *page_1;
	uint8_t *page_2;
	uint8_t *page_3;
};

/*
	����y�[�W�̎��̂��w���|�C���^�B�o���N�؂�ւ��̂ł���y�[�W�̏ꍇ�A
	���ݑI������Ă�����̂������Ă���B
*/
typedef struct slot_map	{
	struct one_slot_st s00;
	struct one_slot_st s01;
	struct one_slot_st s02;
	struct one_slot_st s03;

	struct one_slot_st s10;
	struct one_slot_st s11;
	struct one_slot_st s12;
	struct one_slot_st s13;

	struct one_slot_st s20;
	struct one_slot_st s21;
	struct one_slot_st s22;
	struct one_slot_st s23;

	struct one_slot_st s30;
	struct one_slot_st s31;
	struct one_slot_st s32;
	struct one_slot_st s33;
} slot_map_t;

extern slot_map_t *ms_memmap_slot_map_ptr;

typedef struct ms_memmap {
	uint8_t *main_mem;
} ms_memmap_t;

typedef struct ms_memmap_driver ms_memmap_driver_t;

typedef struct ms_memmap_driver {
	// manager
	ms_memmap_t* memmap;

	// memory (CPU����SLOT1�ASLOT2�Ƃ��Č�����̈�)
	// �ݒ��͓������Ă͂����Ȃ��̂ŁA�|�C���^�̎w�����ύX���Ȃ�����
	// �o���N�؂�ւ��������s��ꂽ��A�𒼂ɒ��g�����������邵���Ȃ�
	// TODO: �����I�ɂ�8K�o���N�؂�ւ��̏�����ǉ�����
	uint8_t* mem_slot1;
	uint8_t* mem_slot2;

	// buffer (�e�h���C�o���g�p����o�b�t�@�̈�)
	uint8_t* buffer;

	void (*deinit)(ms_memmap_driver_t* driver);
	uint8_t (*read8)(ms_memmap_driver_t* memmap, uint16_t addr);
	uint16_t (*read16)(ms_memmap_driver_t* memmap, uint16_t addr);
	void (*write8)(ms_memmap_driver_t* memmap, uint16_t addr, uint8_t data);
	void (*write16)(ms_memmap_driver_t* memmap, uint16_t addr, uint16_t data);
} ms_memmap_driver_t;


ms_memmap_t* ms_memmap_init();
void ms_memmap_init_mac();
void ms_memmap_deinit(ms_memmap_t* memmap);
void ms_memmap_deinit_mac();
void ms_memmap_set_main_mem( void *, int);

#endif // _MS_MEMMAP_H_