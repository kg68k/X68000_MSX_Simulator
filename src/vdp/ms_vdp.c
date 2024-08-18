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
void init_palette();

// Singleton instance
ms_vdp_t* ms_vdp_shared = NULL;

ms_vdp_t* ms_vdp_init() {
	if( ms_vdp_shared != NULL ) {
		return ms_vdp_shared;
	}

	if ( (ms_vdp_shared = (ms_vdp_t*)new_malloc(sizeof(ms_vdp_t))) >= (ms_vdp_t *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}
	if ( (ms_vdp_shared->vram = (uint8_t*)new_malloc(0x20000)) >= (uint8_t *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}
	// X68000�� 1�X�v���C�g(16x16)�p�^�[��������128�o�C�g���K�v
	// MSX�� 256��`�ł��邪�AX68000��128������`�ł��Ȃ����߁A��������ɒ�`�̈������Ă���
	// �\�����ɓ]������悤�ɂ��Ă���
	if ( (ms_vdp_shared->x68_pcg_buffer = (unsigned int*)new_malloc( 256 * 32 * sizeof(unsigned int))) >= (unsigned int *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		return NULL;
	}

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
	init_palette();

	return ms_vdp_shared;
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

void init_palette() {
	// X68000�̃p���b�g�t�H�[�}�b�g GGGGGRRR_RRBBBBB1�ɍ��킹��
	int i;
	for (i = 0; i < 16; i++)
	{
		uint16_t color = 1;
		color |= ((default_palette[i][0] >> 3) & 0x1f) << 6;  // R
		color |= ((default_palette[i][1] >> 3) & 0x1f) << 11; // G
		color |= ((default_palette[i][2] >> 3) & 0x1f) << 1;  // B
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
		vdp->ms_vdp_current_mode = &ms_vdp_DEFAULT;
	}
	vdp->ms_vdp_current_mode->update_resolution(vdp);
	vdp->ms_vdp_current_mode->init(vdp);
	printf("VDP Mode: %s\n", vdp->ms_vdp_current_mode->get_mode_name(vdp));
}


uint16_t crtc_values[4][13] = {
	// 256x192, 60Hz
	{	45, 4,  6, 38, 524, 5, 40, 424, 25, // CRTC���W�X�^0-8
		0xff,   6+4,           40, // �X�v���C�g�R���g���[����ʃ��[�h���W�X�^
		8*8	// �e�L�X�g��ʂ̃I�t�Z�b�g�l
	},
	// 256x212, 60Hz
	{	45, 4,  6, 38, 524, 5, 52, 476, 27, // CRTC���W�X�^0-8
		0xff,   6+4,           52, // �X�v���C�g�R���g���[����ʃ��[�h���W�X�^
		8*5+4	// �e�L�X�g��ʂ̃I�t�Z�b�g�l
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
	CRTR_20 = ((color&0x3) << 10) | 0x10 | ((res&0x1) << 2) | (res&0x1);
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

/*
 	�X�v���C�g�̏���

	MS.X�́AMSX��256�h�b�g��X68000��512�h�b�g�Ɋg�債�Ă���
	���̂��߁AMSX��8x8�h�b�g�̃X�v���C�g�́AX68000���16x16�h�b�g�ɂȂ�
	MSX�̃X�v���C�g�p�^�[���͍ő�256��`���邱�Ƃ��ł��邪�A
	X68000��16x16�h�b�g�̃X�v���C�g�p�^�[�����ő�128������`�ł����A��������Ȃ��B
	�����ŁAMSX�̃X�v���C�g�͍ő�32�����\���ł��Ȃ����Ƃ𗘗p���A
	X68000�̃X�v���C�g�p�^�[���́A���ݕ\�����̃X�v���C�g�݂̂��`���邱�Ƃɂ���B
	 
 */

unsigned short* X68_SP_PALETTE = (unsigned short*)0x00e82200; // �X�v���C�g/�e�L�X�g�p���b�g
unsigned short* X68_SSR = (unsigned short*)0x00eb0000; // �X�v���C�g�X�N���[�����W�X�^
unsigned int* X68_PCG = (unsigned int*)0x00eb8000;

/*
 �X�v���C�g�̏�����
 */
void init_sprite(ms_vdp_t* vdp) {
	int i;

	// PCG�o�b�t�@�̏�����
	for ( i = 0; i < 256 * 32; i++) {
		vdp->x68_pcg_buffer[i] = 0;
	}
	vdp->last_visible_sprite_planes = 0;
	vdp->last_visible_sprite_size = 0;
}

void write_sprite_pattern_256(ms_vdp_t* vdp, int offset, uint32_t pattern);
void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern);

/*
 �X�v���C�g�p�^�[���W�F�l���[�^�e�[�u���ւ̏�������
     offset: �p�^�[���W�F�l���[�^�e�[�u���̃x�[�X�A�h���X����̃I�t�Z�b�g�o�C�g
     pattern: �������ރp�^�[��(����8bit�̂ݎg�p)
*/
void write_sprite_pattern(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_pattern_512(vdp, offset, pattern);
	} else {
		write_sprite_pattern_256(vdp, offset, pattern);
	}
}

void write_sprite_pattern_256(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	int i,j;
	int ptNum = offset / 8; // MSX�̃X�v���C�g�p�^�[���ԍ�
	int pLine = offset % 8; // �p�^�[���̉��s�ڂ� 
	int pcgLine = pLine; // MSX��1���C����X68000�ł�1���C��
	uint32_t pcg_pattern=0; // x68000��16x16�̃p�^�[���ɕϊ���������

    // �E�[�̃h�b�g���珈��
	for(i =0; i < 8; i++) {
		pcg_pattern >>= 4;
		if(pattern & 1) {
			pcg_pattern |= (0xf0000000);
		}
		pattern >>= 1;
	}
	// �p�^�[���W�F�l���[�^�e�[�u���̃o�b�t�@�ɏ�������
	vdp->x68_pcg_buffer[ptNum * 8 + pcgLine] = pcg_pattern;
}

void write_sprite_pattern_512(ms_vdp_t* vdp, int offset, uint32_t pattern) {
	int i,j;
	int ptNum = offset / 8; // MSX�̃X�v���C�g�p�^�[���ԍ�
	int pLine = offset % 8; // �p�^�[���̉��s�ڂ� 
	int pcgLine = pLine * 2; // MSX��1���C����X68000�ł�2���C��
	unsigned int pLeft=0,pRight=0; // 1���C���̍�4�h�b�g�ƉE4�h�b�g�� X68000��8x8�̃p�^�[��2�ɕϊ�

    // �E�[�̃h�b�g���珈��
	for(i =0; i < 4; i++) {
		pRight >>= 8;
		if(pattern & 1) {
			pRight |= (0xff000000);
		}
		pattern >>= 1;
	}
    // �c��̍�4�h�b�g�̏���
	for(i =0; i < 4;i++) {
		pLeft >>= 8;
		if(pattern & 1) {
			pLeft |= (0xff000000);
		}
		pattern >>= 1;
	}
	// �p�^�[���W�F�l���[�^�e�[�u���̃o�b�t�@�ɏ�������
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 16] = pRight;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 16] = pRight;
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute);
void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute);

void write_sprite_attribute(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	if(vdp->ms_vdp_current_mode->sprite_mode & 0x80) {
		write_sprite_attribute_512(vdp, offset, attribute);
	} else {
		write_sprite_attribute_256(vdp, offset, attribute);
	}
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSX�̃X�v���C�g�v���[���ԍ�
	int type = offset % 4; // �����̎��

	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	switch(type) {
		case 0: // Y���W
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				update_sprite_visibility(vdp);
			}
			X68_SSR[plNum*4+1] = ((attribute + 1 ) & 0xff) + 16; // MSX��Y���W��1�{, MSX��1���C�����ɕ\�������̂�+1, X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			break;
		case 1: // X���W
			X68_SSR[plNum*4+0] = (attribute & 0xff) + 16; // MSX��X���W��1�{, X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			// TODO EC�r�b�g�ɂ��ʒu�␳����
			break;
		case 2: // �p�^�[���ԍ�
		case 3: // ����
			// �p�^�[���ԍ��A�J���[���ύX���ꂽ��A���O�Ƀo�b�t�@�ɓW�J���Ă������p�^�[����]��
			unsigned int ptNum = p[(offset & 0x1fffc)+2];
			unsigned int color = p[(offset & 0x1fffc)+3] & 0xf;
			unsigned int colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
			if (vdp->sprite_size == 0) {
				// 8x8
				for( i = 0; i < 8; i++) { 
					X68_PCG[plNum*32+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*8+i] & colorex;
				}
			} else {
				// 16x16
				for( i = 0; i < 32; i++) { 
					X68_PCG[plNum*32+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*8+i] & colorex;
				}
			}
			break;
		default:
			break;
	}

	// �p�^�[���w�� & �p���b�g�w��
	X68_SSR[plNum*4+2] = 0x100 + plNum; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
}

void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSX�̃X�v���C�g�v���[���ԍ�
	int type = offset % 4; // �����̎��

	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	switch(type) {
		case 0: // Y���W
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				update_sprite_visibility(vdp);
			}
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+1] = (((attribute + 1 ) & 0xff) * 2) + (i%2)*16 + 16; // MSX��Y���W��2�{, MSX��1���C�����ɕ\�������̂�+1, X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			}
			break;
		case 1: // X���W
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+0] = ((attribute & 0xff) * 2) + (i/2)*16 + 16; // MSX��X���W��2�{
			}
			// TODO EC�r�b�g�ɂ��ʒu�␳����
			break;
		case 2: // �p�^�[���ԍ�
		case 3: // ����
			// �p�^�[���ԍ��A�J���[���ύX���ꂽ��A���O�Ƀo�b�t�@�ɓW�J���Ă������p�^�[����]��
			unsigned int ptNum = p[(offset & 0x1fffc)+2];
			unsigned int color = p[(offset & 0x1fffc)+3] & 0xf;
			unsigned int colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color; // MSX��4�h�b�g��(X68000����2�{��8�h�b�g�Ɋg��)
			if (vdp->sprite_size == 0) { // 8x8
				for( i = 0; i < 32; i++) { 
					X68_PCG[plNum*32*4+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*32+i] & colorex;
				}
			} else { // 16x16
				for( i = 0; i < 32*4; i++) { 
					X68_PCG[plNum*32*4+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*32+i] & colorex;
				}
			}
			break;
		default:
			break;
	}
	if (vdp->sprite_size == 0) {
		// 8x8���[�h
		X68_SSR[plNum*4+2] = 0x100 + plNum; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
	} else {
		// 16x16���[�h�� X68000��� 32x32�ɂȂ�̂ŁA16x16�̃X�v���C�g��4���ׂĕ\������
		for( i=0; i<4; i++) {
			X68_SSR[plNum*16+i*4+2] = 0x100 + plNum*4+i; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
		}
	}

}

