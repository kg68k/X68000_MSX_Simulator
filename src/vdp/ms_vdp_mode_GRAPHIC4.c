#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ms_vdp.h"

int init_GRAPHIC4(ms_vdp_t* vdp);
uint8_t read_vram_GRAPHIC4(ms_vdp_t* vdp);
void write_vram_GRAPHIC4(ms_vdp_t* vdp, uint8_t data);
void update_palette_GRAPHIC4(ms_vdp_t* vdp);
void update_pnametbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_colortbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_pgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_sprattrtbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_sprpgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
void update_r7_color_GRAPHIC4(ms_vdp_t* vdp, uint8_t data);
char* get_mode_name_GRAPHIC4(ms_vdp_t* vdp);
void update_resolution_GRAPHIC4(ms_vdp_t* vdp);
void vdp_command_exec_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd);
uint8_t vdp_command_read_GRAPHIC4(ms_vdp_t* vdp);
void vdp_command_write_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd);

ms_vdp_mode_t ms_vdp_GRAPHIC4 = {
	// int init_GRAPHIC4(ms_vdp_t* vdp);
	init_GRAPHIC4,
	// uint8_t read_vram_GRAPHIC4(ms_vdp_t* vdp);
	read_vram_GRAPHIC4,
	// void write_vram_GRAPHIC4(ms_vdp_t* vdp, uint8_t data);
	write_vram_GRAPHIC4,
	// void (*update_palette)(ms_vdp_t* vdp);
	update_palette_GRAPHIC4,
	// void update_pnametbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_pnametbl_baddr_GRAPHIC4,
	// void update_colortbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_colortbl_baddr_GRAPHIC4,
	// void update_pgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_pgentbl_baddr_GRAPHIC4,
	// void update_sprattrtbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_sprattrtbl_baddr_GRAPHIC4,
	// void update_sprpgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp);
	update_sprpgentbl_baddr_GRAPHIC4,
	// void update_r7_color_GRAPHIC4(ms_vdp_t* vdp, uint8_t data);
	update_r7_color_GRAPHIC4,
	// char* get_mode_name_GRAPHIC4(ms_vdp_t* vdp);
	get_mode_name_GRAPHIC4,
	// void vdp_command_exec_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_exec_GRAPHIC4,
	// uint8_t vdp_command_read(ms_vdp_t* vdp);
	vdp_command_read_GRAPHIC4,
	// void vdp_command_write(ms_vdp_t* vdp, uint8_t cmd);
	vdp_command_write_GRAPHIC4,
	// void (*update_resolution)(ms_vdp_t* vdp);
	update_resolution_GRAPHIC4,
	// void vsync_draw(ms_vdp_t* vdp);
	vsync_draw_NONE,
	// sprite mode
	2
};


int init_GRAPHIC4(ms_vdp_t* vdp) {
	set_GRAPHIC4_mac();
	update_palette_GRAPHIC4(vdp);
}

uint8_t read_vram_GRAPHIC4(ms_vdp_t* vdp) {
	return read_vram_DEFAULT(vdp);
}

void write_vram_GRAPHIC4(ms_vdp_t* vdp, uint8_t data) {
	w_GRAPHIC4_mac(data);
}

void update_palette_GRAPHIC4(ms_vdp_t* vdp) {
	update_palette_DEFAULT(vdp);
}

void update_pnametbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
	update_pnametbl_baddr_DEFAULT(vdp);
	vdp->pnametbl_baddr &= 0x18000;
	vdp->gr_active = 1 << (vdp->pnametbl_baddr >> 15);
	update_VCRR_02();
}

void update_colortbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
    update_colortbl_baddr_DEFAULT(vdp);
}

void update_pgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
    update_pgentbl_baddr_DEFAULT(vdp);
}

void update_sprattrtbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
    update_sprattrtbl_baddr_DEFAULT(vdp);
}

void update_sprpgentbl_baddr_GRAPHIC4(ms_vdp_t* vdp) {
    update_sprpgentbl_baddr_DEFAULT(vdp);
}

void update_r7_color_GRAPHIC4(ms_vdp_t* vdp, uint8_t data) {
	update_r7_color_DEFAULT(vdp, data);
}

char* get_mode_name_GRAPHIC4(ms_vdp_t* vdp) {
	return "GRAPHIC4";
}

