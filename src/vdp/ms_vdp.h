#ifndef MS_VDP_H
#define MS_VDP_H

#include <stdint.h>

// extern uint16_t * const X68_GR_PAL;
// extern uint16_t * const X68_TX_PAL;
// extern uint16_t * const X68_SP_PAL_B0;
// extern uint16_t * const X68_SP_PAL_B1;
// extern uint16_t * const X68_SP_PAL_B2;

// uint16_t * const X68_GR_PAL = (uint16_t *)0xE82000;
// uint16_t * const X68_TX_PAL = (uint16_t *)0xE82200;
// uint16_t * const X68_SP_PAL_B0 = (uint16_t *)0xE82200;	// �u���b�N0�̓e�L�X�g�Ƌ��p
// uint16_t * const X68_SP_PAL_B1 = (uint16_t *)0xE82220;	// �u���b�N1�̓X�v���C�g�p���b�g�Ɏg�p
// uint16_t * const X68_SP_PAL_B2 = (uint16_t *)0xE82240;	// �u���b�N2�ȍ~�͎g�p���Ă��Ȃ�

#define X68_GRAM	((uint16_t *)0xc00000)
#define X68_GRAM_LEN	0x100000	// ���[�h��
#define X68_GR_PAL	((uint16_t *)0xE82000)
#define X68_TX_PAL	((uint16_t *)0xE82200)
#define X68_SP_PAL_B0	((uint16_t *)0xE82200)	// �u���b�N0�̓e�L�X�g�Ƌ��p
#define X68_SP_PAL_B1	((uint16_t *)0xE82220)	// �u���b�N1�̓X�v���C�g�p���b�g�Ɏg�p
#define X68_SP_PAL_B2	((uint16_t *)0xE82240)	// �u���b�N2�ȍ~�͎g�p���Ă��Ȃ�

#define CRTR		((volatile uint16_t *)0xe80000)		// CRTC���W�X�^
#define CRTR_00		(*(volatile uint16_t *)0xe80000)	// CRTC���W�X�^0
#define CRTR_01		(*(volatile uint16_t *)0xe80002)	// CRTC���W�X�^1
#define CRTR_02		(*(volatile uint16_t *)0xe80004)	// CRTC���W�X�^2
#define CRTR_03		(*(volatile uint16_t *)0xe80006)	// CRTC���W�X�^3
#define CRTR_04		(*(volatile uint16_t *)0xe80008)	// CRTC���W�X�^4
#define CRTR_05		(*(volatile uint16_t *)0xe8000a)	// CRTC���W�X�^5
#define CRTR_06		(*(volatile uint16_t *)0xe8000c)	// CRTC���W�X�^6
#define CRTR_07		(*(volatile uint16_t *)0xe8000e)	// CRTC���W�X�^7
#define CRTR_08		(*(volatile uint16_t *)0xe80010)	// CRTC���W�X�^8
#define CRTR_09		(*(volatile uint16_t *)0xe80012)	// CRTC���W�X�^9
#define CRTR_10		(*(volatile uint16_t *)0xe80014)	// CRTC���W�X�^10 (�e�L�X�g�X�N���[��X)
#define CRTR_11		(*(volatile uint16_t *)0xe80016)	// CRTC���W�X�^11 (�e�L�X�g�X�N���[��Y)
#define CRTR_12		(*(volatile uint16_t *)0xe80018)	// CRTC���W�X�^12
#define CRTR_13		(*(volatile uint16_t *)0xe8001a)	// CRTC���W�X�^13
#define CRTR_14		(*(volatile uint16_t *)0xe8001c)	// CRTC���W�X�^14
#define CRTR_15		(*(volatile uint16_t *)0xe8001e)	// CRTC���W�X�^15

#define CRTR_20		(*(volatile uint16_t *)0xe80028)	// CRTC���W�X�^20
#define CRTR_21		(*(volatile uint16_t *)0xe8002a)	// CRTC���W�X�^21
#define CRTR_23		(*(volatile uint16_t *)0xe8002e)	// CRTC���W�X�^23

#define VCRR_00		(*(volatile uint16_t *)0xe82400)	// �r�f�I�R���g���[�����W�X�^0
#define VCRR_01		(*(volatile uint16_t *)0xe82500)	// �r�f�I�R���g���[�����W�X�^1
#define VCRR_02		(*(volatile uint16_t *)0xe82600)	// �r�f�I�R���g���[�����W�X�^2

