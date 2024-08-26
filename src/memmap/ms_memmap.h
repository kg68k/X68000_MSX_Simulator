#ifndef _MS_MEMMAP_H_
#define _MS_MEMMAP_H_

#include <stdint.h>

#define ROM_TYPE_NOTHING			0
#define ROM_TYPE_MAPPER_RAM			1
#define ROM_TYPE_NORMAL_ROM			2
#define ROM_TYPE_MEGAROM_8			3
#define ROM_TYPE_MEGAROM_16			4
#define ROM_TYPE_MEGAROM_KONAMI		5
#define ROM_TYPE_MEGAROM_KONAMI_SCC	6
#define ROM_TYPE_SOUND_CARTRIG		7
#define ROM_TYPE_MEGAROM_PANA		8
#define ROM_TYPE_DOS_ROM			9
#define ROM_TYPE_PAC				10

#define MS_MEMMAP_HEADER_LENGTH 8
#define MS_MEMMAP_NUM_SEGMENTS 4

extern uint8_t** ms_memmap_current_ptr;

typedef struct ms_memmap_driver ms_memmap_driver_t;
typedef struct ms_memmap ms_memmap_t;
typedef struct ms_memmap_driver_MAINRAM ms_memmap_driver_MAINRAM_t;
typedef struct ms_memmap_driver_NOTHING ms_memmap_driver_NOTHING_t;

/*	�X���b�g���g������Ă��邩�ǂ���	*/
typedef struct slot_ex_info {
	char flag[4];
} slot_ex_info_t;


typedef struct ms_memmap {
	// �y�[�W���؂�ւ�����ۂɊe�h���C�o����Ăяo�����R�[���o�b�N
	void (*update_page_pointer)(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int page8k);

	// ���C���������̃C���X�^���X�͓��ʂɎQ�Ƃ�����
	// (attached_driver�̒��ɂ��܂܂��)
	ms_memmap_driver_MAINRAM_t* mainram_driver;

	// �������݂��Ȃ��y�[�W�p�̃h���C�o�C���X�^���X�����ʂɎQ�Ƃ�����
	ms_memmap_driver_NOTHING_t* nothing_driver;

	// ���݂̊�{�X���b�g�I����� (= 0xa8���W�X�^�̒l)
	uint8_t	slot_sel[4];

	// ���݂̊g���X���b�g�I����� (= 0xffff���W�X�^�̒l x 4�Z�b�g)
	uint8_t slot_sel_ex[4][4];

	// �X���b�g���g������Ă��邩�ǂ���
	slot_ex_info_t slot_expanded;

	// ����CPU���猩���Ă���X���b�g�z�u
	// ���̒l�́A��L slot_sel, slot_sel_expanded, slot_sel_ex ���瓱�o����܂����A
	// ����v�Z����ƒx�����߁A���̒l���L���b�V�����Ă��܂�
	// �܂��A���̒l���X�V�����ۂ́ACPU���ɂ��̕ύX��ʒm����悤�ɂȂ��Ă��܂��̂ŁA
	// �O���ŏ��������Ȃ��ł��������B
	ms_memmap_driver_t* current_driver[4];

	// �S�ẴX���b�g�z�u
	// ��{�X���b�g4 x �g���X���b�g4 x �y�[�W4
	ms_memmap_driver_t* attached_driver[4][4][4];


	// CPU���Ƌ��L���Ă���|�C���^�̔z��ւ̃|�C���^
	uint8_t** current_ptr;
} ms_memmap_t;


/*
	���̃A�h���X�̓w�b�_�����܂܂Ȃ��������w���B����āA�w�b�_��
	(�A�h���X) - 8 �̃A�h���X���瑶�݂���B
*/
typedef struct ms_memmap_page {
	uint8_t *first_half;    // �O��8K�o�C�g
	uint8_t *second_half;   // �㔼8K�o�C�g
} ms_memmap_page_t;

typedef struct ms_memmap_slot {
	ms_memmap_page_t page_0;
	ms_memmap_page_t page_1;
	ms_memmap_page_t page_2;
	ms_memmap_page_t page_3;
} ms_memmap_page_slot_t;

typedef struct ms_memmap_driver ms_memmap_driver_t;

typedef struct ms_memmap_driver {
	// �{�h���C�o�C���X�^���X���������ꍇ�ɌĂяo���܂�
	void (*deinit)(ms_memmap_driver_t* driver);
	// memmap���W���[�����{�h���C�o���A�^�b�`�����ۂɌĂяo���܂�
	void (*did_attach)(ms_memmap_driver_t* driver);
	// memmap���W���[�����{�h���C�o���f�^�b�`����ۂɌĂяo���܂�
	int (*will_detach)(ms_memmap_driver_t* driver);
	// �������}�b�p�[�Z�O�����g�I�����W�X�^(port FCh,FDh,FEh,FFh) �̒l���ύX���ꂽ�ۂɌĂяo���܂�
	void (*did_update_memory_mapper)(ms_memmap_driver_t* driver, int page, uint8_t segment_num);
	// 8�r�b�g�̓ǂݏo������
	uint8_t (*read8)(ms_memmap_driver_t* memmap, uint16_t addr);
	// 16�r�b�g�̓ǂݏo������
	uint16_t (*read16)(ms_memmap_driver_t* memmap, uint16_t addr);
	// 8�r�b�g�̏������ݏ���
	void (*write8)(ms_memmap_driver_t* memmap, uint16_t addr, uint8_t data);
	// 16�r�b�g�̏������ݏ���
	void (*write16)(ms_memmap_driver_t* memmap, uint16_t addr, uint16_t data);

	// �^�C�v
	int type;
	// ����
	const char* name;

	// ������Ǘ����Ă��� memmap �ւ̎Q��
	ms_memmap_t* memmap;
	//
	int attached_slot_base;
	//
	int attached_slot_ex;

	// 64K�o�C�g��Ԃ�8K�o�C�g�P�ʂŋ�؂����|�C���^�̔z��
	// ���̃h���C�o���Ή����Ă���y�[�W�̃|�C���^�̂݃Z�b�g����Ă���A����ȊO��NULL������܂�
	// ���쒆�Ƀ|�C���^�̒l�������������ꍇ�́Amemmap->update_page_pointer(attached_slot, page_num)���Ăяo���Ă�������
	uint8_t* page8k_pointers[8];

	// buffer (�e�h���C�o���g�p����o�b�t�@�̈�)
	uint8_t* buffer;

} ms_memmap_driver_t;

ms_memmap_t* ms_memmap_init();
void ms_memmap_init_mac();
void ms_memmap_deinit(ms_memmap_t* memmap);
void ms_memmap_deinit_mac();
void ms_memmap_set_main_mem( void *, int);

int ms_memmap_attach_driver(ms_memmap_t* memmap, ms_memmap_driver_t* driver, int slot_base, int slot_ex);

void allocateAndSetROM(const char *romFileName, int kind, int slot_base, int slot_ex, int page);
void allocateAndSetROM_Cartridge(const char* romFileName, int slot_base);

int filelength(int fh);

// �Ō�ɒu��
#include "ms_memmap_NOTHING.h"
#include "ms_memmap_NORMALROM.h"
#include "ms_memmap_MAINRAM.h"
#include "ms_memmap_MEGAROM_8K.h"
#include "ms_memmap_MEGAROM_KONAMI.h"
#include "ms_memmap_MEGAROM_KONAMI_SCC.h"

#endif // _MS_MEMMAP_H_