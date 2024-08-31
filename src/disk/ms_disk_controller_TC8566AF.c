/*
	TC8566AF��I/O

	Address	R/W	Feature
	0x3FF8	W	���W�X�^2 ���X�V (write only)
	0x3FF9	W	���W�X�^3 ���X�V (write only)
	0x3FFA	R/W	���W�X�^4 ���Q�ƁE�X�V
	0x3FFB	R/W	���W�X�^5 ���Q�ƁE�X�V

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include "ms_disk_controller_TC8566AF.h"
#include "../memmap/ms_memmap.h"

#define THIS ms_disk_controller_TC8566AF_t

THIS* ms_disk_controller_TC8566AF_alloc() {
	return (THIS*)new_malloc(sizeof(ms_disk_controller_TC8566AF_t));
}

void ms_disk_controller_TC8566A_init(ms_disk_controller_TC8566AF_t* instance, ms_disk_container_t* container) {
	ms_disk_drive_floppy_init(&instance->drive, container);

	instance->status0 = 0;
	instance->status1 = 0;
	instance->status2 = 0;
}

void ms_disk_controller_TC8566A_deinit(ms_disk_controller_TC8566AF_t* instance) {

}


/**
 * @brief 
 * 
 * 0x3FF8 - ���W�X�^2 - �h���C�u�I�� (Write Only)
 * Bit-7	Bit-6	Bit-5	Bit-4	Bit-3	Bit-2	Bita-1	Bit-0
 * ?		?		LED2	LED1	?		DriveId	DriveId	DriveId
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg2_write(THIS* d, uint8_t data) {
	d->led1 = (data >> 4) & 0x01;
	d->led2 = (data >> 5) & 0x01;
	d->driveId = data & 0x07;
}

/**
 * @brief TC8566AF�̃��W�X�^3�ɏ�������
 * 
 * �����N����Ȃ�
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg3_write(THIS* d, uint8_t data) {
}

/**
 * @brief TC8566AF�̃��W�X�^4��ǂ�
 * 
 * �X�e�[�^�X���W�X�^
 * Bit-7	Bit-6	Bit-5	Bit-4	Bit-3	Bit-2	Bit-1	Bit-0
 * RQM		DIO		NDM		CB		D3B		D2B		D1B		D0B
 * 
 * RQM; Request for Master
 *	1: FDC���f�[�^�󂯕t�����A�܂��̓f�[�^���M��������
 *	0: FDC���f�[�^�󂯎��s�A�܂��̓f�[�^���M����������
 * DIO; Data Input/Output
 *	1: FDC���z�X�g�����]��
 *	0: �z�X�g��FDC�����]��
 * NDM; NonDMA Mode
 *	1: FDC��NonDMA���[�h�Ńf�[�^�]�����ŃT�[�r�X��v�����Ă���
 *	0: FDC�̃T�[�r�X�v���Ȃ�
 * CB; FDC Busy
 *	1: FDC���R�}���h�����s��
 *	0: FDC�̓R�}���h���󂯕t���\
 * D3B?D0B; FDD 3~0 Busy
 *	1: FDD 0?3 ���V�[�N��
 * 	0: FDD 0?3 ���V�[�N���ł͂Ȃ�
 * @param d 
 * @return uint8_t 
 */
