#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_vdp.h"

extern ms_vdp_mode_t ms_vdp_DEFAULT;
extern ms_vdp_mode_t ms_vdp_TEXT1;
extern ms_vdp_mode_t ms_vdp_TEXT2;
extern ms_vdp_mode_t ms_vdp_MULTICOLOR;
extern ms_vdp_mode_t ms_vdp_GRAPHIC1;
extern ms_vdp_mode_t ms_vdp_GRAPHIC2;
extern ms_vdp_mode_t ms_vdp_GRAPHIC3;
extern ms_vdp_mode_t ms_vdp_GRAPHIC4;
extern ms_vdp_mode_t ms_vdp_GRAPHIC5;
extern ms_vdp_mode_t ms_vdp_GRAPHIC6;
extern ms_vdp_mode_t ms_vdp_GRAPHIC7;
extern ms_vdp_mode_t ms_vdp_SCREEN10;
extern ms_vdp_mode_t ms_vdp_SCREEN12;

/* 
	��ʃ��[�h�ꗗ
set_CRT_jpt:
	.dc.l	set_GRAPHIC1	* 0x00
	.dc.l	set_TEXT1	* 0x01
	.dc.l	set_MULTICOLOR	* 0x02
	.dc.l	no_mode
	.dc.l	set_GRAPHIC2	* 0x04
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC3	* 0x08
	.dc.l	set_TEXT2	* 0x09
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC4	* 0x0c
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC5	* 0x10
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC6	* 0x14
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	set_GRAPHIC7	* 0x1c
	.dc.l	no_mode
	.dc.l	no_mode
	.dc.l	no_mode
*/
ms_vdp_mode_t *ms_vdp_mode_table[32] = {
	&ms_vdp_GRAPHIC1,	// 0x00
	&ms_vdp_TEXT1,
	&ms_vdp_MULTICOLOR,
	NULL,
	&ms_vdp_GRAPHIC2,	// 0x04
	NULL,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC3,	// 0x08
	&ms_vdp_TEXT2,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC4,	// 0x0c
	NULL,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC5,	// 0x10
	NULL,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC6,	// 0x14
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	&ms_vdp_GRAPHIC7,	// 0x1c
	NULL,
	NULL,
	NULL
};

int ms_vdp_init_mac(ms_vdp_t* vdp);
void ms_vdp_deinit_mac(ms_vdp_t* vdp);
void init_sprite(ms_vdp_t* vdp);
void init_palette(ms_vdp_t* vdp);

// Singleton instance
ms_vdp_t* ms_vdp_shared = NULL;
uint8_t ms_vdp_shared_initialized = 0;

ms_vdp_t* ms_vdp_alloc() {
	if( ms_vdp_shared != NULL ) {
		return ms_vdp_shared;
	}
	if ( (ms_vdp_shared = (ms_vdp_t*)new_malloc(sizeof(ms_vdp_t))) == NULL)
	{
		MS_LOG(MS_LOG_INFO,"���������m�ۂł��܂���\n");
		return NULL;
	}
	return ms_vdp_shared;
}