/*
	DX, DY���� VRAM�A�h���X�����߂�

	addr = DY�~128 + dx/2
*/
uint32_t get_vram_add_G4(ms_vdp_t* vdp, uint16_t x, uint16_t y, int* mod) {
	*mod = x & 1;
	return (y&0x3ff)*128 + x/2;
}

uint16_t* to_gram(ms_vdp_t* vdp, uint32_t vaddr, int mod) {
	uint16_t p = (vaddr & 0x18000) >> 15;
	uint16_t y = (vaddr & 0x07f80) >> 7;
	uint16_t x = (vaddr & 0x0007f);		// X���W��1/2�Ȃ̂Œ���
 	return X68_GRAM + (p*512*512) + (y*512) + (x*2) + mod;
}

void _PSET_G4(ms_vdp_t* vdp, uint8_t cmd) {
	int mod;
	uint32_t vaddr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, &mod);
	if (mod) {
		vdp->vram[vaddr] = (vdp->vram[vaddr] & 0xf0) | (vdp->clr & 0x0f);
	} else {
		vdp->vram[vaddr] = (vdp->vram[vaddr] & 0x0f) | (vdp->clr << 4);
	}
	uint16_t* gram = to_gram(vdp, vaddr, mod);
	*gram = vdp->clr;
}

void _LMMC_exe_G4(ms_vdp_t* vdp, uint8_t value);

void _LMMC_G4(ms_vdp_t* vdp, uint8_t cmd, uint8_t logiop) {
	printf("LMMC START G4********\n");

	int mod;
	vdp->cmd_current = cmd;
	vdp->cmd_logiop = logiop;
	vdp->cmd_arg = vdp->arg;
	vdp->cmd_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, &mod);
	vdp->cmd_vram_addr_mod = mod;

	vdp->cmd_nx_count = vdp->nx;
	vdp->cmd_ny_count = vdp->ny;
	vdp->s02 |= 0x01;						// CE�r�b�g���Z�b�g
	_LMMC_exe_G4(vdp, vdp->clr);			// �ŏ���1�h�b�g����������
}

void _LMMC_exe_G4(ms_vdp_t* vdp, uint8_t value) {
	//printf("LMMC exe G4: %02x, nx count=%03x, ny count=%03x\n", value, vdp->cmd_nx_count, vdp->cmd_ny_count);
	if(vdp->cmd_ny_count == 0 && vdp->cmd_nx_count == 0) {
		vdp->s02 &= 0xfe;	// CE�r�b�g���N���A
		vdp->cmd_current = 0;
		return;
	}
	uint8_t logiop = vdp->cmd_logiop;
	uint32_t vaddr = vdp->cmd_vram_addr;
	uint16_t vamod = vdp->cmd_vram_addr_mod;
	uint32_t vaddr_mod = vaddr * 2 + vamod;
	uint16_t* gram = to_gram(vdp, vaddr, vamod);

	uint8_t dst = vdp->vram[vaddr];
	if (vamod == 0) {
		dst >>= 4;
	} else {
		dst &= 0x0f;
	}
	switch(logiop) {
	case 0x0:	// IMP
		dst = value;
		break;
	case 0x1:	// AND
		dst = value & dst;
		break;
	case 0x2:	// OR
		dst = value | dst;
		break;
	case 0x3:	// XOR
		dst = value ^ dst;
		break;
	case 0x4:	// NOT
		dst = !value;
		break;
	case 0x8:	// TIMP
		dst = value == 0 ? dst : value;
		break;
	case 0x9:	// TAND
		dst = value == 0 ? dst : value & dst;
		break;
	case 0xa:	// TOR
		dst = value == 0 ? dst : value | dst;
		break;
	case 0xb:	// TXOR
		dst = value == 0 ? dst : value ^ dst;
		break;
	case 0xc:	// TNOT
		dst = value == 0 ? dst : !value;
		break;
	}
	if (vamod == 0) {
		vdp->vram[vaddr] = (vdp->vram[vaddr] & 0x0f) + (dst << 4);
	} else {
		vdp->vram[vaddr] = (vdp->vram[vaddr] & 0xf0) + (dst & 0xf);
	}

	gram[0] = dst;

	vdp->s02 |= 0x80;				// TR�r�b�g���Z�b�g
	vdp->cmd_nx_count-=1;
	// DIX�ɏ]����VRAM�A�h���X���X�V
	vaddr_mod += (vdp->cmd_arg & 0x4) == 0 ? 1 : -1;
	if(vdp->cmd_nx_count == 0) {
		// 1�s�I������玟�̍s��
		vdp->cmd_ny_count--;
		vdp->ny--;			// NY�͍X�V�����H
		if(vdp->cmd_ny_count > 0) {
			vdp->cmd_nx_count = vdp->nx;
			vaddr_mod += (vdp->cmd_arg & 0x4) == 0 ? vdp->nx * -1 : vdp->nx;
			vaddr_mod += (vdp->cmd_arg & 0x8) == 0 ? 256 : -256;
		} else {
			// �S���I�������CE�r�b�g���N���A
			vdp->s02 &= 0xfe;
			vdp->cmd_current = 0;
		}
	}
	// �㏈��
	vdp->cmd_vram_addr = vaddr_mod / 2;		// VRAM�A�h���X���X�V
	vdp->cmd_vram_addr_mod = vaddr_mod % 2;	// VRAM�A�h���X��1�o�C�g���̈ʒu���X�V
}

