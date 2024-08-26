#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "ms_vdp.h"

/*
 	�X�v���C�g�̏���

	MSX�̃X�v���C�g�ɂ́A
	* 8x8�h�b�g
	* 16x16�h�b�g
	* 8x8�h�b�g(�g��)
	* 16x16�h�b�g(�g��)
	��4�̃T�C�Y������܂��B

	MSX�ɂ�256�h�b�g���[�h��512�h�b�g���[�h������܂����A�X�v���C�g�̌����ڂ̃T�C�Y�͕ς�炸�A
	��� 256�h�b�g���[�h�ŕ\������܂��B

	����AMS.X�́A�Č�����X�N���[�����[�h�ɂ���� 256�h�b�g���[�h�� 512�h�b�g���[�h��
	2�̃O���t�B�b�N��ʂ��g�������Ă��܂����AX68000�̃X�v���C�g�̃h�b�g�T�C�Y��
	�O���t�B�b�N��ʂƓ����ł��邽�߁A���̑g�ݍ��킹�����܂���������K�v������܂��B

	�g�ݍ��킹��񋓂���ƁA�ȉ��̂悤�ɂȂ�܂��B

	* MSX 8x8�h�b�g�X�v���C�g & X68000 256�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 1�� �̃X�v���C�g(16x16��1/4���g�p)�ɑΉ�
	* MSX 8x8�h�b�g�X�v���C�g & X68000 512�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 1�� �̃X�v���C�g(16x16)�ɑΉ�
	* MSX 16x16�h�b�g�X�v���C�g & X68000 256�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 1�� �̃X�v���C�g(16x16)�ɑΉ�
	* MSX 16x16�h�b�g�X�v���C�g & X68000 512�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 4�� �̃X�v���C�g(32x32)�ɑΉ�
	* MSX 8x8�h�b�g(�g��)�X�v���C�g & X68000 256�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 1�� �̃X�v���C�g(16x16)�ɑΉ�
	* MSX 8x8�h�b�g(�g��)�X�v���C�g & X68000 512�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 4�� �̃X�v���C�g(32x32)�ɑΉ�
	* MSX 16x16�h�b�g(�g��)�X�v���C�g & X68000 256�h�b�g���[�h
		* MSX��1���̃X�v���C�g�� X68000�� 4�� �̃X�v���C�g(32x32)�ɑΉ�
	* MSX 16x16�h�b�g(�g��)�X�v���C�g & X68000 512�h�b�g���[�h
		* (�Ή����Ȃ�)

	���̂悤�ɂ��Ă݂�ƁAMSX�̃X�v���C�g�v���[���͍ő��X68000��4�̃X�v���C�g��
	�Ή����邱�Ƃ��킩��܂��B

	�Ή��������ÂÂ�Ă����̂ł��񂪂炪��₷�����߁A�ȉ��̂悤�ɗp����`���܂��B

	* �V���O�����[�h / �}���`���[�h
		* 1�X�v���C�g�v���[����1�̃X�v���C�g�ɂȂ�P�[�X���V���O�����[�h�ƌĂ�
		* 1�X�v���C�g�v���[���������̃X�v���C�g�ɂȂ�P�[�X���}���`���[�h�ƌĂ�
	* D1X���[�h / D2X���[�h / D4X���[�h
		* MSX�̃X�v���C�g�p�^�[��1�r�b�g���AX68000��1�h�b�g�ɂȂ�ꍇ�� D1X���[�h�ƌĂ�
			* �g��@�\��2�h�b�g�ɂȂ�ꍇ������Ɋ܂�
		* MSX�̃X�v���C�g�p�^�[��1�r�b�g���AX68000��2�h�b�g�ɂȂ�ꍇ�� D2X���[�h�ƌĂ�
		* MSX�̃X�v���C�g�p�^�[��1�r�b�g���AX68000��4�h�b�g�ɂȂ�ꍇ�� D4X���[�h�ƌĂ�
			* 512�h�b�g���[�h�Ŋg��X�v���C�g���g���ꍇ4�h�b�g�K�v�ɂȂ�

	MSX�͂ǂ̃T�C�Y�ł�32���̃X�v���C�g�v���[����\���ł��AX68000��16x16�̃X�v���C�g��
	128���\���ł���̂ŁA���傤�ǂ҂�������܂��Ă��܂��B

	�����ŁAMS.X�ł́A�ǂ̉�ʃ��[�h�ɂ����Ă��AMSX�̃X�v���C�g�v���[���ԍ���
	�ȉ��̂悤�Ƀ}�b�s���O���邱�Ƃɂ��Ă��܂��B

	MSX�̃X�v���C�g�v���[��  0�� -> X68000�̃X�v���C�g 0-3��
	MSX�̃X�v���C�g�v���[��  1�� -> X68000�̃X�v���C�g 4-7��
		:
	MSX�̃X�v���C�g�v���[�� 31�� -> X68000�̃X�v���C�g 124-127��

	���̂悤�ɁA1�̃X�v���C�g�ŊԂɍ����ꍇ�ł��ԍ����l�߂��ɁA4��΂��Ŏg�p���܂��B
	�������Ă����ƁA��ʃ��[�h���s�����������ł��Ή��֌W����v����̂ŁA
	�Ǘ����y�ɂȂ�̂ł͂Ǝv���Ă��܂��B

	��L�̓X�v���C�g�v���[���̘b�ł����A�X�v���C�g�p�^�[����`�̂ق��́AMSX�̃X�v���C�g��
	�ő�4�{�̒�`��PCG��ɓW�J���邱�Ƃ��ł��Ȃ����߁A���������PCG�p�^�[�������炩����
	�W�J���Ă���(PCG�o�b�t�@)�A�X�v���C�g�v���[���̕\�����ɓ]������悤�ɂ��Ă��܂��B

	���̎��APCG�o�b�t�@�ɂ́AMSX�̃X�v���C�g��`(8x8�P��)�̏��Ԓʂ�ɋl�߂Ċi�[���Ă����܂��B
	�����A���Ԓʂ�ƌ����Ă��AD2X���[�h��D4X���[�h�̏ꍇ�́AMSX��1�X�v���C�g��`���A
	�������ɖc���P�[�X������̂ŁA�������Ȃ��悤�ɂ��Ă��������B


	����AX68000�̃X�v���C�g�ԍ� N�Ԃɂ� ��PCG��N�ł�Ή��Â���悤�ɂ��܂��B���̑Ή�
	�Â��͑S�Ẳ�ʃ��[�h�ŕς��Ȃ����߁A4��΂��Ŏg���邱�Ƃ�����܂��B

	����ƁAMSX��8x8���[�h�̎��́APCG�o�b�t�@��ɂ͘A�����ăp�^�[�����i�[����Ă��܂����A
	X68000�̎�PCG�ɓW�J����ۂ́A16��΂��œW�J���邱�ƂɂȂ�ȂǁA�Ή�����Έ��
	�Ȃ�Ȃ��P�[�X������̂Œ��ӂ��Ă��������B
	(PCG�o�b�t�@�͂����܂�PCG�����Ƃ��̑f�ނ��l�܂��Ă���Ǝv���Ă�������)
 */