void ms_vdp_init(ms_vdp_t* instance) {
	if (instance == NULL || ms_vdp_shared_initialized) {
		return;
	}

	if ( (ms_vdp_shared->vram = (uint8_t*)new_malloc(0x20000)) == NULL)
	{
		MS_LOG(MS_LOG_INFO,"���������m�ۂł��܂���\n");
		ms_vdp_deinit(ms_vdp_shared);
		return;
	}
	// X68000�� 1�X�v���C�g(16x16)�p�^�[��������128�o�C�g(uint32_t��32���[�h)���K�v
	// MSX�� 256��`�ł��邪�AX68000��128������`�ł��Ȃ����߁A��������ɒ�`�̈������Ă���
	// �\�����ɓ]������悤�ɂ��Ă���
	// PCG�o�b�t�@�̕K�v�ő�ʂ́A
	//  * MSX 8x8�h�b�g�̃X�v���C�g256��`
	//  * X68000��512�h�b�g���[�h�Ŋg��X�v���C�g���g����1�h�b�g��4�h�b�g�ɂȂ�A16x16�h�b�g�̃X�v���C�g��4���ׂĕ\������
	//  * => 256 * 4 * 32 * 4�o�C�g = 128KB
	if ( (ms_vdp_shared->x68_pcg_buffer = (uint32_t*)new_malloc( 256 * 4 * 32 * sizeof(uint32_t))) == NULL)
	{
		MS_LOG(MS_LOG_INFO,"���������m�ۂł��܂���\n");
		ms_vdp_deinit(ms_vdp_shared);
		return;
	}

	// b7: F,  b6: 5S, b5: Collision, b4-b0: �Փ˔ԍ�
	ms_vdp_shared->s00 = 0b00011111;
	// b7: FL, b6: LPS, b5-1: V9958��ID, b0: FH
	ms_vdp_shared->s01 = 0b00000100;
	// b7: TR, b6: VR, b5: HR, b4: BD, b3: 1, b2: 1, b1: EO, b0: CE
	ms_vdp_shared->s02 = 0b10001100; // TR�͏��1
	ms_vdp_shared->s04 = 0b11111110; // ��ʃr�b�g��1�Œ�
	ms_vdp_shared->s06 = 0b11111100; // ��ʃr�b�g��1�Œ�
	ms_vdp_shared->s09 = 0b11111100; // ��ʃr�b�g��1�Œ�

	// ������ʃ��[�h�� 512x512�ɂ���
	// ���ۂɂ́AMSX�̉�ʃ��[�h�ɉ����Ă��̌�F�X�ω�����
	_iocs_crtmod(4);	// 512x512, 31kHz, 16�F 4��
	_iocs_g_clr_on();	// �O���t�B�b�N�X��ʏ�����
	_iocs_sp_init();

	ms_vdp_init_mac(ms_vdp_shared);
	// ������Ԃ�TEXT1
	ms_vdp_set_mode(ms_vdp_shared, 0);

	init_sprite(ms_vdp_shared);

	update_resolution_COMMON(ms_vdp_shared, 1, 0, 0); // 512, 16�F, BG�s�g�p

	// GRAM�N���A
	int i;
	for(i=0;i<X68_GRAM_LEN;i++) {
		X68_GRAM[i] = 0;
	}
	// VRAM�N���A
	for(i=0;i<0x20000;i++) {
		ms_vdp_shared->vram[i] = 0;
	}

	// �p���b�g������
	init_palette(ms_vdp_shared);

	return;
}

void ms_vdp_deinit(ms_vdp_t* vdp) {
	ms_vdp_deinit_mac(ms_vdp_shared);
	new_free(vdp->x68_pcg_buffer);
	new_free(vdp->vram);
}

/*
	TMS9918�̓p���b�g���Ȃ��̂ŁAMSX1��ROM���g���ƃp���b�g������������Ȃ�
	���̂��߁A�����ŏ���������
	TMS9918�̐F����V9938�̃p���b�g�͌����ɂ͈Ⴄ���A�����ł�TMS9918�̐F���ɍ��킹��X68000�̃p���b�g������������
	MSX2�ȍ~��ROM���g���Ə㏑������Ă��܂��̂�OK�Ƃ���
	�J���[�R�[�h�͂��̕ӂ���Q�l�ɂ���
	https://forums.atariage.com/topic/262599-palette-fixes-for-colem-adamem-and-classic99/
*/
uint16_t default_palette[16][3] = {
	{0,0,0},		// 0 TRANSPARENT
	{0,0,0},		// 1 BLACK
	{79,176,69},	// 2 MEDIUM GREEN
	{129,202,119},	// 3 LIGHT GREEN
	{95,81,237},	// 4 DARK BLUE
	{129,116,255},	// 5 LIGHT BLUE
	{173,101,77},	// 6 DARK RED
	{103,195,228},	// 7 CYAN
	{204,110,80},	// 8 MEDIUM RED
	{240,146,116},	// 9 LIGHT RED
	{193,202,81},	// 10 DARK YELLOW
	{209,215,129},	// 11 LIDHT YELLOW
	{72,156,59},	// 12 DARK GREEN
	{176,104,190},	// 13 MAGENTA
	{204,204,204},	// 14 GRAY
	{255,255,255}	// 15 WHITE
};

