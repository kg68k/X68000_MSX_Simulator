#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

/*
	GRAPHIC7 (SCREEN 8) �� 256�~212���[�h�ŁA�h�b�g���Ƃ�256�F��w��\
 */

int init_GRAPHIC7(ms_vdp_t* vdp);
uint8_t read_vram_GRAPHIC7(ms_vdp_t* vdp);
void write_vram_GRAPHIC7(ms_vdp_t* vdp, uint8_t data);
void update_palette_GRAPHIC7(ms_vdp_t* vdp);
void update_pnametbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_colortbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_pgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
void update_r7_color_GRAPHIC7(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_GRAPHIC7(ms_vdp_t* vdp);
void update_resolution_GRAPHIC7(ms_vdp_t* vdp);
void vdp_command_exec_GRAPHIC7(ms_vdp_t* vdp, uint8_t cmd);
uint8_t vdp_command_read_GRAPHIC7(ms_vdp_t* vdp);
void vdp_command_write_GRAPHIC7(ms_vdp_t* vdp, uint8_t cmd);

ms_vdp_mode_t ms_vdp_GRAPHIC7 = {
	// int init_GRAPHIC7(ms_vdp_t* vdp);
	init_GRAPHIC7,
	// uint8_t read_vram_GRAPHIC7(ms_vdp_t* vdp);
	read_vram_GRAPHIC7,
	// void write_vram_GRAPHIC7(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC7,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC7,
	// void update_pnametbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_pnametbl_baddr_GRAPHIC7,
	// void update_colortbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_colortbl_baddr_GRAPHIC7,
	// void update_pgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_pgentbl_baddr_GRAPHIC7,
	// void update_sprattrtbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_GRAPHIC7,
	// void update_sprpgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_GRAPHIC7,
	// void update_r7_color_GRAPHIC7(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_GRAPHIC7,
	// char* get_mode_name_GRAPHIC7(ms_vdp_t* vdp);
	get_mode_name_GRAPHIC7,
	// void vdp_command_exec_GRAPHIC7(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_exec_GRAPHIC7,
	// uint8_t vdp_command_read(ms_vdp_t* vdp);
	vdp_command_read_GRAPHIC7,
	// void vdp_command_write(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_write_GRAPHIC7,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_GRAPHIC7,
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_NONE,
	// sprite mode
	2,
	// crt_width
	256,
	// dots_per_byte
	1,
	// bits_per_dot
	8
};


int init_GRAPHIC7(ms_vdp_t* vdp) {
	set_GRAPHIC7_mac();
	ms_vdp_update_sprite_area(vdp);
	update_palette_GRAPHIC7(vdp);
	return 0;
}

uint8_t read_vram_GRAPHIC7(ms_vdp_t* vdp) {
	return read_vram_DEFAULT(vdp);
}

void write_vram_GRAPHIC7(ms_vdp_t* vdp, uint8_t data) {
	w_GRAPHIC7_mac(data);
}

void update_palette_GRAPHIC7(ms_vdp_t* vdp) {
	// MSX���̃p���b�g�͂Ȃ��̂ŁA
	// X68000�� 256�F�p���b�g�� MSX��256�F�Ɋ��蓖�Ă�
	int i,r,g,b;
	uint16_t color;
	for(i = 0;i<256;i++) {
		// 5bit RGB
		int g = (i & 0b11100000) >> 3;
		int r = (i & 0b00011100);
		int b = (i & 0b00000011) << 3;
		g = g | (g>>3);
		r = r | (r>>3);
		b = b | (b>>2) | (b>>4);
		color = (g << 11) | (r << 6) | b << 1;
		if(vdp->tx_active) {
			// �P�x�𔼕��ɗ��Ƃ�
			color &= 0b1111011110111100;
			color >>= 1;
		}
		X68_GR_PAL[i] = color;
	}
}

void update_pnametbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
	vdp->pnametbl_baddr = (vdp->_r02 << 11) & 0x10000;
	// 256�F���[�h�̏ꍇ��2��ʂ����Ȃ����A���̎��́A(b3,b2), (b1,b0) ���y�A�ŃZ�b�g���܂�
	vdp->gr_active = 0x3 << (vdp->pnametbl_baddr >> 15);
	switch(vdp->pnametbl_baddr >> 16) {
	case 0:
		vdp->gr_active = 0b0011;
		vdp->gr_active_interlace = 0b0011;		// GRAPHIC7�̃C���^�[���[�X���[�h�͂܂����Ή�
		break;
	case 1:
		vdp->gr_active = 0b1100;
		vdp->gr_active_interlace = 0b1100;		// GRAPHIC7�̃C���^�[���[�X���[�h�͂܂����Ή�
		break;
	default:
		vdp->gr_active = 0b0011;
		vdp->gr_active_interlace = 0b0011;		// GRAPHIC7�̃C���^�[���[�X���[�h�͂܂����Ή�
		break;
	}
	ms_vdp_update_visibility(vdp);
}

void update_colortbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
	update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
	update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
	update_sprattrtbl_baddr_MODE2(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC7(ms_vdp_t* vdp) {
	update_sprpgentbl_baddr_MODE2(vdp);
}

void update_r7_color_GRAPHIC7(ms_vdp_t* vdp, uint8_t data) {
	update_r7_color_DEFAULT(vdp, data);
}

char* get_mode_name_GRAPHIC7(ms_vdp_t* vdp) {
	return "GRAPHIC7";
}

void vdp_command_exec_GRAPHIC7(ms_vdp_t* vdp, uint8_t cmd) {
	vdp_command_exec(vdp, cmd);
}

uint8_t vdp_command_read_GRAPHIC7(ms_vdp_t* vdp) {
	vdp_command_read(vdp);
}

void vdp_command_write_GRAPHIC7(ms_vdp_t* vdp, uint8_t value) {
	vdp_command_write(vdp, value);
}

void update_resolution_GRAPHIC7(ms_vdp_t* vdp) {
	ms_vdp_update_resolution_COMMON(vdp, 0, 1, 0); // 256, 256�F, BG�s�g�p
}