#define COL_SIZE			16				// MSX�̃X�v���C�g�J���[�e�[�u���̃T�C�Y(�o�C�g��)
#define SAT_SIZE			4				// MSX�̃X�v���C�g�A�g���r���[�g�e�[�u���̃T�C�Y(�o�C�g��)
#define PCG_SIZE			32				// X68k�̃X�v���C�g1��(16x16)������́APCG�p�^�[���̃��[�h��
#define SSR_SIZE			4				// X68k�̃X�v���C�g1������́ASSR���W�X�^�̃��[�h��
#define PCG_UNIT			(PCG_SIZE * 4)	// MSX�̃v���[��1������́APCG�̃��[�h��
#define SSR_UNIT			(SSR_SIZE * 4)	// MSX�̃v���[��1������́ASSR�̃��[�h��
#define PCG_BUF_UNIT_D1X	8				// D1X���[�h�ŁAMSX��1��`(8x8)��PCG�o�b�t�@��Ő�߂郏�[�h��
#define PCG_BUF_UNIT_D2X	32				// D2X���[�h�ŁAMSX��1��`(8x8)��PCG�o�b�t�@��Ő�߂郏�[�h��
#define PCG_BUF_UNIT_D4X	128				// D4X���[�h�ŁAMSX��1��`(8x8)��PCG�o�b�t�@��Ő�߂郏�[�h��

/*
 �X�v���C�g�̏�����
 */
void init_sprite(ms_vdp_t* vdp) {
	int i;

	// PCG�o�b�t�@�̏�����
	for ( i = 0; i < 256 * PCG_BUF_UNIT_D4X; i++) {
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
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D1X + pcgLine] = pcg_pattern;
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
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+0 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+1 + 0] = pLeft;
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+0 + 16] = pRight;
	vdp->x68_pcg_buffer[ptNum * PCG_BUF_UNIT_D2X + pcgLine+1 + 16] = pRight;
}