void _HMMV_G4(ms_vdp_t* vdp, uint8_t cmd) {
	printf("HMMV START G4********\n");

	int mod;
	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	uint16_t dst_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, &mod);

	int x,y;
	for(y=0;y<vdp->ny;y++) {
		for(x=0;x<vdp->nx;x+=2) {
			uint8_t data = vdp->clr;
			vdp->vram[dst_vram_addr] = data;
			uint16_t* gram = to_gram(vdp, dst_vram_addr, mod);
			gram[0] = data >> 4;
			gram[1] = data & 0x0f;
			// DIX�ɏ]����VRAM�A�h���X���X�V
			if (vdp->cmd_arg & 0x04) {
				dst_vram_addr -= 1;
			} else {
				dst_vram_addr += 1;
			}
		}
		dst_vram_addr += (vdp->cmd_arg & 0x4) == 0 ? (vdp->nx/2) * -1 : (vdp->nx/2);
		dst_vram_addr += (vdp->cmd_arg & 0x8) == 0 ? 128 : -128;
	}
}

void _HMMM_G4(ms_vdp_t* vdp, uint8_t cmd) {
	printf("HMMM START G4********\n");

	int mod;
	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	uint16_t src_vram_addr = get_vram_add_G4(vdp, vdp->sx, vdp->sy, &mod);
	uint16_t dst_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, &mod);

	int x,y;
	for(y=0;y<vdp->ny;y++) {
		for(x=0;x<vdp->nx;x+=2) {
			uint8_t data = vdp->vram[src_vram_addr];
			vdp->vram[dst_vram_addr] = data;
			uint16_t* gram = to_gram(vdp, dst_vram_addr, mod);
			gram[0] = data >> 4;
			gram[1] = data & 0x0f;
			// DIX�ɏ]����VRAM�A�h���X���X�V
			if (vdp->cmd_arg & 0x04) {
				src_vram_addr -= 1;
				dst_vram_addr -= 1;
			} else {
				src_vram_addr += 1;
				dst_vram_addr += 1;
			}
		}
		src_vram_addr += (vdp->cmd_arg & 0x4) == 0 ? (vdp->nx/2) * -1 : (vdp->nx/2);
		src_vram_addr += (vdp->cmd_arg & 0x8) == 0 ? 128 : -128;
		dst_vram_addr += (vdp->cmd_arg & 0x4) == 0 ? (vdp->nx/2) * -1 : (vdp->nx/2);
		dst_vram_addr += (vdp->cmd_arg & 0x8) == 0 ? 128 : -128;
	}
}

void _HMMC_exe_G4(ms_vdp_t* vdp, uint8_t value);
static uint8_t debug_count = 0;

void _HMMC_G4(ms_vdp_t* vdp, uint8_t cmd) {
	printf("HMMC START G4********\n");

	int mod;
	vdp->cmd_current = cmd;
	vdp->cmd_arg = vdp->arg;
	vdp->cmd_vram_addr = get_vram_add_G4(vdp, vdp->dx, vdp->dy, &mod);

	vdp->cmd_nx_count = vdp->nx/2;
	vdp->cmd_ny_count = vdp->ny;
	vdp->s02 |= 0x01;						// CE�r�b�g���Z�b�g
	_HMMC_exe_G4(vdp, vdp->clr);			// �ŏ���1�o�C�g����������
}