void init_palette(ms_vdp_t* vdp) {
	// X68000�̃p���b�g�t�H�[�}�b�g GGGGGRRR_RRBBBBB1�ɍ��킹��
	int i;
	for (i = 0; i < 16; i++)
	{
		uint16_t color = 1;
		color |= ((default_palette[i][0] >> 3) & 0x1f) << 6;  // R
		color |= ((default_palette[i][1] >> 3) & 0x1f) << 11; // G
		color |= ((default_palette[i][2] >> 3) & 0x1f) << 1;  // B
		vdp->palette[i] = color;
		X68_GR_PAL[i] = color;
		X68_SP_PAL_B1[i] = color;
	}
}

/*
	VDP�̉�ʃ��[�h���Z�b�g����
 */
void ms_vdp_set_mode(ms_vdp_t* vdp, int mode) {
	vdp->ms_vdp_current_mode = ms_vdp_mode_table[mode];
	if (vdp->ms_vdp_current_mode == NULL) {
		MS_LOG(MS_LOG_INFO,"Unknown VDP mode: %d\n", mode);
		vdp->ms_vdp_current_mode = &ms_vdp_DEFAULT;
	}
	vdp->ms_vdp_current_mode->update_resolution(vdp);
	// GRAM�N���A
	uint32_t words = 0;
	switch(vdp->ms_vdp_current_mode->bits_per_dot) {
	case 2:
	case 4:
		// 4�F, 16�F
		words = X68_GRAM_LEN;
		break;
	case 8:
		// 256�F
		words = X68_GRAM_LEN / 2;
		break;
	case 16:
		// 65536�F
		words = X68_GRAM_LEN / 4;
		break;
	}
	int i;
	for(i=0;i<words;i++) {
		X68_GRAM[i] = 0;
	}
	vdp->ms_vdp_current_mode->init(vdp);
	MS_LOG(MS_LOG_INFO,"VDP Mode: %s\n", vdp->ms_vdp_current_mode->get_mode_name(vdp));

	// �X�v���C�g�̏���������
	vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;
}


uint16_t crtc_values[4][13] = {
	// 256x192, 60Hz
	{	45, 4,  6, 38, 524, 5, 40, 424, 25, // CRTC���W�X�^0-8
		0xff,   6+4,           40, // �X�v���C�g�R���g���[����ʃ��[�h���W�X�^
		8*(32+8)	// �e�L�X�g��ʂ̃I�t�Z�b�g�l
	},
	// 256x212, 60Hz
	{	45, 4,  6, 38, 524, 5, 52, 476, 27, // CRTC���W�X�^0-8
		0xff,   6+4,           52, // �X�v���C�g�R���g���[����ʃ��[�h���W�X�^
		8*(32+5)+4	// �e�L�X�g��ʂ̃I�t�Z�b�g�l
	},
	// 512x384, 60Hz
	{	91, 9, 17, 81, 524, 5, 40, 424, 25, // CRTC���W�X�^0-8
		0xff,  17+4,           40, // �X�v���C�g�R���g���[����ʃ��[�h���W�X�^
		16*8	// �e�L�X�g��ʂ̃I�t�Z�b�g�l
	},
	// 512x424, 60Hz
	{	91, 9, 17, 81, 524, 5, 52, 476, 27, // CRTC���W�X�^0-8
		0xff,  17+4,           52, // �X�v���C�g�R���g���[����ʃ��[�h���W�X�^
		16*5+8	// �e�L�X�g��ʂ̃I�t�Z�b�g�l
	}
};