void write_sprite_attribute_256(ms_vdp_t* vdp, int offset, uint32_t attribute);
void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute);
void refresh_sprite_512(ms_vdp_t* vdp, int plNum);
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

	uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;
	switch(type) {
		case 0: // Y���W
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
			}
			uint8_t scroll_offset = vdp->r23; // �c�X�N���[����
			// MSX��1���C�����ɕ\�������̂�+1
			// MSX��R23�̏c�X�N���[���ŃX�v���C�g���ꏏ�ɃX�N���[������̂ŁA���̕�������
			// 256���[�h��MSX��Y���W��1�{
			// X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			int y = ((attribute + 1 - scroll_offset) & 0xff) + 16;
			X68_SSR[plNum*SSR_UNIT+1] = y;
			break;
		case 1: // X���W
			X68_SSR[plNum*SSR_UNIT+0] = (attribute & 0xff) + 16; // MSX��X���W��1�{, X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
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
}

void refresh_sprite_256(ms_vdp_t* vdp, int plNum) {
	if((vdp->ms_vdp_current_mode->sprite_mode & 0x3) == 1) {
		refresh_sprite_256_mode1(vdp, plNum);
	} else {
		// ���[�h2�͈����������ƑS�̂ɉe�����o��̂ŁA�����A�����Ԃɂ܂Ƃ߂ď���������
		// TODO: ����Ȃ�ɏd���̂ŁA���������͈͂����肵����
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_ATTR;
	}
}

void refresh_sprite_256_mode1(ms_vdp_t* vdp, int plNum) {
	int i,j;
	uint8_t* p = vdp->vram + vdp->sprattrtbl_baddr;
	unsigned int ptNum = p[plNum*SAT_SIZE+2];
	unsigned int color = p[(plNum*SAT_SIZE & 0x1fffc)+3] & 0xf;
	unsigned int colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
	if (vdp->sprite_size == 0) {
		// 8x8
		for( i = 0; i < 8; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*PCG_BUF_UNIT_D1X+i] & colorex;
		}
	} else {
		// 16x16
		for( i = 0; i < 32; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*PCG_BUF_UNIT_D1X+i] & colorex;
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
			if((patr[plNum*SAT_SIZE+0] == patr[n*SAT_SIZE+0]) && (patr[plNum*SAT_SIZE+1] == patr[n*SAT_SIZE+1]) && //
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
					uint32_t color = pcol[i*COL_SIZE+y] & 0xf;
					uint32_t colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color;
					uint32_t ptNum = patr[i*SAT_SIZE+2];
					uint32_t pattern = vdp->x68_pcg_buffer[(ptNum & ptNumMask)*PCG_BUF_UNIT_D1X+yy] & colorex;
					j=i;
					while(j<=m) {
						if( j == m ) {
							X68_PCG[i*PCG_UNIT+yy] = pattern;
							i=j+1;
							break;
						}
						j++; // j==m���ɔ��肵�Ă���̂ŁAi+1��m���I�[�o�[���邱�Ƃ͂Ȃ�
						if( (sprite_cc_flags[j]&mask) == 0) {
							// CC=0�ɑ���������A����ȍ~�͍������Ȃ�
							X68_PCG[i*PCG_UNIT+yy] = pattern;
							i=j;
							break;
						}
						// CC=1�̂��̂����������̂ō�������
						uint32_t color_add = pcol[j*COL_SIZE+y] & 0xf;
						uint32_t colorex_add = color_add << 28 | color_add << 24 | color_add << 20 | color_add << 16 | color_add << 12 | color_add << 8 | color_add << 4 | color_add;
						uint32_t ptNum_add = patr[j*SAT_SIZE+2];
						uint32_t pattern_add = vdp->x68_pcg_buffer[(ptNum_add & 0xff)*PCG_BUF_UNIT_D1X+yy] & colorex_add;
						pattern |= pattern_add;
					}
				}
			}
		}
	}
}


