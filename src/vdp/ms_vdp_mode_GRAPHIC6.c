#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

/*
	GRAPHIC6 (SCREEN 7) は 512×212モードで、ドットごとに16色画指定可能
 */

int init_GRAPHIC6(ms_vdp_t* vdp);
uint8_t read_vram_GRAPHIC6(ms_vdp_t* vdp);
void write_vram_GRAPHIC6(ms_vdp_t* vdp, uint8_t data);
void update_palette_GRAPHIC6(ms_vdp_t* vdp);
void update_pnametbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
void update_colortbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
void update_pgentbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
void update_text_color_GRAPHIC6(ms_vdp_t* vdp);
void update_back_color_GRAPHIC6(ms_vdp_t* vdp);

ms_vdp_mode_t ms_vdp_GRAPHIC6 = {
	// int init_GRAPHIC6(ms_vdp_t* vdp);
	init_GRAPHIC6,
	// uint8_t read_vram_GRAPHIC6(ms_vdp_t* vdp);
	read_vram_GRAPHIC6,
	// void write_vram_GRAPHIC6(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC6,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC6,
	// void update_pnametbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
	update_pnametbl_baddr_GRAPHIC6,
	// void update_colortbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
	update_colortbl_baddr_GRAPHIC6,
	// void update_pgentbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
	update_pgentbl_baddr_GRAPHIC6,
	// void update_sprattrtbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_GRAPHIC6,
	// void update_sprpgentbl_baddr_GRAPHIC6(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_GRAPHIC6,
	// void update_text_color_GRAPHIC6(ms_vdp_t* vdp);
	update_text_color_GRAPHIC6,
	// void update_back_color_GRAPHIC6(ms_vdp_t* vdp);
	update_back_color_GRAPHIC6
};


int init_GRAPHIC6(ms_vdp_t* vdp) {
	set_GRAPHIC6_mac();
}

uint8_t read_vram_GRAPHIC6(ms_vdp_t* vdp) {

}

void write_vram_GRAPHIC6(ms_vdp_t* vdp, uint8_t data) {

}

void update_palette_GRAPHIC6(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_GRAPHIC6(ms_vdp_t* vdp) {
	update_pnametbl_baddr_DEFAULT(vdp);
	vdp->pnametbl_baddr &= 0x10000;
	vdp->gr_active = 1 << (vdp->pnametbl_baddr >> 16);
	update_VCRR_02();
}

void update_colortbl_baddr_GRAPHIC6(ms_vdp_t* vdp) {
    update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_GRAPHIC6(ms_vdp_t* vdp) {
    update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_GRAPHIC6(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC6(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_text_color_GRAPHIC6(ms_vdp_t* vdp) {

}

void update_back_color_GRAPHIC6(ms_vdp_t* vdp) {

}