#define SPCON_BGCON	(*(volatile uint16_t *)0xeb0808)	// BG�R���g���[�����W�X�^
#define SPCON_HTOTAL (*(volatile uint16_t *)0xeb080a)	// �����g�[�^�����W�X�^
#define SPCON_HDISP	(*(volatile uint16_t *)0xeb080c)	// �����𑜓x�ݒ背�W�X�^
#define SPCON_VSISP	(*(volatile uint16_t *)0xeb080e)	// �����𑜓x�ݒ背�W�X�^
#define SPCON_RES	(*(volatile uint16_t *)0xeb0810)	// �X�v���C�g�𑜓x�ݒ背�W�X�^

#define PCG	(*(volatile uint16_t *)0xeb8000)			// �X�v���C�g�p�^�[��

extern uint8_t* ms_vdp_rewrite_flag_tbl;

typedef struct ms_vdp_mode ms_vdp_mode_t;

/**
 * @brief VDP�N���X�̒�`
 * 
 */
typedef struct ms_vdp {
	uint8_t	r00;	// Mode register 0
	uint8_t	r01;	// Mode register 1
	uint8_t	_r02;	// R02�̐��̒l�B���ۂ̃A�N�Z�X�ł� pnametbl_baddr ���g��
	uint8_t _r03;	// R03�̐��̒l�B���ۂ̃A�N�Z�X�ł� R10 �ƌ������� colortbl_baddr ���g��
	uint8_t _r04;	// R04�̐��̒l�B���ۂ̃A�N�Z�X�ł� pgentbl_baddr ���g��
	uint8_t _r05;	// R05�̐��̒l�B���ۂ̃A�N�Z�X�ł� R11 �ƌ������� sprattrtbl_baddr ���g��
	uint8_t _r06;	// R06�̐��̒l�B���ۂ̃A�N�Z�X�ł� sprpgentbl_baddr ���g��
	uint8_t _r07;	// R07�̐��̒l�B���ۂ̃A�N�Z�X�ł� text_color / back_color ���g��
	uint8_t r08;	// Mode register 2
	uint8_t r09;	// Mode register 3
	uint8_t _r10;	// R10�̐��̒l�B���ۂ̃A�N�Z�X�ł� R03 �ƌ������� colortbl_baddr ���g��
	uint8_t _r11;	// R11�̐��̒l�B���ۂ̃A�N�Z�X�ł� R05 �ƌ������� sprattrtbl_baddr ���g��
	uint8_t r12;
	uint8_t r13;
	uint8_t r14;
	uint8_t r15;
	uint8_t r16;
	uint8_t r17;
	uint8_t r18;
	uint8_t r19;
	uint8_t r20;
	uint8_t r21;
	uint8_t r22;
	uint8_t r23;
	uint8_t r24;
	uint8_t r25;
	uint8_t r26;
	uint8_t r27;
	uint8_t r28;
	uint8_t r29;
	uint8_t r30;
	uint8_t r31;
	uint16_t sx;	// 32,33 (���т��t�ɂȂ�̂Œ���)
	uint16_t sy;	// 34,35
	uint16_t dx;	// 36,37
	uint16_t dy;	// 38,39
	uint16_t nx;	// 40,41
	uint16_t ny;	// 42,43
	uint8_t clr;	// 44
	uint8_t arg;	// 45
	uint8_t r46;

	uint8_t dummy1;	// 47

	// Status Registers
	uint8_t s00;	// offset = +48
	uint8_t s01;
	uint8_t s02;
	uint8_t s03;
	uint8_t s04;
	uint8_t s05;
	uint8_t s06;
	uint8_t s07;
	uint8_t s08;
	uint8_t s09;	// offset = +57

	// Palette Registers
	uint16_t palette[16];	// offset = +58

	// Special Control Registers
	// 	����R���g���[�����W�X�^
	//  R_00:	.dc.b	0		* [ 0 ][ DG][IE2][IE1][ M5][ M4][ M3][ 0 ]
	//	R_01:	.dc.b	0		* [ 0 ][ BL][IE0][ M1][ M2][ 0 ][ SI][MAG]
	uint8_t crt_mode;		// R_0,R_1��[ M5]�`[ M0]
	uint8_t sprite_size;	// R_1��[ SI]
	uint8_t sprite_zoom;	// R_1��[MAG]
	uint8_t dummy2;

	// base address registers
	uint32_t pnametbl_baddr;		// R02: Pattern name table base address
	uint32_t colortbl_baddr;		// R03: Color table base address
	uint32_t pgentbl_baddr;		// R04: Pattern generator table base address
	uint32_t sprattrtbl_baddr;		// R05: Sprite attribute table bse address
	uint32_t sprpgentbl_baddr;	// R06: Sprite pattern generator table base address
	uint16_t text_color;			// R07: Text color
	uint16_t back_color;			// R07: Back color

	//
	uint32_t vram_addr;
	uint32_t gram_addr;

	//
	uint16_t dummy3;
	ms_vdp_mode_t *ms_vdp_current_mode;

	//
	uint16_t tx_active;
	uint16_t gr_active;

	// X68000���Ɋm�ۂ���VRAM�̐擪�A�h���X
	uint8_t* vram;

	// X68000��PCG�ɓ]�����邽�߂̃o�b�t�@�̈�
	unsigned int* x68_pcg_buffer;
	int last_visible_sprite_planes;
	int last_visible_sprite_size;
} ms_vdp_t;