void write_sprite_attribute_512(ms_vdp_t* vdp, int offset, uint32_t attribute) {
	int i,j;
	int plNum = (offset / SAT_SIZE) % 32; // MSX�̃X�v���C�g�v���[���ԍ�
	int type = offset % SAT_SIZE; // �����̎��

	switch(type) {
		case 0: // Y���W
			if(plNum >= vdp->last_visible_sprite_planes || attribute == 208) {
				vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_COORD;
			}
			uint8_t scroll_offset = vdp->r23; // �c�X�N���[����
			// MSX��1���C�����ɕ\�������̂�+1
			// MSX��R23�̏c�X�N���[���ŃX�v���C�g���ꏏ�ɃX�N���[������̂ŁA���̕�������
			// 512���[�h��MSX��Y���W��2�{
			// X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			int y = ((attribute + 1 - scroll_offset) & 0xff)*2 + 16;
			for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+1] = y + (i%2)*16;
			}
			break;
		case 1: // X���W
			int ec = 0;
			int spMode = vdp->ms_vdp_current_mode->sprite_mode & 0x3;
			switch(spMode) {
			case 1:
				uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;
				ec = (pattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
				break;
			case 2:
				uint8_t* pcolr = vdp->vram + vdp->sprcolrtbl_baddr;
				ec = (pcolr[plNum*COL_SIZE+0] & 0x80) >> 7; // ���C�����Ƃ�EC�̓T�|�[�g���Ȃ��̂�1���C���ڂ����݂�
				break;
			}
			int x = attribute & 0xff;
			x = ((x - ec*32)*2 + 16) & 0x3ff; // MSX��X���W��2�{
			for( i=0; i<4; i++) {
				X68_SSR[plNum*SSR_UNIT+i*4+0] = x + (i/2)*16;
			}
			break;
		case 2: // �p�^�[���ԍ�
		case 3: // ����
			// �p�^�[���ԍ��A�J���[���ύX���ꂽ��A���O�Ƀo�b�t�@�ɓW�J���Ă������p�^�[����]��
			refresh_sprite_512(vdp, plNum);
			break;
		default:
			break;
	}
}

