#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_memmap.h"


typedef struct ms_memmap_driver_MEGAROM_KONAMI_SCC  ms_memmap_driver_MEGAROM_KONAMI_SCC_t;

int init_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* memmap, char* filename, int argc, char* argv[]);
void deinint_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* memmap);
uint8_t read8_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* memmap, uint16_t addr);
uint16_t read16_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* memmap, uint16_t addr);
void write8_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* memmap, uint16_t addr, uint8_t data);
void write16_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* memmap, uint16_t addr, uint16_t data);

void _select_bank_KONAMI_SCC(ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d, int rom_page, int bank);

/*
	�m�� & ���������[�`��
 */
ms_memmap_driver_t* ms_memmap_MEGAROM_KONAMI_SCC_init(ms_memmap_t* memmap, const uint8_t* buffer, uint32_t length) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* instance;
	instance = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)new_malloc(sizeof(ms_memmap_driver_MEGAROM_KONAMI_SCC_t));
	if (instance == NULL) {
		return NULL;
	}
	instance->base.memmap = memmap;
	instance->base.read8 = read8_MEGAROM_KONAMI_SCC;
	instance->base.read16 = read16_MEGAROM_KONAMI_SCC;
	instance->base.write8 = write8_MEGAROM_KONAMI_SCC;
	instance->base.write16 = write16_MEGAROM_KONAMI_SCC;

	uint8_t* buf;
	buf = (uint8_t*)new_malloc( 16*1024 + MS_MEMMAP_HEADER_LENGTH );
	if(buf == NULL) {
		printf("���������m�ۂł��܂���B\n");
		deinint_MEGAROM_KONAMI_SCC((ms_memmap_driver_t*)instance);
		return NULL;
	}
	instance->base.mem_slot1 = buf;
	
	buf = (uint8_t*)new_malloc( 16*1024 + MS_MEMMAP_HEADER_LENGTH );
	if(buf == NULL) {
		printf("���������m�ۂł��܂���B\n");
		return NULL;
	}
	instance->base.mem_slot2 = buf;

	//
	instance->base.buffer = (uint8_t*)buffer;
	instance->bank_size = length / 0x2000;
	int rom_page;
	for(rom_page=0;rom_page<4;rom_page++) {
		_select_bank_KONAMI_SCC(instance, rom_page, rom_page);	// KONAMI_SCC���K�����̏ꍇ�A�����l��0,1,2,3
	}
	return (ms_memmap_driver_t*)instance;
}

void deinint_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
	new_free(d->base.buffer);
	new_free(d->base.mem_slot2);
	new_free(d->base.mem_slot1);
	new_free(d);
}

void _select_bank_KONAMI_SCC(ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d, int rom_page, int bank) {
	if ( bank >= d->bank_size) {
		printf("MEGAROM_KONAMI_SCC: bank out of range: %d\n", bank);
		return;
	}
	d->selected_bank[rom_page] = bank;
	// �o���N�؂�ւ�����(�������R�s�[)
	int i;
	for(i=0;i<0x2000;i++) {
		if(rom_page < 2) {
			d->base.mem_slot1[MS_MEMMAP_HEADER_LENGTH + ((rom_page%2)*0x2000) + i] = d->base.buffer[bank*0x2000 + i];
		} else {
			d->base.mem_slot2[MS_MEMMAP_HEADER_LENGTH + ((rom_page%2)*0x2000) + i] = d->base.buffer[bank*0x2000 + i];
		}
	}
	if(0) {
		printf("MEGAROM_KONAMI_SCC: bank %d selected for rom page %d\n", bank, rom_page);
		if(0) {
			for (i = 0; i < 4; i++)
			{
				printf(" rom page%d: %02x\n", i, d->selected_bank[i]);	
			}
		}
	}
	return;
}


uint8_t read8_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
	int slot_page = addr >> 13;
	if( slot_page < 2 || slot_page > 5) {
		printf("MEGAROM_KONAMI_SCC: read out of range: %04x\n", addr);
		return 0xff;
	}
	slot_page -= 2;
	int bank = d->selected_bank[slot_page];
	if (bank > d->bank_size) {
		printf("MEGAROM_KONAMI_SCC: bank out of range: %d\n", bank);
		return 0xff;
	}
	int long_addr = (addr & 0x1fff) + (0x2000 * bank);
	uint8_t ret = driver->buffer[long_addr];
	//printf("MEGAROM_KONAMI_SCC: read %04x[%06x] -> %02x\n", addr, long_addr, ret);
	return ret;
}

uint16_t read16_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver, uint16_t addr) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
	return read8_MEGAROM_KONAMI_SCC(driver, addr) | (read8_MEGAROM_KONAMI_SCC(driver, addr + 1) << 8);
}

/*
	SCC������KONAMI_SCC���K�����̐؂�ւ�����
	https://www.msx.org/wiki/MegaROM_Mappers#Konami_MegaROMs_with_SCC

	* 4000h~5FFFh (mirror: C000h~DFFFh)
		* �؂�ւ��A�h���X:	5000h (mirrors: 5001h~57FFh)
		* �����Z�O�����g	0
	* 6000h~7FFFh (mirror: E000h~FFFFh)
		* �؂�ւ��A�h���X	7000h (mirrors: 7001h~77FFh)
		* �����Z�O�����g	1
	* 8000h~9FFFh (mirror: 0000h~1FFFh)
		* �؂�ւ��A�h���X	9000h (mirrors: 9001h~97FFh)
		* �����Z�O�����g	Random
	* A000h~BFFFh (mirror: 2000h~3FFFh)
		* �؂�ւ��A�h���X	b000h (mirrors: B001h~B7FFh)
		* �����Z�O�����g	Random
 */
void write8_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver, uint16_t addr, uint8_t data) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
	// �o���N�؂�ւ�����
	int rom_page = -1;
	int area = addr >> 11;
	switch(area) {
		case 0x5*2:
			break;
		case 0x7*2:
			rom_page = 1;
			break;
		case 0x9*2:
			rom_page = 2;
			break;
		case 0xb*2:
			rom_page = 3;
			break;
	}
	if (rom_page != -1) {
		_select_bank_KONAMI_SCC(d, rom_page, data);
	}
	return;
}

void write16_MEGAROM_KONAMI_SCC(ms_memmap_driver_t* driver, uint16_t addr, uint16_t data) {
	ms_memmap_driver_MEGAROM_KONAMI_SCC_t* d = (ms_memmap_driver_MEGAROM_KONAMI_SCC_t*)driver;
	write8_MEGAROM_KONAMI_SCC(driver, addr + 0, data & 0xff);
	write8_MEGAROM_KONAMI_SCC(driver, addr + 1, data >> 8);
	return;
}