uint8_t _TC8556AF_reg4_read(THIS* d) {
	int request_for_master = 1;
	int data_input_output = 0;
	int non_dma_mode = 0;
	int fdc_busy = 0;
	int fdd_busy = 0;
	switch(d->phase) {
	case TC8566AF_PHASE_IDLE:
		// Perform idle operation
		printf("FDC(Idle): Rd#4:");
		break;
	case TC8566AF_PHASE_COMMAND:
		printf("FDC(Comm): CMD:%02x Rd#4:", d->command);
		// Perform d->command operation
		fdc_busy = 1;
		break;
	case TC8566AF_PHASE_DATA_TRANSFER:
		printf("FDC(Trns): CMD:%02x Rd#4:", d->command);
		fdc_busy = 1;
		request_for_master = 0;
		switch(d->command) {
		case TC8566AF_CMD_READ_DATA:
			// Perform read data operation
			data_input_output = 1; // FDC -> Host
		break;
		case TC8566AF_CMD_WRITE_DATA:
			// Perform write data operation
			data_input_output = 0; // Host -> FDC
			break;
		case TC8566AF_CMD_WRITE_DELETED_DATA:
			// Perform write deleted data operation
			data_input_output = 0; // Host -> FDC
			break;
		case TC8566AF_CMD_READ_DELETED_DATA:
			// Perform read deleted data operation
			data_input_output = 1; // FDC -> Host
			break;
		case TC8566AF_CMD_READ_DIAGNOSTIC:
			// Perform read diagnostic operation
			data_input_output = 1; // FDC -> Host
			break;
		case TC8566AF_CMD_READ_ID:
			// Perform read ID operation
			data_input_output = 1; // FDC -> Host
			break;
		case TC8566AF_CMD_FORMAT:
			// Perform format operation
			data_input_output = 0; // Host -> FDC
			break;
		case TC8566AF_CMD_SCAN_EQUAL:
			// Perform scan equal operation
			data_input_output = 0; // Host -> FDC �z�X�g���珑�����񂾂��̂ƃf�B�X�N�̓��e����v���Ă��邩���r����
			break;
		case TC8566AF_CMD_SCAN_LOW_OR_EQUAL:
			// Perform scan low or equal operation
			data_input_output = 0; // Host -> FDC �z�X�g���珑�����񂾂��̂ƃf�B�X�N�̓��e����v���Ă��邩���r����
			break;
		case TC8566AF_CMD_SCAN_HIGH_OR_EQUAL:
			// Perform scan high or equal operation
			data_input_output = 0; // Host -> FDC �z�X�g���珑�����񂾂��̂ƃf�B�X�N�̓��e����v���Ă��邩���r����
			break;
		case TC8566AF_CMD_SEEK:
			// Perform seek operation
			// TODO SEEK�̌������킩��Ȃ�
			break;
		case TC8566AF_CMD_RECALIBRATE:
			// Perform recalibrate operation
			// TODO SEEK�̌������킩��Ȃ�
			break;
		default:
			printf("invalid\n");
			break;
		}
		break;
	case TC8566AF_PHASE_RESULT:
		printf("FDC(Resl): CMD:%02x Rd#4:", d->command);
		fdc_busy = 0;
		request_for_master = 1;
		data_input_output = 1; // FDC -> Host
		switch(d->command) {
		case TC8566AF_CMD_READ_DATA:
			// Perform read data operation
			break;
		case TC8566AF_CMD_WRITE_DATA:
			// Perform write data operation
			break;
		case TC8566AF_CMD_WRITE_DELETED_DATA:
			// Perform write deleted data operation
			break;
		case TC8566AF_CMD_READ_DELETED_DATA:
			// Perform read deleted data operation
			break;
		case TC8566AF_CMD_READ_DIAGNOSTIC:
			// Perform read diagnostic operation
			break;
		case TC8566AF_CMD_READ_ID:
			// Perform read ID operation
			request_for_master = 0;
			break;
		case TC8566AF_CMD_FORMAT:
			// Perform format operation
			break;
		case TC8566AF_CMD_SCAN_EQUAL:
			// Perform scan equal operation
			break;
		case TC8566AF_CMD_SCAN_LOW_OR_EQUAL:
			// Perform scan low or equal operation
			break;
		case TC8566AF_CMD_SCAN_HIGH_OR_EQUAL:
			// Perform scan high or equal operation
			break;
		case TC8566AF_CMD_SEEK:
			// Perform seek operation
			break;
		case TC8566AF_CMD_RECALIBRATE:
			// Perform recalibrate operation
			break;
		case TC8566AF_CMD_SENSE_INTERRUPT_STATUS:
			// Perform sense interrupt status operation
			break;
		case TC8566AF_CMD_SPECIFY:
			// Perform specify operation
			break;
		case TC8566AF_CMD_SENSE_DEVICE_STATUS:
			// Perform sense device status operation
			break;
		}
	}

	uint8_t ret = (request_for_master << 7) | (data_input_output << 6) | (non_dma_mode << 5) | (fdc_busy << 4) | (fdd_busy & 0x3);
	printf("%02x\n", ret);
	return ret;
}

/**
 * @brief TC8566AF�̃��W�X�^4�ɏ�������
 * 
 * �����N����Ȃ�
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg4_write(THIS* d, uint8_t data) {
}

/**
 * @brief TC8566AF�̃��W�X�^5��ǂ�
 * 
 * �f�[�^���W�X�^�ł�
 * 
 * @param d 
 * @return uint8_t 
 */