/*
 �X�v���C�g�̕\��/��\�����X�V����
 �֘A����d�l�͈ȉ���2��
  * R#8��bit1��1�̂Ƃ��̓X�v���C�g���\���ɂ���
  * ����X�v���C�g�v���[����Y���W��208�ɂ���ƁA���̃X�v���C�g�v���[���ȍ~�͑S�Ĕ�\���ɂȂ�
 */
void update_sprite_visibility(ms_vdp_t* vdp) {
	int i,j;
	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;

	int	visible_sprite_planes; // ��ʂɕ\������X�v���C�g�v���[���̐��B0�ɂ���Ɣ�\���ɂȂ�
	if (vdp->r08 & 0x01) {
		visible_sprite_planes = 0;
	} else {
		visible_sprite_planes = 32;
		for (i = 0; i < 32; i++) {
			if (p[i*4+0] == 208) {
				visible_sprite_planes = i;
				break;
			}
		}
	}
	if ((vdp->last_visible_sprite_planes == visible_sprite_planes) && //
		(vdp->last_visible_sprite_size == vdp->sprite_size)) {
		return;
	}

	vdp->last_visible_sprite_planes = visible_sprite_planes;
	vdp->last_visible_sprite_size = vdp->sprite_size;
	for (i = 0; i < visible_sprite_planes; i++) {
		if (vdp->sprite_size == 0) {
			// 8x8���[�h
			X68_SSR[i*4+3] = 3; // �X�v���C�g�\��
		} else {
			// 16x16���[�h�� X68000��� 32x32�ɂȂ�̂ŁA16x16�̃X�v���C�g��4���ׂĕ\������
			for( j=0; j<4; j++) {
				X68_SSR[i*16+j*4+3] = 3; // �X�v���C�g�\��
			}
		}
	}
	for (; i < 32; i++) {
		if (vdp->sprite_size == 0) {
			// 8x8���[�h
			X68_SSR[i*4+3] = 0; // �X�v���C�g��\��
		} else {
			// 16x16���[�h�� X68000��� 32x32�ɂȂ�̂ŁA16x16�̃X�v���C�g��4���ׂĕ\������
			for( j=0; j<4; j++) {
				X68_SSR[i*16+j*4+3] = 0; // �X�v���C�g��\��
			}
		}
	}
}