/*
 MSX�̉�ʃ��[�h���Ƃɐ؂�ւ��鏈���Q
 
 * init
 	* X68000����CRTC�p�����[�^�ݒ�Ȃǂ��s��
 * VDP���W�X�^�̏��������ɑ΂��鏈��
 * VRAM�ǂݍ���
 * VRAM��������
	* VDP���W�X�^�̒l�ƃA�h���X���r���A�ȉ��𕪊�
		* ��ʏ��������n
		* �X�v���C�g�̏�������
 * VDP�R�}���h�̎��s

 */
typedef struct ms_vdp_mode {
	int (*init)(ms_vdp_t* vdp);
	uint8_t (*read_vram)(ms_vdp_t* vdp);
	void (*write_vram)(ms_vdp_t* vdp, uint8_t data);
	void (*update_palette)(ms_vdp_t* vdp);
	void (*update_pnametbl_baddr)(ms_vdp_t* vdp);
	void (*update_colortbl_baddr)(ms_vdp_t* vdp);
	void (*update_pgentbl_baddr)(ms_vdp_t* vdp);
	void (*update_sprattrtbl_baddr)(ms_vdp_t* vdp);
	void (*update_sprpgentbl_baddr)(ms_vdp_t* vdp);
	void (*update_r7_color)(ms_vdp_t* vdp, uint8_t data);
	char* (*get_mode_name)(ms_vdp_t* vdp);
	void (*exec_vdp_command)(ms_vdp_t* vdp, uint8_t cmd);
	void (*update_resolution)(ms_vdp_t* vdp);
	void (*vsync_draw)(ms_vdp_t* vdp);
	int sprite_mode; // 0x00: ���g�p, 0x01: MODE1, 0x02: MODE2, bit7: 0=256�h�b�g, 1=512�h�b�g
} ms_vdp_mode_t;

ms_vdp_t* ms_vdp_init();
void ms_vdp_deinit(ms_vdp_t* vdp);
void ms_vdp_set_mode(ms_vdp_t* vdp, int mode);

void write_sprite_pattern(ms_vdp_t* vdp, int offset, uint32_t pattern);
void write_sprite_attribute(ms_vdp_t* vdp, int offset, uint32_t attribute);
void update_sprite_visibility(ms_vdp_t* vdp);

void vsync_draw_NONE(ms_vdp_t* vdp);
void exec_vdp_command_DEFAULT(ms_vdp_t* vdp, uint8_t cmd);
void exec_vdp_command_NONE(ms_vdp_t* vdp, uint8_t cmd);

/**
 * 
 * @param vdp 
 * @param res 0=256�h�b�g, 1=512�h�b�g
 * @param color 0=16�F, 1=256�F, 3=65536�F
 * @param bg 0=��\��, 1=�\��
 */
void update_resolution_COMMON(ms_vdp_t* vdp, unsigned int res, unsigned int color, unsigned int bg);

#endif