uint8_t _TC8556AF_reg5_read(THIS* d) {
	uint8_t ret = 0;
	switch(d->phase) {
	case TC8566AF_PHASE_IDLE:
		printf("FDC(Idle):          Rd#5:");
		break;
	case TC8566AF_PHASE_COMMAND:
		printf("FDC(Comm): CMD:%02x Rd#5:", d->command);
		break;
	case TC8566AF_PHASE_DATA_TRANSFER:
		printf("FDC(Trns): CMD:%02x Rd#5:", d->command);
		if(d->transfer_datas_rest == 0) {
			printf("Illegal state\n");
			d->transfer_datas_rest = 1;
		}
		d->transfer_datas_rest--;
		if (d->transfer_datas_rest == 0) {
			d->result_datas_rest = 7;
			d->phase = TC8566AF_PHASE_RESULT;
		}
		break;
	case TC8566AF_PHASE_RESULT:	
		printf("FDC(Resl): CMD:%02x Rd#5:", d->command);
		switch(d->command) {
		case TC8566AF_CMD_READ_DATA:
			// Perform read data operation
			break;
		case TC8566AF_CMD_WRITE_DATA:
			// Perform write data operation
			break;
		case TC8566AF_CMD_WRITE_DELETED_DATA:
			// Perform write deleted data operation
			break;
		case TC8566AF_CMD_READ_DELETED_DATA:
			// Perform read deleted data operation
			break;
		case TC8566AF_CMD_READ_DIAGNOSTIC:
			// Perform read diagnostic operation
			break;
		case TC8566AF_CMD_READ_ID:
			// Perform read ID operation
			break;
		case TC8566AF_CMD_FORMAT:
			// Perform format operation
			break;
		case TC8566AF_CMD_SCAN_EQUAL:
			// Perform scan equal operation
			break;
		case TC8566AF_CMD_SCAN_LOW_OR_EQUAL:
			// Perform scan low or equal operation
			break;
		case TC8566AF_CMD_SCAN_HIGH_OR_EQUAL:
			// Perform scan high or equal operation
			break;
		case TC8566AF_CMD_SEEK:
			// Perform seek operation
			break;
		case TC8566AF_CMD_RECALIBRATE:
			// Perform recalibrate operation
			break;
		case TC8566AF_CMD_SENSE_INTERRUPT_STATUS:
			// Perform sense interrupt status operation
			switch(d->result_datas_rest) {
			case 2:
				ret = d->status0;
				d->status0 = 0; // reset
				break;
			case 1:
				ret = 0; // TODO
				break;
			}
			break;
		case TC8566AF_CMD_SPECIFY:
			// Perform specify operation
			break;
		case TC8566AF_CMD_SENSE_DEVICE_STATUS:
			// Perform sense device status operation
			ret = d->status3;
			break;
		}
		d->result_datas_rest--;
		if (d->result_datas_rest == 0) {
			d->phase = TC8566AF_PHASE_IDLE;
		}
		break;
	}
	printf("%02x\n", ret);
	return ret;
}

