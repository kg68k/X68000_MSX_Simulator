#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

/*
	GRAPHIC5 (SCREEN 6) �� 512�~212���[�h�ŁA�h�b�g���Ƃ�4�F��w��\
	GRAPHIC5�̓���@�\:
	* ���ӐF���^�C�����O�����
		MS.X �ł͎��ӐF�̓T�|�[�g���Ă��Ȃ�(�e�L�X�g��ʂ��Ŕw�ʂɎ����Ă����Ύ����ł���̂ŁA�����l����)
	* �X�v���C�g���^�C�����O�����
		MS.X �ł̃X�v���C�g�̃^�C�����O�͏�������
 */

int init_GRAPHIC5(ms_vdp_t* vdp);
uint8_t read_vram_GRAPHIC5(ms_vdp_t* vdp);
void write_vram_GRAPHIC5(ms_vdp_t* vdp, uint8_t data);
void update_palette_GRAPHIC5(ms_vdp_t* vdp);
void update_pname_tbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
void update_colortbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
void update_pgentbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
void update_sprattrtbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
void update_sprpgentbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
void update_text_color_GRAPHIC5(ms_vdp_t* vdp);
void update_back_color_GRAPHIC5(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_GRAPHIC5 = {
	// int init_GRAPHIC5(ms_vdp_t* vdp);
	init_GRAPHIC5,
	// uint8_t read_vram_GRAPHIC5(ms_vdp_t* vdp);
	read_vram_GRAPHIC5,
	// void write_vram_GRAPHIC5(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC5,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC5,
	// void update_pname_tbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
	update_pname_tbl_baddr_GRAPHIC5,
	// void update_colortbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
	update_colortbl_baddr_GRAPHIC5,
	// void update_pgentbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
	update_pgentbl_baddr_GRAPHIC5,
	// void update_sprattrtbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
	update_sprattrtbl_baddr_GRAPHIC5,
	// void update_sprpgentbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr);
	update_sprpgentbl_baddr_GRAPHIC5,
	// void update_text_color_GRAPHIC5(ms_vdp_t* vdp);
	update_text_color_GRAPHIC5,
	// void update_back_color_GRAPHIC5(ms_vdp_t* vdp);
	update_back_color_GRAPHIC5
};


int init_GRAPHIC5(ms_vdp_t* vdp) {
	set_GRAPHIC5_mac();
}

uint8_t read_vram_GRAPHIC5(ms_vdp_t* vdp) {

}

void write_vram_GRAPHIC5(ms_vdp_t* vdp, uint8_t data) {

}

void update_palette_GRAPHIC5(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pname_tbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr) {
    update_pname_tbl_baddr_GRAPHIC4(vdp, addr);
}

void update_colortbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr) {
    update_colortbl_baddr_DEFAULT(vdp, addr);
}

void update_pgentbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr) {
    update_pgentbl_baddr_DEFAULT(vdp, addr);
}

void update_sprattrtbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr) {
    update_sprattrtbl_baddr_DEFAULT(vdp, addr);
}

void update_sprpgentbl_baddr_GRAPHIC5(ms_vdp_t* vdp, uint32_t addr) {
    update_sprpgentbl_baddr_DEFAULT(vdp, addr);
}

void update_text_color_GRAPHIC5(ms_vdp_t* vdp) {

}

void update_back_color_GRAPHIC5(ms_vdp_t* vdp) {

}