void refresh_sprite_512(ms_vdp_t* vdp, int plNum) {
	int i;
	uint8_t* pattr = vdp->vram + vdp->sprattrtbl_baddr;

	unsigned int ptNum = pattr[plNum*SAT_SIZE+2];
	unsigned int color = pattr[plNum*SAT_SIZE+3] & 0xf;
	unsigned int colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color; // MSX��4�h�b�g��(X68000����2�{��8�h�b�g�Ɋg��)
	if (vdp->sprite_size == 0) { // 8x8
		for( i = 0; i < 32; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = vdp->x68_pcg_buffer[(ptNum & 0xff)*PCG_BUF_UNIT_D2X+i] & colorex;
		}
	} else { // 16x16
		for( i = 0; i < 32*4; i++) { 
			X68_PCG[plNum*PCG_UNIT+i] = vdp->x68_pcg_buffer[(ptNum & 0xfc)*PCG_BUF_UNIT_D2X+i] & colorex;
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
		// ���񔽉f����̂͑�ςȂ̂ŁAVRAM�ɂ��������Ă����A�t���O�𗧂āA�����A�����Ԃɂ܂Ƃ߂���������
		vdp->sprite_refresh_flag |= SPRITE_REFRESH_FLAG_CC;

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


void ms_vdp_sprite_vsync_draw(ms_vdp_t* vdp) {
	if (!vdp->sprite_refresh_flag) {
		return;
	}

	uint8_t* vram = vdp->vram;

	int spSize = vdp->sprite_size == 0 ? 8 : 16;
	int mag512 = (vdp->ms_vdp_current_mode->sprite_mode & 0x80) ? 2 : 1;
	int spMode = vdp->ms_vdp_current_mode->sprite_mode & 0x3;

	int i,j;
	int plNum;
	uint16_t flag = vdp->sprite_refresh_flag;
	if (flag & SPRITE_REFRESH_FLAG_PGEN) {
		// �p�^�[���W�F�l���[�^�e�[�u������PCG�o�b�t�@�̍č\�z���s���܂�
		uint32_t sprpgenaddr = vdp->sprpgentbl_baddr & 0x1fe00; // ����9�r�b�g���N���A
		for(i=0;i<256;i++) {
			for(j=0;j<8;j++) {
				write_sprite_pattern(vdp, i*8+j, vram[sprpgenaddr + i*8 + j]);
			}
		}
		flag |= SPRITE_REFRESH_FLAG_CC;	// CC�t���O�X�V�APCG�X�V�A�X�v���C�g�A�g���r���[�g�e�[�u���̍X�V���s��
	}
	if (flag & SPRITE_REFRESH_FLAG_CC) {
		// �X�v���C�g�J���[�e�[�u����CC�r�b�g�}�b�v�t���O���č쐬
		if (spMode == 2) {
			uint8_t* sprcolr = vram + vdp->sprcolrtbl_baddr;
			for(plNum=0; plNum<32; plNum++) {
				uint16_t ccflag = 0;
				for(i=0; i<16; i++) {
					if (*sprcolr++ & 0x40) { // CC�r�b�g������
						ccflag |= (1 << i);
					}
				}
				sprite_cc_flags[plNum] = ccflag;
			}
		}
		flag |= SPRITE_REFRESH_FLAG_ATTR;	// �X�v���C�g�J���[�e�[�u���̍X�V���s��
	}
	if (flag & SPRITE_REFRESH_FLAG_ATTR) {
		// PCG�X�V����
		if (mag512 == 2 ) {
			// 512�h�b�g���[�h�̎�
			for(i=0;i<32;i++) {
				refresh_sprite_512(vdp, i);
			}
		} else {
			// 256�h�b�g���[�h�̎�
			if (spMode == 1) {
				for(i=0;i<32;i++) {
					refresh_sprite_256_mode1(vdp, i);
				}
			} else {
				refresh_sprite_256_mode2(vdp);
			}
		}
		flag != SPRITE_REFRESH_FLAG_COORD;	// �X�v���C�g�A�g���r���[�g�e�[�u���̍X�V���s��
	}
	if (flag & SPRITE_REFRESH_FLAG_COORD) {
		// �X�v���C�g�A�g���r���[�g�e�[�u���݂̂̍X�V
		uint8_t* sprattr = vram + vdp->sprattrtbl_baddr;
		uint8_t* sprcolr = vram + vdp->sprcolrtbl_baddr;
		uint8_t scroll_offset = vdp->r23; // �c�X�N���[����
		int visible_sprite_planes = 0;
		plNum = 0;
		int x,y,ec = 0;
		if ( (vdp->r08 & 0x01) == 0 ) {
			// �X�v���C�g��\���ł͂Ȃ���
			for(;plNum<32;plNum++) {
				y = sprattr[plNum*SAT_SIZE+0];
				x = sprattr[plNum*SAT_SIZE+1];
				if ( y == 208) {
					// ���̃v���[���ȍ~�͕`�悵�Ȃ�
					break;
				}
				switch(spMode) {
				case 1:
					ec = (sprattr[plNum*SAT_SIZE+3] & 0x80) >> 7;
					break;
				case 2:
					ec = (sprcolr[plNum*COL_SIZE+0] & 0x80) >> 7; // ���C�����Ƃ�EC�̓T�|�[�g���Ȃ��̂�1���C���ڂ����݂�
					break;
				}
				y = ((y + 1 - scroll_offset) & 0xff) * mag512 + 16;
				x = (x * mag512 - ec*32 + 16) & 0x3ff;
				if( mag512 == 2 && spSize == 16) {
					// 512�h�b�g���[�h�A16x16�T�C�Y�̎�
					for( i=0; i<4; i++) {
						X68_SSR[plNum*SSR_UNIT+i*4+0] = x + (i/2)*16;
						X68_SSR[plNum*SSR_UNIT+i*4+1] = y + (i%2)*16;
						X68_SSR[plNum*SSR_UNIT+i*4+2] = 0x100 + plNum*4; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
						X68_SSR[plNum*SSR_UNIT+i*4+3] = 3; // �X�v���C�g�\��
					}
				} else {
					// 256�h�b�g���[�h�̎��A512�h�b�g���[�h��8x8�T�C�Y�̎�
					X68_SSR[plNum*SSR_UNIT+0] = x;
					X68_SSR[plNum*SSR_UNIT+1] = y;
					X68_SSR[plNum*SSR_UNIT+2] = 0x100 + plNum*4; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�					
					X68_SSR[plNum*SSR_UNIT+3] = 3; // �X�v���C�g�\��
				}
			}
		}
		visible_sprite_planes = plNum;
		// �ȉ��̃X�v���C�g�v���[���͔�\���ɂ���
		for(;plNum<32;plNum++) {
			if( mag512 == 2 ) {
				// 512�h�b�g���[�h�̎�
				for( i=0; i<4; i++) {
					X68_SSR[plNum*SSR_UNIT+i*4+3] = 0; // �X�v���C�g��\��
				}
			} else {
				// 256�h�b�g���[�h�̎�
				X68_SSR[plNum*SSR_UNIT+3] = 0; // �X�v���C�g��\��
			}
		}
		vdp->last_visible_sprite_planes = visible_sprite_planes;
		vdp->last_visible_sprite_size = vdp->sprite_size;
	}

	// �t���O�N���A
	vdp->sprite_refresh_flag = 0;
}