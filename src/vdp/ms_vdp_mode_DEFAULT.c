#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_DEFAULT(ms_vdp_t* vdp);
uint8_t read_vram_DEFAULT(ms_vdp_t* vdp);
void write_vram_DEFAULT(ms_vdp_t* vdp, uint8_t data);
void update_palette_DEFAULT(ms_vdp_t* vdp);
void update_pname_tbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
void update_colortbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
void update_pgentbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
void update_sprattrtbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
void update_sprpgentbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
void update_text_color_DEFAULT(ms_vdp_t* vdp);
void update_back_color_DEFAULT(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_DEFAULT = {
	// int init_DEFAULT(ms_vdp_t* vdp);
	init_DEFAULT,
	// uint8_t read_vram_DEFAULT(ms_vdp_t* vdp);
	read_vram_DEFAULT,
	// void write_vram_DEFAULT(ms_vdp_t* vdp, uint8_t data);
	write_vram_DEFAULT,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_DEFAULT,
	// void update_pname_tbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
	update_pname_tbl_baddr_DEFAULT,
	// void update_colortbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
	update_colortbl_baddr_DEFAULT,
	// void update_pgentbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
	update_pgentbl_baddr_DEFAULT,
	// void update_sprattrtbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
	update_sprattrtbl_baddr_DEFAULT,
	// void update_sprpgentbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr);
	update_sprpgentbl_baddr_DEFAULT,
	// void update_text_color_DEFAULT(ms_vdp_t* vdp);
	update_text_color_DEFAULT,
	// void update_back_color_DEFAULT(ms_vdp_t* vdp);
	update_back_color_DEFAULT
};


int init_DEFAULT(ms_vdp_t* vdp) {
}

uint8_t read_vram_DEFAULT(ms_vdp_t* vdp) {
}

void write_vram_DEFAULT(ms_vdp_t* vdp, uint8_t data) {

}

/*
*
*	�p���b�g�O�̓o�b�N�h���b�v�Ɏg�p���ꂽ���̂ݗL���B�|���a�f�̃p���b�g�P�ɃZ�b�g����B
*	��TODO �o�b�N�h���b�v��BG���g���̂���߂��̂ōl������
*

	lea.l	P_0,a0
	lea.l	$E82220+2,a1			* �a�f�p�̃p���b�g

	move.w	back_color,d0
	add.w	d0,d0
	move.w	0(a0,d0.w),(a1)			* a0 �� P_0 �̃A�h���X�������Ă���

	lea.l	2(a0),a0
	move.w	#15-1,d0
	lea.l	g_palette+2,a1			* �O���t�B�b�N�p�̃p���b�g
	lea.l	t_palette+32+2,a2		* �X�v���C�g�p�̃p���b�g(0x10?0x1f���g�p)
@@:	move.w	(a0),(a1)+
	move.w	(a0)+,(a2)+
	dbra	d0,@b

	rts*/
void update_palette_DEFAULT(ms_vdp_t* vdp) {
	int i;

	for(i =1; i < 16; i++) {
		X68_GR_PAL[i] = vdp->palette[i];
		X68_SP_PAL_B1[i] = vdp->palette[i];
	}
}

void update_pname_tbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr) {
	vdp->pnametbl_baddr = addr;
}

void update_colortbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr) {
	vdp->colortbl_baddr = addr;
}

void update_pgentbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr) {
	vdp->pgentbl_baddr = addr;
}

void update_sprattrtbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr) {
	vdp->sprattrtbl_baddr = addr;
}

void update_sprpgentbl_baddr_DEFAULT(ms_vdp_t* vdp, uint32_t addr) {
	vdp->sprpgentbl_baddr = addr;
}

void update_text_color_DEFAULT(ms_vdp_t* vdp) {

}

void update_back_color_DEFAULT(ms_vdp_t* vdp) {

}
