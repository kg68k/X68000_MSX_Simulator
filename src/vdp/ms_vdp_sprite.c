#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_vdp.h"

/*
 	�X�v���C�g�̏���

	MS.X�́AMSX��256�h�b�g��X68000��512�h�b�g�Ɋg�債�Ă���
	���̂��߁AMSX��8x8�h�b�g�̃X�v���C�g�́AX68000���16x16�h�b�g�ɂȂ�
	MSX�̃X�v���C�g�p�^�[���͍ő�256��`���邱�Ƃ��ł��邪�A
	X68000��16x16�h�b�g�̃X�v���C�g�p�^�[�����ő�128������`�ł����A��������Ȃ��B
	�����ŁAMSX�̃X�v���C�g�͍ő�32�����\���ł��Ȃ����Ƃ𗘗p���A
	X68000�̃X�v���C�g�p�^�[���́A���ݕ\�����̃X�v���C�g�݂̂��`���邱�Ƃɂ���B
	 
 */

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
void refresh_sprite_256(ms_vdp_t* vdp, int plNum);
void refresh_sprite_256_mode1(ms_vdp_t* vdp, int plNum);
void refresh_sprite_256_mode2(ms_vdp_t* vdp);

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
			uint8_t scroll_offset = vdp->r23; // �c�X�N���[����
			// MSX��1���C�����ɕ\�������̂�+1
			// MSX��R23�̏c�X�N���[���ŃX�v���C�g���ꏏ�ɃX�N���[������̂ŁA���̕�������
			// 256���[�h��MSX��Y���W��1�{
			// X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			int y = ((attribute + 1 - scroll_offset) & 0xff) + 16;
			X68_SSR[plNum*4+1] = y;
			break;
		case 1: // X���W
			X68_SSR[plNum*4+0] = (attribute & 0xff) + 16; // MSX��X���W��1�{, X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			// TODO EC�r�b�g�ɂ��ʒu�␳����
			break;
		case 2: // �p�^�[���ԍ�
		case 3: // ����
			// �p�^�[���ԍ��A�J���[���ύX���ꂽ��A���O�Ƀo�b�t�@�ɓW�J���Ă������p�^�[����]�����A����������
			refresh_sprite_256(vdp, plNum);
			break;
		default:
			break;
	}

	// �p�^�[���w�� & �p���b�g�w��
	X68_SSR[plNum*4+2] = 0x100 + plNum; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
}