void _HMMC_exe_G4(ms_vdp_t* vdp, uint8_t value) {
	//printf("HMMC exe G4: %02x, nx count=%03x, ny count=%03x\n", value, vdp->cmd_nx_count, vdp->cmd_ny_count);
	if(vdp->cmd_ny_count == 0 && vdp->cmd_nx_count == 0) {
		vdp->s02 &= 0xfe;	// CE�r�b�g���N���A
		vdp->cmd_current = 0;
		return;
	}
	uint32_t vaddr = vdp->cmd_vram_addr;
	uint16_t* gram = to_gram(vdp, vaddr, 0);

	//value = debug_count;
	//debug_count = (debug_count + 1) % 16;

	vdp->vram[vaddr] = value; 	// vdp->clr �Ɠ����l�̂͂�
	gram[0] = value >> 4;
	gram[1] = value & 0x0f;

	vdp->s02 |= 0x80;				// TR�r�b�g���Z�b�g
	vdp->cmd_nx_count-=1;
	// DIX�ɏ]����VRAM�A�h���X���X�V
	vaddr += (vdp->cmd_arg & 0x4) == 0 ? 1 : -1;
	if(vdp->cmd_nx_count == 0) {
		// 1�s�I������玟�̍s��
		vdp->cmd_ny_count--;
		vdp->ny--;			// NY�͍X�V�����H
		if(vdp->cmd_ny_count > 0) {
			vdp->cmd_nx_count = vdp->nx/2;
			vaddr += (vdp->cmd_arg & 0x4) == 0 ? (vdp->nx/2) * -1 : (vdp->nx/2);
			vaddr += (vdp->cmd_arg & 0x8) == 0 ? 128 : -128;
		} else {
			// �S���I�������CE�r�b�g���N���A
			vdp->s02 &= 0xfe;
			vdp->cmd_current = 0;
		}
	}
	// �㏈��
	vdp->cmd_vram_addr = vaddr;		// VRAM�A�h���X���X�V
}


void vdp_command_exec_GRAPHIC4(ms_vdp_t* vdp, uint8_t cmd) {
	uint8_t command = (cmd & 0b11110000) >> 4;
	int logiop = cmd & 0b00001111;
	switch(command){
	// case 0b0100: // POINT
	// 	break;
	case 0b0101: // PSET
		_PSET_G4(vdp, command);
		break;
	// case 0b0110: // SRCH
	// 	break;
	// case 0b0111: // LINE
	// 	break;
	// case 0b1000: // LMMV
	// 	break;
	// case 0b1001: // LMMM
	// 	break;
	// case 0b1010: // LMCM
	// 	break;
	case 0b1011: // LMMC
		_LMMC_G4(vdp, command, logiop);
		break;
	case 0b1100: // HMMV
	 	_HMMV_G4(vdp, command);
	 	break;
	 case 0b1101: // HMMM
	 	_HMMM_G4(vdp, command);
	 	break;
	// case 0b1110: // YMMC
	// 	break;
	case 0b1111: // HMMC
		_HMMC_G4(vdp, command);
		break;
	default:
		vdp_command_exec_DEFAULT(vdp, cmd);
	}
	return;
}

uint8_t vdp_command_read_GRAPHIC4(ms_vdp_t* vdp) {
	return 0;
}

void vdp_command_write_GRAPHIC4(ms_vdp_t* vdp, uint8_t value) {
	switch(vdp->cmd_current) {
	case 0b0100: // POINT
		break;
	case 0b0101: // PSET
		break;
	case 0b0110: // SRCH
		break;
	case 0b0111: // LINE
		break;
	case 0b1000: // LMMV
		break;
	case 0b1001: // LMMM
		break;
	case 0b1010: // LMCM
		break;
	case 0b1011: // LMMC
		_LMMC_exe_G4(vdp, value);
		break;
	case 0b1100: // HMMV
		break;
	case 0b1101: // HMMM
		break;
	case 0b1110: // YMMC
		break;
	case 0b1111: // HMMC
		_HMMC_exe_G4(vdp, value);
		break;
	default:
		break;
	}
}

void update_resolution_GRAPHIC4(ms_vdp_t* vdp) {
	update_resolution_COMMON(vdp, 0, 0, 0); // 256, 16�F, BG�s�g�p
}