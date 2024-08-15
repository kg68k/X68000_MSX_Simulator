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
void ms_vdp_init_sprite(ms_vdp_t* vdp);

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

	// ������ʃ��[�h�� 512x512�ɂ���
	// ���ۂɂ́AMSX�̉�ʃ��[�h�ɉ����Ă��̌�F�X�ω�����
	_iocs_crtmod(4);	// 512x512, 31kHz, 16�F 4��
	_iocs_g_clr_on();	// �O���t�B�b�N�X��ʏ�����

	ms_vdp_init_mac(ms_vdp_shared);

	ms_vdp_init_sprite(ms_vdp_shared);

	update_resolution_COMMON(ms_vdp_shared, 1, 0);

	// GRAM�N���A
	int i;
	for(i=0;i<X68_GRAM_LEN;i++) {
		X68_GRAM[i] = 0;
	}
	// VRAM�N���A
	for(i=0;i<0x20000;i++) {
		ms_vdp_shared->vram[i] = 0;
	}

	return ms_vdp_shared;
}

void ms_vdp_deinit(ms_vdp_t* vdp) {
	ms_vdp_deinit_mac(ms_vdp_shared);
	new_free(vdp->x68_pcg_buffer);
	new_free(vdp->vram);
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
 */
void update_resolution_COMMON(ms_vdp_t* vdp, int res, int color) {
	// lines 0=192���C��, 1=212���C�� (MSX���Z)
	int lines = (vdp->r09 & 0x80) >> 7;
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
 �v���C�g�̏�����
 */
void ms_vdp_init_sprite(ms_vdp_t* vdp) {
	int i;

	// PCG�o�b�t�@�̏�����
	for ( i = 0; i < 256 * 32; i++) {
		vdp->x68_pcg_buffer[i] = 0;
	}
	vdp->last_visible_sprite_planes = 0;
	vdp->last_visible_sprite_size = 0;
}

/*
 �X�v���C�g�p�^�[���W�F�l���[�^�e�[�u���ւ̏�������
     offset: �p�^�[���W�F�l���[�^�e�[�u���̃x�[�X�A�h���X����̃I�t�Z�b�g�o�C�g
     pattern: �������ރp�^�[��(����8bit�̂ݎg�p)
*/
void writeSpritePattern(ms_vdp_t* vdp, int offset, unsigned int pattern) {
	int i,j;
	int ptNum = offset / 8; // MSX�̃X�v���C�g�p�^�[���ԍ�
	int pLine = offset % 8; // �p�^�[���̉��s�ڂ� 
	int pcgLine = pLine * 2; // MSX��1���C����X68000�ł�2���C��
	unsigned int pLeft=0,pRight=0; // 1���C���̍�4�h�b�g�ƉE4�h�b�g�� X68000��8x8�̃p�^�[��2�ɕϊ�

    // �E�[�̃h�b�g���珈��
	for(i =0; i < 4;i++) {
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
	// �p�^�[���W�F�l���[�^�e�[�u���ւ̏�������
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+0 + 16] = pRight;
	vdp->x68_pcg_buffer[ptNum * 32 + pcgLine+1 + 16] = pRight;
}

void writeSpriteAttribute(ms_vdp_t* vdp, int offset, unsigned int attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSX�̃X�v���C�g�v���[���ԍ�
	int type = offset % 4; // �����̎��

	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	switch(type) {
		case 0: // Y���W
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				updateSpriteVisibility(vdp);
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
void updateSpriteVisibility(ms_vdp_t* vdp) {
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