/**
 * @brief Set the display resolution
 * 
 * @param vdp 
 * @param res 0=256�h�b�g, 1=512�h�b�g
 * @param color 0=16�F, 1=256�F, 3=65536�F
 * @param bg 0=��\��, 1=�\��
 */
void update_resolution_COMMON(ms_vdp_t* vdp, unsigned int res, unsigned int color, unsigned int bg) {
	// lines 0=192���C��, 1=212���C�� (MSX���Z)
	int lines = (vdp->r09 & 0x80) >> 7;
	// sprite 0=��\��, 1=�\��
 	int sprite = (vdp->ms_vdp_current_mode->sprite_mode > 0) ? 1 : 0;
	int m = res * 2 + lines;

	CRTR_00	= crtc_values[m][0];
	CRTR_01	= crtc_values[m][1];
	CRTR_02	= crtc_values[m][2];
	CRTR_03	= crtc_values[m][3];
	CRTR_04	= crtc_values[m][4];
	CRTR_05	= crtc_values[m][5];
	CRTR_06	= crtc_values[m][6];
	CRTR_07	= crtc_values[m][7];
	CRTR_08	= crtc_values[m][8];
	CRTR_20 = ((color&0x3) << 8) | 0x10 | ((res&0x1) << 2) | (res&0x1);
	SPCON_HTOTAL = crtc_values[m][9];
	SPCON_HDISP = crtc_values[m][10];
	SPCON_VSISP = crtc_values[m][11];
	SPCON_RES = 0x10 | ((res&0x1) << 2) | (res&0x1);

	// �r�f�I�R���g���[�����W�X�^�̐F�ݒ�
	VCRR_00 = (color&0x3);

	// �e�L�X�g��ʂ̃X�N���[���ʒu�␳
	CRTR_11 = crtc_values[m][12];

	// �X�v���C�g��BG�̐ݒ�
	SPCON_BGCON =	(((sprite | bg) & 0x1) << 9) | // SP/BG = ON
					(0x0 << 4 ) | // BG1 TXSEL
					(0x0 << 3 ) | // BG1 ON
					(0x1 << 2 ) | // BG0 TXSEL
					((bg & 0x1) << 0 );  // BG0 ON
}


uint8_t last_vdp_R1 = 0;
uint8_t last_vdp_R8 = 0;
uint8_t last_vdp_R23 = 0;

/*
*/
void ms_vdp_vsync_draw(ms_vdp_t* vdp) {
	// ��ʃ��[�h���Ƃ̍ĕ`�揈�����Ăяo��
	vdp->ms_vdp_current_mode->vsync_draw(vdp);

	// �X�v���C�g�̍ĕ`�揈��

	if ( (vdp->r01 & 0x01) != (last_vdp_R1 & 0x01) ) {
		// �X�v���C�g�̊g��/�W���T�C�Y���ω�
		// TODO: �܂��X�v���C�g�̊g��͑Ή����Ă��Ȃ�
	}
	if ( (vdp->r01 & 0x02) != (last_vdp_R1 & 0x02) ) {
		// �X�v���C�g�T�C�Y(8x8 or 16x16)���ω�
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_PGEN;
	}
	if ( (vdp->r08 & 0x02) != (last_vdp_R8 & 0x02) ) {
		// �X�v���C�g�\�� ON/OFF�t���O���ω�
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
	}
	if ( vdp->r23 != last_vdp_R23 ) {
		// �X�N���[���ʂ��ω�
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
	}
	ms_vdp_sprite_vsync_draw(vdp);
	last_vdp_R1 = vdp->r01;
	last_vdp_R8 = vdp->r08;
	last_vdp_R23 = vdp->r23;
}