/**
 * @brief TC8566AF�̃��W�X�^5�ɏ�������
 * 
 * �f�[�^���W�X�^�ł��BTC8566AF�̃X�e�[�g(Phase)�ɂ���ē��삪�قȂ�܂��B
 * 
 * ## 1. Idle Phase
 * �A�C�h���t�F�[�Y�ł͎��s����R�}���h�̎�ނ��������݂܂��B
 * 
 * if ((value & 0x1f) == 0x06) d->command = CMD_READ_DATA;
 * if ((value & 0x3f) == 0x05) d->command = CMD_WRITE_DATA;
 * if ((value & 0x3f) == 0x09) d->command = CMD_WRITE_DELETED_DATA;
 * if ((value & 0x1f) == 0x0c) d->command = CMD_READ_DELETED_DATA;
 * if ((value & 0xbf) == 0x02) d->command = CMD_READ_DIAGNOSTIC;
 * if ((value & 0xbf) == 0x0a) d->command = CMD_READ_ID;
 * if ((value & 0xbf) == 0x0d) d->command = CMD_FORMAT;
 * if ((value & 0x1f) == 0x11) d->command = CMD_SCAN_EQUAL;
 * if ((value & 0x1f) == 0x19) d->command = CMD_SCAN_LOW_OR_EQUAL;
 * if ((value & 0x1f) == 0x1d) d->command = CMD_SCAN_HIGH_OR_EQUAL;
 * if ((value & 0xff) == 0x0f) d->command = CMD_SEEK;
 * if ((value & 0xff) == 0x07) d->command = CMD_RECALIBRATE;
 * if ((value & 0xff) == 0x08) d->command = CMD_SENSE_INTERRUPT_STATUS;
 * if ((value & 0xff) == 0x03) d->command = CMD_SPECIFY;
 * if ((value & 0xff) == 0x04) d->command = CMD_SENSE_DEVICE_STATUS;
 *
 * ## 2. d->command Phase
 * �R�}���h�t�F�[�Y�ł́A�R�}���h�ɕK�v�ȃp�����[�^���������݂܂��B�R�}���h�̎�ނɂ���ĉ��񂩌J��Ԃ�write�j
 * 
 * ## 3. Result Phase
 * �f�[�^�ǂݍ��� or �������݂��s���t�F�[�Y�ł�
 * 
 * ���ۂ�FDC�́ACommand Phase�̌� Execution Phase������A�����Ńf�B�X�N�ւ̃A�N�Z�X���s���܂����A
 * �G�~�����[�^�͂����̑҂����Ԃ��l������K�v���Ȃ��̂ŁA�ȗ����Ă��܂��B
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg5_write(THIS* d, uint8_t value) {
	switch(d->phase) {
	case TC8566AF_PHASE_IDLE:
		printf("FDC(Idle): Wr#5: %02x\n", value);
		d->command_params_index = 0;
		d->command_params_rest = 0;
		d->result_datas_rest = 0;
		if ((value & 0x1f) == 0x06) {
			d->command = TC8566AF_CMD_READ_DATA;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0x3f) == 0x05) {
			d->command = TC8566AF_CMD_WRITE_DATA;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0x3f) == 0x09) {
			d->command = TC8566AF_CMD_WRITE_DELETED_DATA;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0x1f) == 0x0c) {
			d->command = TC8566AF_CMD_READ_DELETED_DATA;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0xbf) == 0x02) {
			d->command = TC8566AF_CMD_READ_DIAGNOSTIC;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, dtl
		} else if ((value & 0xbf) == 0x0a) {
			d->command = TC8566AF_CMD_READ_ID;
			d->command_params_rest = 1;	// flag
		} else if ((value & 0xbf) == 0x0d) {
			d->command = TC8566AF_CMD_FORMAT;
			d->command_params_rest = 5;	// flag, n, sc, gpl, d
		} else if ((value & 0x1f) == 0x11) {
			d->command = TC8566AF_CMD_SCAN_EQUAL;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, stp
		} else if ((value & 0x1f) == 0x19) {
			d->command = TC8566AF_CMD_SCAN_LOW_OR_EQUAL;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, stp
		} else if ((value & 0x1f) == 0x1d) {
			d->command = TC8566AF_CMD_SCAN_HIGH_OR_EQUAL;
			d->command_params_rest = 8;	// flag, c, h, r, n, eot, gpl, stp
		} else if ((value & 0xff) == 0x0f) {
			d->command = TC8566AF_CMD_SEEK;
			d->command_params_rest = 2;	// flag, ncn
		} else if ((value & 0xff) == 0x07) {
			d->command = TC8566AF_CMD_RECALIBRATE;
			d->command_params_rest = 1;	// flag
		} else if ((value & 0xff) == 0x08) {
			d->command = TC8566AF_CMD_SENSE_INTERRUPT_STATUS;
			d->command_params_rest = 0;
			d->result_datas_rest = 2;
			d->phase = TC8566AF_PHASE_RESULT; // Command Phase, Execution Phase ���Ȃ��̂Œ���Result Phase��
			return;
		} else if ((value & 0xff) == 0x03) {
			d->command = TC8566AF_CMD_SPECIFY;
			d->command_params_rest = 2;	// srt/hut, hlt/nd
		} else if ((value & 0xff) == 0x04) {
			d->command = TC8566AF_CMD_SENSE_DEVICE_STATUS;
			d->command_params_rest = 1;	// flag
		} else {
			// invalid command
			d->command = TC8566AF_CMD_INVALID;
			d->phase = TC8566AF_PHASE_RESULT; // Command Phase, Execution Phase ���Ȃ��̂Œ���Result Phase��
			return;
		}

		// Command Phase �ֈڍs
		d->phase = TC8566AF_PHASE_COMMAND;
		if (d->command_params_rest == 0) {
			// �ُ���
			printf("Command Phase �� command_params_rest �� 0 �ł�\n");
		}
		break;
	case TC8566AF_PHASE_COMMAND:
		printf("FDC(Comm): Wr#5: %02x\n", value);
		d->command_params[d->command_params_index++] = value;
		d->command_params_rest--;
		if (d->command_params_rest == 0) {
			switch(d->command) {
			case TC8566AF_CMD_READ_DATA:
				d->transfer_datas_rest = 128 << d->command_params[4] & 0x07; // CHRN��N�ɂ���ĕς��
				break;
			case TC8566AF_CMD_SPECIFY:
				// Result Phase�Ȃ��ŏI��
				printf("FDC: Comm to Idle phase\n");
				d->phase = TC8566AF_PHASE_IDLE;
				break;
			case TC8566AF_CMD_RECALIBRATE:
				// TODO: ���L�����u���[�g�������Ŏ��s
				d->status0 |= 0x20; // Seek End
				d->status0 |= 0x08; // Not ready (�f�[�^���ǂ߂�悤�ɂȂ�܂ł́ANot Ready�Ō떂����
				printf("FDC: Comm to Idle phase\n");
				d->phase = TC8566AF_PHASE_IDLE;
				break;
			case TC8566AF_CMD_SENSE_DEVICE_STATUS:
				// TODO: �f�o�C�X�X�e�[�^�X���s
				d->status3 = (d->command_params[0] & 0x07) | //
					(1 * 0x08) | // Two side
					(1 * 0x10) | // Track 0
					(1 * 0x20) | // Ready
					(1 * 0x40) | // Write Protect
					(0 * 0x80); // Fault
				printf("FDC: Comm to Rslt phase\n");
				d->phase = TC8566AF_PHASE_RESULT;
				break;
			case TC8566AF_CMD_SEEK:
				// TODO: �V�[�N���s
				d->status3 = (d->command_params[0] & 0x03) | //
					(1 * 0x08) | // Two side
					(1 * 0x10) | // Track 0
					(1 * 0x20) | // Ready
					(1 * 0x40) | // Write Protect
					(0 * 0x80); // Fault
				// TODO: d->command_params[1] �Ɏw�肳�ꂽ�g���b�N�Ɉړ�
				d->status0 |= 0x20; // Seek End
				d->status0 |= 0x08; // Not ready (�f�[�^���ǂ߂�悤�ɂȂ�܂ł́ANot Ready�Ō떂����
				printf("FDC: Comm to Rslt phase\n");
				d->phase = TC8566AF_PHASE_IDLE;
				break;
			default:
				// Execute
				printf("FDC: Comm to Trns phase\n");
				ms_fdc_execute_TC8566AF(d);
				d->phase = TC8566AF_PHASE_DATA_TRANSFER;
			}
		}
		break;
	case TC8566AF_PHASE_DATA_TRANSFER:
		printf("FDC(Tran): Wr#5: %02x\n", value);
		// Perform data transfer operation
		break;
	case TC8566AF_PHASE_RESULT:
		printf("FDC(Rslt): Wr#5: %02x\n", value);
		switch(d->command) {
		default:
			break;
		}
		break;
	}
}

void ms_fdc_execute_TC8566AF(THIS* d) {
	int data_input_output = 0;
	switch(d->command) {
	case TC8566AF_CMD_READ_DATA:
		// Perform read data operation
		data_input_output = 1; // FDC -> Host
		break;
	case TC8566AF_CMD_WRITE_DATA:
		// Perform write data operation
		data_input_output = 0; // Host -> FDC
		break;
	case TC8566AF_CMD_WRITE_DELETED_DATA:
		// Perform write deleted data operation
		break;
	case TC8566AF_CMD_READ_DELETED_DATA:
		// Perform read deleted data operation
		break;
	case TC8566AF_CMD_READ_DIAGNOSTIC:
		// Perform read diagnostic operation
		break;
	case TC8566AF_CMD_READ_ID:
		// Perform read ID operation
		break;
	case TC8566AF_CMD_FORMAT:
		// Perform format operation
		break;
	case TC8566AF_CMD_SCAN_EQUAL:
		// Perform scan equal operation
		break;
	case TC8566AF_CMD_SCAN_LOW_OR_EQUAL:
		// Perform scan low or equal operation
		break;
	case TC8566AF_CMD_SCAN_HIGH_OR_EQUAL:
		// Perform scan high or equal operation
		break;
	case TC8566AF_CMD_SEEK:
		// Perform seek operation
		break;
	case TC8566AF_CMD_RECALIBRATE:
		// Perform recalibrate operation
		break;
	case TC8566AF_CMD_SENSE_INTERRUPT_STATUS:
		// Perform sense interrupt status operation
		break;
	case TC8566AF_CMD_SPECIFY:
		// Perform specify operation
		break;
	case TC8566AF_CMD_SENSE_DEVICE_STATUS:
		// Perform sense device status operation
		break;
	}
}