void refresh_sprite_256(ms_vdp_t* vdp, int plNum) {
	if((vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1) {
		refresh_sprite_256_mode1(vdp, plNum);
	} else {
		refresh_sprite_256_mode2(vdp);
	}
}

void refresh_sprite_256_mode1(ms_vdp_t* vdp, int plNum) {
	int i,j;
	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	unsigned int ptNum = p[plNum*4+2];
	unsigned int color = p[(plNum*4 & 0x1fffc)+3] & 0xf;
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
}

/*
�@�X�v���C�g���[�h2�̐F���������̎���
�@���[�h2�̐F�����̓h�b�g�P�ʂōs���邽�߁A�^�ʖڂɎ�������Ɣ��ɕ��G�ɂȂ�A���ׂ������̂ŁA
�@�ȗ������đΉ�����i�_���ȃP�[�X�������悤�Ȃ�ȍl�������j
�@�ʏ�̃Q�[���ȂǂŐF�������g���ꍇ�A�ȉ��̂悤�Ȏg���������Ă���͂��B
	* X,Y���W�͊��S�Ɉ�v������
	* �A������X�v���C�g�v���[�����g�p����
		* ���[�h2�̎g�p��́u���ꃉ�C����ɕ���ł��镨�̒��ō����v�Ȃ̂ŕK�������A�����Ă��Ȃ��Ă��ǂ�
		* ���A�X�v���C�g�����؂�ۂȂǂɔj�]����̂ŁA�ʏ�͘A��������͂�
	* �ő�4���̍����܂�
		* 4�r�b�g�����16�F�S�Ă��\���ł���̂�5���ȏ�d�˂�P�[�X�͂Ȃ��Ƒz��
�@�ȏ��O��Ƃ��āA�ȉ��̂悤�ɍ�������B
	* n=0����J�n
	* �v���[��n�Ԃ̃X�v���C�g��XY���W���擾
	* �v���[��n+1�Ԃ���31�̃X�v���C�g�̂����AXY���W������̂��̂��ACC=1�̃��C������ł�������̂𒊏o(�A�����Ă��镨�̂�)
		* �A�������Ō�̔ԍ���m�Ƃ���
	* y=0����ȉ����J��Ԃ�
	* �v���[��n�Ԃ�y���C���ڂ̃p�^�[���ɐF�R�[�h���|����
	* �v���[��n+1����m�܂ňȉ����J��Ԃ�
		* y���C���ڂ̐F�f�[�^���擾���A���ꂼ��̐F�R�[�h���|����
		* CC=1�̏ꍇ�́A���̐F�𒼑O��CC=0�̃v���[���ƍ�������
		* CC=0�̏ꍇ�́A���ꎩ�g�̂̃v���[��(�{���̃v���[��)�ɐF��`�悷��
	OR��������
	* y=y+1���ČJ��Ԃ�
	* �S�Ẵ��C��(8x8���[�h�̏ꍇ��8���C���A16x16���[�h�̏ꍇ��16���C��)�ɑ΂��ČJ��Ԃ�
	* n�� m+1�ɍX�V���An��31�𒴂���܂ŌJ��Ԃ�
�@���̂悤�ɂ���ƁA�ȉ��̂悤�ȗ�O�P�[�X�͏E���Ȃ����A������x�Č��ł���͂��B
	* �v���[��0�Ԃ�CC�r�b�g��1�������ꍇ�A�{���͕`�悵�Ȃ��͂��i���Ⴂ�ԍ����Ȃ��̂Łj�����A�\������Ă��܂�
	* XY�������ɂ���ďd�Ȃ��Ă���P�[�X�͑S�č�������Ȃ�
�@�Ȃ��ACC�r�b�g�̔�������������邽�߁A�ȉ��̎d�g�݂��p�ӂ���B
	* �e�v���[���� 16bit�̃r�b�g�}�b�v�t���O��p�ӂ���
	* �X�v���C�g�J���[�e�[�u�����X�V���ꂽ��ACC=1�̏ꍇ�̓��C���ԍ��ɑΉ�����r�b�g��1�ɂ��ACC=0�̏ꍇ��0�ɂ���
�@���̂悤�ɂ��Ă����ƁA����v���[���̃t���O��0�łȂ���΁A�����ꂩ�̃��C����CC=1�ɂȂ��Ă��邱�Ƃ��킩��
 */

uint16_t sprite_cc_flags[32]; // �X�v���C�g�J���[�e�[�u����CC�r�b�g�̃t���O

void refresh_sprite_256_mode2(ms_vdp_t* vdp) {
	int plNum,n,y,i,j;
	uint8_t* pcol = vdp->vram + vdp->sprcolrtbl_baddr;
	uint8_t* patr = vdp->vram + vdp->sprattrtbl_baddr;
	// �X�v���C�g���[�h2�̐F�������s��
	for (plNum=0;plNum<32;plNum++) {
		int m = plNum;
		for(n=plNum+1;n<32;n++) {
			// XY���W������̂��̂��ACC=1�̃��C������ł�������̂𒊏o(�A�����Ă��镨�̂�)
			if((patr[plNum*4+0] == patr[n*4+0]) && (patr[plNum*4+1] == patr[n*4+1]) && //
				(sprite_cc_flags[n] != 0)) {
				m = n;
			} else {
				break;
			}
		}

		// ************
		//m = plNum; // �e�X�g�p
		// ************

		// ���C�����Ƃ̍�������
		int ymax = vdp->sprite_size == 0 ? 8 : 16;
		int lrmax = vdp->sprite_size == 0 ? 1 : 2;
		int ptNumMask = vdp->sprite_size == 0 ? 0xff : 0xfc;
		int lr;
		for (lr=0;lr < lrmax; lr++) {
			uint16_t mask = 1;
			for	(y=0; y<ymax; y++, mask <<= 1) {
				int yy = y+lr*16;
				i=plNum;
				while(i<=m) {
					uint32_t color = pcol[i*16+y] & 0xf;
					uint32_t colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
					uint32_t ptNum = patr[i*4+2];
					uint32_t pattern = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*8+yy] & colorex;
					j=i;
					while(j<=m) {
						if( j == m ) {
							X68_PCG[i*32+yy] = pattern;
							i=j+1;
							break;
						}
						j++; // j==m���ɔ��肵�Ă���̂ŁAi+1��m���I�[�o�[���邱�Ƃ͂Ȃ�
						if( (sprite_cc_flags[j]&mask) == 0) {
							// CC=0�ɑ���������A����ȍ~�͍������Ȃ�
							X68_PCG[i*32+yy] = pattern;
							i=j;
							break;
						}
						// CC=1�̂��̂����������̂ō�������
						uint32_t color_add = pcol[j*16+y] & 0xf;
						uint32_t colorex_add = color_add << 28 | color_add << 24 | color_add << 20 | color_add << 16 | color_add << 12 | color_add << 8 | color_add << 4 | color_add;
						uint32_t ptNum_add = patr[j*4+2];
						uint32_t pattern_add = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*8+yy] & colorex_add;
						pattern |= pattern_add;
					}
				}
			}
		}
	}
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
			uint8_t scroll_offset = vdp->r23; // �c�X�N���[����
			// MSX��1���C�����ɕ\�������̂�+1
			// MSX��R23�̏c�X�N���[���ŃX�v���C�g���ꏏ�ɃX�N���[������̂ŁA���̕�������
			// 512���[�h��MSX��Y���W��2�{
			// X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			int y = ((attribute + 1 - scroll_offset) & 0xff)*2 + 16;
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+1] = y + (i%2)*16;
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
 �X�v���C�g�J���[�e�[�u���ւ̏�������
     offset: �J���[�e�[�u���̃x�[�X�A�h���X����̃I�t�Z�b�g�o�C�g
     pattern: �������ޒl(����8bit�̂ݎg�p)
*/
void write_sprite_color(ms_vdp_t* vdp, int offset, uint32_t color) {
	if((vdp->ms_vdp_current_mode->sprite_mode & 0x3) != 2) {
		//�@�X�v���C�g���[�h2�ȊO�͉������Ȃ�
		return;
	} else {
		// TODO ���񔽉f����̂͑�ςȂ̂ŁAVRAM�ɂ��������Ă����A�t���O�𗧂āA��������@�ւɂ܂Ƃ߂���������
		
		// CC�r�b�g�̃r�b�g�}�b�v�t���O���X�V
		int plNum = offset / 16; // MSX�̃X�v���C�g�v���[���ԍ�
		int line = offset & 0x0f; // ����X�V���郉�C���ԍ�
		if (color & 0x40) { // CC�r�b�g������
			sprite_cc_flags[plNum] |= (1 << line);
		} else {
			sprite_cc_flags[plNum] &= ~(1 << line);
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

void rewrite_all_sprite(ms_vdp_t* vdp) {
	if(0) {
		printf(".");
		fflush(stdout); 
	}
	int i,j;
	uint32_t sprpgenaddr = vdp->sprpgentbl_baddr & 0x1fe00; // ����9�r�b�g���N���A
	for(i=0;i<256;i++) {
		for(j=0;j<8;j++) {
			write_sprite_pattern(vdp, i*8+j, vdp->vram[sprpgenaddr + i*8 + j]);
		}
	}
	uint32_t sprattraddr = vdp->sprattrtbl_baddr; // & 0x1f80; // ����7�r�b�g���N���A
	for(i=0;i<32;i++) {
		write_sprite_attribute(vdp, i*4+0, vdp->vram[sprattraddr + i*4 + 0]);
		write_sprite_attribute(vdp, i*4+1, vdp->vram[sprattraddr + i*4 + 1]);
		//write_sprite_attribute(vdp, i*4+3, vdp->vram[sprattraddr + i*4 + 3]);
	}
	refresh_sprite_256_mode2(vdp);
}