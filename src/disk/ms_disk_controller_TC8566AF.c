/**
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
#include "../ms.h"
#include "ms_disk_controller_TC8566AF.h"
#include "../memmap/ms_memmap.h"

#define THIS ms_disk_controller_TC8566AF_t

static void _TC8556AF_reg2_write(THIS* d, uint8_t data);
static void _TC8556AF_reg3_write(THIS* d, uint8_t data);
static uint8_t _TC8556AF_reg4_read(THIS* d);
static void _TC8556AF_reg4_write(THIS* d, uint8_t data);
static uint8_t _TC8556AF_reg5_read(THIS* d);
static void _TC8556AF_reg5_write(THIS* d, uint8_t data);

typedef void (*exec_func_t)(THIS* d);
typedef uint8_t (*trans_read_func_t)(THIS* d, uint8_t* finished);
typedef void (*trans_write_func_t)(THIS* d, uint8_t data, uint8_t* finished);
typedef uint8_t (*result_func_t)(THIS* d, uint8_t* finished);

THIS* ms_disk_controller_TC8566AF_alloc() {
	return (THIS*)new_malloc(sizeof(ms_disk_controller_TC8566AF_t));
}

static void read_data_exec(THIS* d);
static uint8_t read_data_transfer(THIS* d, uint8_t* finished); 
static void write_data_exec(THIS* d);
static void write_data_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void read_deleted_data_exec(THIS* d);
static uint8_t read_deleted_data_transfer(THIS* d, uint8_t* finished);
static void write_deleted_data_exec(THIS* d);
static void write_deleted_data_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void read_diagnostic_exec(THIS* d);
static uint8_t read_diagnostic_transfer(THIS* d, uint8_t* finished);
static void read_id_exec(THIS* d);
static uint8_t read_id_transfer(THIS* d, uint8_t* finished);
static void format_exec(THIS* d);  // format = write id
static void format_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void scan_equal_exec(THIS* d);
static void scan_equal_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void scan_low_or_equal_exec(THIS* d);
static void scan_low_or_equal_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void scan_high_or_equal_exec(THIS* d);
static void scan_high_or_equal_transfer(THIS* d, uint8_t data, uint8_t* finished);
static void seek_exec(THIS* d);
static void recalibrate_exec(THIS* d);
static void sense_interrupt_status_exec(THIS* d);
static void specify_exec(THIS* d);
static void sense_device_status_exec(THIS* d);
static void invalid_exec(THIS* d);

static uint8_t chrn_result(THIS* d, uint8_t* finished);
static uint8_t sense_interrupt_status_result(THIS* d, uint8_t* finished);
static uint8_t sense_device_status_result(THIS* d, uint8_t* finished);
static uint8_t st0_result(THIS* d, uint8_t* finished);

void end_command(THIS* d);
void execute(THIS* d);
void to_result_phase(THIS* d);

static TC8566AF_command_t Command(uint8_t* name, uint8_t command_byte_count, exec_func_t execution_phase, trans_read_func_t read_func, trans_write_func_t write_func, result_func_t result_phase) {
	TC8566AF_command_t ret;
	ret.name = name;
	ret.command_byte_count = command_byte_count;
	ret.execution_phase = execution_phase;
	ret.trans_read = read_func;
	ret.trans_write = write_func;
	ret.result_phase = result_phase;
	return ret;
}

void ms_disk_controller_TC8566A_init(THIS* instance, ms_disk_container_t* container) {
	if (instance == NULL) {
		return;
	}
	instance->write_reg2 = _TC8556AF_reg2_write;
	instance->write_reg3 = _TC8556AF_reg3_write;
	instance->read_reg4 = _TC8556AF_reg4_read;
	instance->write_reg4 = _TC8556AF_reg4_write;
	instance->read_reg5 = _TC8556AF_reg5_read;
	instance->write_reg5 = _TC8556AF_reg5_write;

	int i = 0;
	ms_disk_drive_floppy_init(&instance->drive[i], container);
	for(i=1; i<4; i++) {
		ms_disk_drive_floppy_init(&instance->drive[i], NULL);
	}

	instance->phase = TC8566AF_PHASE_IDLE;
	for(i=0; i< 32; i++) {
		instance->commands[i] = 						  Command("INVALID", 			1, invalid_exec,						NULL, NULL,				st0_result);
	}
	instance->commands[TC8566AF_CMD_READ_DATA] 			= Command("READ DATA", 			9, read_data_exec,			read_data_transfer, NULL,			chrn_result);
	instance->commands[TC8566AF_CMD_WRITE_DATA]			= Command("WRITE DATA",			9, write_data_exec, 		NULL, write_data_transfer,			chrn_result);
	instance->commands[TC8566AF_CMD_READ_DELETED_DATA]	= Command("READ DELETED DATA",	9, read_deleted_data_exec,	read_deleted_data_transfer, NULL,	chrn_result);
	instance->commands[TC8566AF_CMD_WRITE_DELETED_DATA]	= Command("WRITE DELETED DATA", 9, write_deleted_data_exec, NULL, write_deleted_data_transfer,	chrn_result);
	instance->commands[TC8566AF_CMD_READ_DIAGNOSTIC]	= Command("READ DIAGNOSTIC",	9, read_diagnostic_exec,	read_diagnostic_transfer, NULL,		chrn_result);
	instance->commands[TC8566AF_CMD_READ_ID]			= Command("READ ID",			2, read_id_exec,			read_id_transfer, NULL,				chrn_result);
	instance->commands[TC8566AF_CMD_FORMAT]				= Command("FORMAT",				6, format_exec,				NULL, format_transfer,				chrn_result);
	instance->commands[TC8566AF_CMD_SCAN_EQUAL]			= Command("SCAN EQUAL",			9, scan_equal_exec,			NULL, scan_equal_transfer,			chrn_result);
	instance->commands[TC8566AF_CMD_SCAN_LOW_OR_EQUAL]	= Command("SCAN LOW OR EQUAL",	9, scan_low_or_equal_exec,	NULL, scan_low_or_equal_transfer,	chrn_result);
	instance->commands[TC8566AF_CMD_SCAN_HIGH_OR_EQUAL]	= Command("SCAN HIGH OR EQUAL",	9, scan_high_or_equal_exec,	NULL, scan_high_or_equal_transfer,	chrn_result);
	instance->commands[TC8566AF_CMD_SEEK]				= Command("SEEK",				3, seek_exec, 							NULL, NULL,				NULL);
	instance->commands[TC8566AF_CMD_RECALIBRATE]		= Command("RECALIBRATE",		2, recalibrate_exec, 					NULL, NULL,				NULL);
	instance->commands[TC8566AF_CMD_SENSE_INTERRUPT_STATUS]	= Command("SENSE INTERRUPT STATUS", 1, sense_interrupt_status_exec, NULL, NULL,				sense_interrupt_status_result);
	instance->commands[TC8566AF_CMD_SPECIFY]			= Command("SPECIFY",			3, specify_exec, 						NULL, NULL,				NULL);
	instance->commands[TC8566AF_CMD_SENSE_DEVICE_STATUS]	= Command("SENSE DEVICE STATUS", 2, sense_device_status_exec, 		NULL, NULL,				sense_device_status_result);

	instance->status0 = 0;
	instance->status1 = 0;
	instance->status2 = 0;

	instance->request_for_master = 1;
	instance->non_dma_mode = 1; // MSX��DMA���g��Ȃ�
}

void ms_disk_controller_TC8566A_deinit(THIS* instance) {

}

/**
 * @brief 
 * 
 * 0x3FF8 - ���W�X�^2 (�R���g���[�����W�X�^0) - �h���C�u�I�� (Write Only)
 * Bit-7	Bit-6	Bit-5	Bit-4	Bit-3	Bit-2	Bita-1	Bit-0
 * LED4		LED3	LED2	LED1	ENID	RST		DriveId	DriveId
 * 
 * * LED4-1 - 1�ɂ���ƃ��[�^�[����]��LED���_��
 * * ENID - 1�ɂ���ƁAINT/DRQ���A�T�[�g����� (�g���Ă��Ȃ�)
 * * RST - 0 �ɂ���ƁAFDC�����Z�b�g����� (�g���Ă��Ȃ�)
 * * DriveId - �h���C�u�ԍ� (0-3)
 * 
 * @param d 
 * @param data 
 */
static void _TC8556AF_reg2_write(THIS* d, uint8_t data) {
	d->drive[3].set_motor(&d->drive[3], (data >> 7) & 0x01);
	d->drive[2].set_motor(&d->drive[2], (data >> 6) & 0x01);
	d->drive[1].set_motor(&d->drive[1], (data >> 5) & 0x01);
	d->drive[0].set_motor(&d->drive[0], (data >> 4) & 0x01);

	d->driveId = data & 0x03;
	MS_LOG(1, "FDC Wr#2: %02x\n", data);
}

/**
 * @brief TC8566AF�̃��W�X�^3�ɏ�������
 * 
 * 0x3FF9 - ���W�X�^3 (�R���g���[�����W�X�^1)
 * �����N����Ȃ�
 * 
 * @param d 
 * @param data 
 */
static void _TC8556AF_reg3_write(THIS* d, uint8_t data) {
}

/**
 * @brief TC8566AF�̃��W�X�^4 (�X�e�[�^�X���W�X�^) ��ǂ�
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
static uint8_t _TC8556AF_reg4_read(THIS* d) {
	// bit5�� NDM�́ADMA���g��Ȃ��ꍇ�ɁA�f�[�^�]���t�F�[�Y�̃n���h�V�F�C�N�̂��߂Ɏg����t���O�ł�
	// ��̓I�ɂ́A�f�[�^�]���t�F�[�Y�̍ŏ���1�ɃZ�b�g����A�f�[�^�]�����I����0�Ƀ��Z�b�g����܂�
	uint8_t ndm_req = ( (d->non_dma_mode == 1) && (d->phase == TC8566AF_PHASE_DATA_TRANSFER)) ? 1 : 0;
	uint8_t ret = (d->request_for_master << 7) | //
				(d->data_input_output << 6) | //
				(ndm_req << 5) | //
				(d->fdc_busy << 4) | //
				(d->fdd_busy & 0x3);
	MS_LOG(1, "FDC Rd#4: %02x\n", ret);
	return ret;
}

/**
 * @brief TC8566AF�̃��W�X�^4(�X�e�[�^�X���W�X�^)�ɏ�������
 * 
 * �����N����Ȃ�
 * 
 * @param d 
 * @param data 
 */
static void _TC8556AF_reg4_write(THIS* d, uint8_t data) {
}

/**
 * @brief TC8566AF�̃��W�X�^5(�f�[�^���W�X�^)��ǂ�
 * 
 * �f�[�^���W�X�^�ł�
 * 
 * @param d 
 * @return uint8_t 
 */
static uint8_t _TC8556AF_reg5_read(THIS* d) {
	uint8_t ret = 0;
	uint8_t phase = d->phase;
	TC8566AF_command_t* command = &d->commands[d->command];
	switch(phase) {
	case TC8566AF_PHASE_IDLE:
		MS_LOG(1, "FDC(Idle): Rd#5:\n");
		break;
	case TC8566AF_PHASE_COMMAND:
		MS_LOG(1, "FDC(Comm): CMD:\n %s Rd#5:\n", command->name);
		break;
	case TC8566AF_PHASE_DATA_TRANSFER:
		MS_LOG(1, "FDC(Trns): CMD: rest=%d\n %s Rd#5:\n", d->transfer_datas_rest, command->name);
		trans_read_func_t read = command->trans_read;
		if (read == NULL) {
			MS_LOG(1, "***Trans read not implemented\n");
			end_command(d);
		} else {
			uint8_t finished = 0;
			ret = read(d, &finished);
			if( finished ) {
				if( command->result_phase != NULL) {
					to_result_phase(d);
				} else {
					end_command(d);
				}
			}
		}
		break;
	case TC8566AF_PHASE_RESULT:
		MS_LOG(1, "FDC(Resl): CMD:\n %s Rd#5:\n", command->name);
		result_func_t result = command->result_phase;
		if (result == NULL) {
			MS_LOG(1, "***Result not implemented\n");
			end_command(d);
		} else {
			uint8_t finished = 0;
			ret = result(d, &finished);
			d->result_byte_index++;
			if (finished) {
				end_command(d);
			}
		}
		break;	
	}
	MS_LOG(1, "  -> ret: %02x\n", ret);
	return ret;
}

/**
 * @brief TC8566AF�̃��W�X�^5(�f�[�^���W�X�^)�ɏ�������
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
 * ## 2. Command Phase
 * �R�}���h�t�F�[�Y�ł́A�R�}���h�ɕK�v�ȃp�����[�^���������݂܂��B�R�}���h�̎�ނɂ���ĉ��񂩌J��Ԃ�write�j
 * 
 * ## 3. Data Transfer Phase
 * �f�[�^�ǂݍ��� or �������݂��s���t�F�[�Y�ł�
 *
 * ## 4. Result Phase
 * ���ʂ��z�X�g���ɕԋp����t�F�[�Y�ł�
 * 
 * 
 * @param d 
 * @param data 
 */
void _TC8556AF_reg5_write(THIS* d, uint8_t value) {
	TC8566AF_command_t* command;

	switch(d->phase) {
	case TC8566AF_PHASE_IDLE:
		uint8_t cmd = 0;
		if ((value & 0x1f) == 0x06) {
			cmd = TC8566AF_CMD_READ_DATA;
		} else if ((value & 0x3f) == 0x05) {
			cmd = TC8566AF_CMD_WRITE_DATA;
		} else if ((value & 0x3f) == 0x09) {
			cmd = TC8566AF_CMD_WRITE_DELETED_DATA;
		} else if ((value & 0x1f) == 0x0c) {
			cmd = TC8566AF_CMD_READ_DELETED_DATA;
		} else if ((value & 0xbf) == 0x02) {
			cmd = TC8566AF_CMD_READ_DIAGNOSTIC;
		} else if ((value & 0xbf) == 0x0a) {
			cmd = TC8566AF_CMD_READ_ID;
		} else if ((value & 0xbf) == 0x0d) {
			cmd = TC8566AF_CMD_FORMAT;
		} else if ((value & 0x1f) == 0x11) {
			cmd = TC8566AF_CMD_SCAN_EQUAL;
		} else if ((value & 0x1f) == 0x19) {
			cmd = TC8566AF_CMD_SCAN_LOW_OR_EQUAL;
		} else if ((value & 0x1f) == 0x1d) {
			cmd = TC8566AF_CMD_SCAN_HIGH_OR_EQUAL;
		} else if ((value & 0xff) == 0x0f) {
			cmd = TC8566AF_CMD_SEEK;
		} else if ((value & 0xff) == 0x07) {
			cmd = TC8566AF_CMD_RECALIBRATE;
		} else if ((value & 0xff) == 0x08) {
			cmd = TC8566AF_CMD_SENSE_INTERRUPT_STATUS;
		} else if ((value & 0xff) == 0x03) {
			cmd = TC8566AF_CMD_SPECIFY;
		} else if ((value & 0xff) == 0x04) {
			cmd = TC8566AF_CMD_SENSE_DEVICE_STATUS;
		}
		d->fdc_busy = 1;
		d->command = cmd;
		d->command_byte_value[0] = cmd;
		d->command_byte_index = 1;

		command = &d->commands[cmd]; // cmd = 0 �Ȃ� INVALID ������
		MS_LOG(1, "FDC(Idle): Accept\n %s Wr#5: %02x\n", command->name, value);
		if( command->command_byte_count == 1) {
			d->request_for_master = 0;
			execute(d);
		} else {
			d->phase = TC8566AF_PHASE_COMMAND;
		}
		break;
	case TC8566AF_PHASE_COMMAND:
		command = &d->commands[d->command];
		MS_LOG(1, "FDC(Comm): Wr#5:\n %s[%d] = %02x\n", command->name, d->command_byte_index, value);
		d->command_byte_value[d->command_byte_index++] = value;
		if (d->command_byte_index == command->command_byte_count) {
			execute(d);
		}
		break;
	case TC8566AF_PHASE_DATA_TRANSFER:
		command = &d->commands[d->command];
		MS_LOG(1, "FDC(Trns): %s Wr#5: %02x\n", command->name, value);
		if( command->trans_write == NULL) {
			MS_LOG(1, "invalid write. igonre\n");
		} else {
			uint8_t finished = 0;
			command->trans_write(d, value, &finished);
			if( finished) {
				if( command->result_phase != NULL) {
					to_result_phase(d);
				} else {
					end_command(d);
				}
			}
		}
		break;
	case TC8566AF_PHASE_RESULT:
		command = &d->commands[d->command];
		MS_LOG(1, "FDC(Rslt): %s Wr#5: %02x\n", command->name, value);
		MS_LOG(1, "invalid result. ignore\n");  // result phase �� read only
		break;		
	}
}

void end_command(THIS* d) {
	d->fdc_busy = 0;			// CB
	d->data_input_output = 0;	// DIO
	d->request_for_master = 1;	// RQM
	d->phase = TC8566AF_PHASE_IDLE;
}

void execute(THIS* d) {
	TC8566AF_command_t* command = &d->commands[d->command];
	exec_func_t exec = command->execution_phase;
	exec(d);
	if( command->trans_read != NULL) {
		d->phase = TC8566AF_PHASE_DATA_TRANSFER;
		d->data_input_output = 1;  // FDC -> Host
	} else if (command->trans_write != NULL) {
		d->phase = TC8566AF_PHASE_DATA_TRANSFER;
		d->data_input_output = 0;  // Host -> FDC
	} else if (command->result_phase != NULL) {
		to_result_phase(d);
	} else {
		end_command(d);
	}
}


void decode_cmd_chrn(THIS* d) {
	d->value_HS = (d->command_byte_value[1] & 0x04) >> 2;
	d->value_DS = (d->command_byte_value[1] & 0x03);
	d->value_C = d->command_byte_value[2];
	d->value_H = d->command_byte_value[3];
	d->value_R = d->command_byte_value[4];
	d->value_N = d->command_byte_value[5];
	d->value_EOT = d->command_byte_value[6];
	d->value_GPL = d->command_byte_value[7];
	d->value_DTL_STP = d->command_byte_value[8];

	// status 0���X�V
	d->status0 &= 0b00111100; // Interrupt code (b7-6)�� Drive select (b1-0) ���N���A
	d->status0 |= (d->value_DS);
	ms_disk_drive_floppy_t* drive = &d->drive[d->driveId];
	if (drive->container == NULL) {
		// �h���C�u�����݂��Ȃ�
		d->status0 |= 0b10000000; // b7-6 = 10 (Invalid command)
	}

	// status 3���X�V
	d->status3 = //
		(0 * 0x80); // Fault
		(drive->is_write_protected * 0x40) | // Write Protect
		(drive->is_disk_inserted * 0x20) | // Ready
		(drive->is_track00 * 0x10) | // Track 0
		(drive->is_double_sided * 0x08) | // Two side
		(d->value_HS * 0x04) | // Head select
		(d->value_DS); // Drive select

	// �h���C�u�̃w�b�h(�T�C�h)��I��
	drive->set_side(drive, d->value_HS);
}

/* *********************************************************

	READ DATA

	READ DATA �R�}���h�́A9�o�C�g����Ȃ�A�ȉ��̏�񂪗^�����܂��B

	* 0�o�C�g��: b7:MT, b6:MFM, b5:SK�Ab4-0:�R�}���h(0x06)
		* MT: Multi-Track�A�����g���b�N��ǂނ��ǂ���
		* MFM: MFM�AMFM�Ńf�[�^��ǂނ��ǂ���
		* SK: Skip�ADDAM(Deleted Data Address Mark)�̂���Z�N�^(�폜���ꂽ�Z�N�^)���X�L�b�v���邩�ǂ���
	* 1�o�C�g��: HS, DS1, DS0
		* HS: �w�b�h�Z���N�g�AFD�̗��\�̂ǂ���̃T�C�h�����w�肵�܂�
		* DS: 2�r�b�g�Ńh���C�u�ԍ����w�肵�܂�
	* 2�o�C�g��: Cylinder
	* 3�o�C�g��: Head
	* 4�o�C�g��: Record (�w�b�h�Ȃ��̃Z�N�^�ԍ�)
	* 5�o�C�g��: N (�Z�N�^�T�C�Y�̎w��A���L�\�Q��)
		* N=0: 128�o�C�g
		* N=1: 256�o�C�g
		* N=2: 512�o�C�g
		* N=3: 1024�o�C�g
	* 6�o�C�g��: EOT (End of Track)�A�g���b�N���̂ǂ̃Z�N�^�܂œǂނ�
	* 7�o�C�g��: GPL (Gap Length)
	* 8�o�C�g��: DTL (Data Length)

  MT, MFM SK�͖������Ă��܂� (MT=1)

 READ DATA �R�}���h�̎��s�́A�ȉ��̎菇�ōs���܂��B
 * Step1: �w�b�h�̃��[�h
	* �w�b�h�����[�h����Ă��Ȃ��ꍇ�̓��[�h���܂�
 * Step2: ID Address Mark�� ID field �̓ǂݍ���
 	* ID Address Mark�Ƃ����̂́AID�t�B�[���h���n�܂邱�Ƃ���������ȃf�[�^(0xa1a1a1fe) �ł�
	* ID field�Ƃ����̂� CHRN �̏����܂ރf�[�^�̂��Ƃł�
 * Step3: �f�[�^�̓ǂݍ���(�]��)
 	* �R�}���h�Ɏw�肳�ꂽ CHRN�ƁAID field��CHRN����v����Z�N�^�������܂ő҂��A�������ꂽ��f�[�^��ǂݍ��݁A�]�����܂�
	* �����w�b�h�̂���ʒu�ƈقȂ�CHRN���w�肵�� READ DATA�R�}���h�𔭍s�����ꍇ�́A�f�[�^��������Ȃ��̂ŁANot Found �G���[��Ԃ��܂�(�����炭)
 * Step4: �f�[�^�̓]���I�����������܂ŌJ��Ԃ�
 	* 1�Z�N�^�[��ǂ񂾂�A�Z�N�^�[�ԍ�(R)���C���N�������g���Ď��̃Z�N�^�[��ǂݍ��݂܂�
	* ����� Multi-sector Read Operation �Ƃ����܂�
	* MT=1�̏ꍇ�͗��ʂ܂œǂ݂ɍs���܂����AMT=0�̏ꍇ�͓���g���b�N���݂̂����ǂ݂܂���
	* EOT�Ŏw�肳�ꂽ�Z�N�^�ԍ��ɒB���邩�A�ő�ŏI�Z�N�^�ɒB������]�����I�����܂�
	* �ő�ŏI�Z�N�^�́AMT=0�̏ꍇ�͓���g���b�N���̍ő�Z�N�^�AMT=1�̏ꍇ�͗��ʂ̍ő�Z�N�^�ŁA�ȉ��̂悤�ɂȂ�܂�
		* MT=0�ŃT�C�h0��ǂ񂾏ꍇ�̍ő�ŏI�Z�N�^	= �T�C�h0, �Z�N�^ 9, �ő�]���Z�N�^��=9	
		* MT=0�ŃT�C�h1��ǂ񂾏ꍇ�̍ő�ŏI�Z�N�^	= �T�C�h1, �Z�N�^ 9, �ő�]���Z�N�^��=9
		* MT=1�ł�ǂ񂾏ꍇ�̍ő�ŏI�Z�N�^		= �T�C�h1, �Z�N�^ 9, �ő�]���Z�N�^��=18
* Step5: �]�����I������烊�U���g�t�F�[�Y�Ō��ʂ�Ԃ��܂�

�ǂݍ��ݒ���CRC�G���[�����������ꍇ�Ȃǂ́Aresult status0,1,2�Ɉȉ��̃r�b�g�𗧂Ă܂�
 * status0: 0b01000000 : IC(Interrupt Code) = 01 = �R�}���h�ُ�I��
 * status1: 0b00100000 : DE(Data Error)
 * status2: 0b00100000 : DD(Data Error in Data Field)
 
********************************************************* */

static void read_data_exec(THIS* d) {
	decode_cmd_chrn(d);
	// TODO: read data from disk

	// ���ۂ�FDC�̓���Ƃ��ẮA�w�b�h�����[�h���A��]����f�B�X�N�̒�����
	// CHRN�ɍ��v����Z�N�^�������A���̃g���b�N�̃f�[�^���Ō�܂œǂݍ��݂܂�
	// �v���e�N�g�̍Č�������ꍇ�͂��̃^�C�~���O�Ȃǂ����킹��K�v������܂����A
	// �����ł͒P�Ƀf�[�^��ǂݍ��ނ����Ƃ��܂�

	if ( d->value_R > d->value_EOT) {
		// �w�肳�ꂽ�Z�N�^�ԍ���EOT�𒴂��Ă���ꍇ�A0�o�C�g
		d->transfer_datas_rest = 0;
	} else {
		d->transfer_datas_rest = (d->value_EOT - d->value_R + 1) * (128 << d->value_N); // TODO �ЂƂ܂����ߑł�
	}
}

static uint8_t read_data_transfer(THIS* d, uint8_t* finished) {
	uint8_t ret = 0;
	d->transfer_datas_rest -= 1;
	if( d->transfer_datas_rest <= 0) {
		to_result_phase(d);
	}
	// TODO : read data from disk
	return ret;
}

void to_result_phase(THIS* d) {
	d->data_input_output = 1;  // FDC -> Host
	d->request_for_master = 1; // RQM
	d->result_byte_index = 0;
	d->phase = TC8566AF_PHASE_RESULT;
}

/* *********************************************************

	WRITE DATA

********************************************************* */
static void write_data_exec(THIS* d) {
}

static void write_data_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/* *********************************************************

	READ DELETED DATA

********************************************************* */
static void read_deleted_data_exec(THIS* d) {

}

static uint8_t read_deleted_data_transfer(THIS* d, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	WRITE DELETED DATA

********************************************************* */
static void write_deleted_data_exec(THIS* d) {

}

static void write_deleted_data_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	READ DIAGNOSTIC

********************************************************* */
static void read_diagnostic_exec(THIS* d) {

}

static uint8_t read_diagnostic_transfer(THIS* d, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	READ ID

********************************************************* */
static void read_id_exec(THIS* d) {

}
static uint8_t read_id_transfer(THIS* d, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	FORMAT

********************************************************* */
static void format_exec(THIS* d) {

}

static void format_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	SCAN EQUAL

********************************************************* */
static void scan_equal_exec(THIS* d) {

}

static void scan_equal_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	SCAN LOW OR EQUAL

********************************************************* */
static void scan_low_or_equal_exec(THIS* d) {

}

static void scan_low_or_equal_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	SCAN HIGH OR EQUAL

********************************************************* */
static void scan_high_or_equal_exec(THIS* d) {

}

static void scan_high_or_equal_transfer(THIS* d, uint8_t data, uint8_t* finished) {
	to_result_phase(d);
}

/*********************************************************

	SEEK

********************************************************* */
static void seek_exec(THIS* d) {
	ms_disk_drive_floppy_t* drive = &d->drive[d->driveId];
	if (drive->container == NULL) {
		// �h���C�u�����݂��Ȃ�
		d->status0 |= 0b10000000; // b7-6 = 10 (Invalid command)
	}
	d->status0 |= 0x20; // Seek End
	d->status0 |= 0x08; // Not ready (�f�[�^���ǂ߂�悤�ɂȂ�܂ł́ANot Ready�Ō떂����)
	d->status3 = //
		(0 * 0x80) | // Fault
		(drive->is_write_protected * 0x40) | // Write Protect
		(drive->is_disk_inserted * 0x20) | // Ready
		(drive->is_track00 * 0x10) | // Track 0
		(drive->is_double_sided * 0x08) | // Two side
		(d->value_HS * 0x04) | // Head select
		(d->value_DS); // Drive select
}

/*********************************************************

	RECALIBRATE

********************************************************* */
static void recalibrate_exec(THIS* d) {
	d->status0 |= 0x20; // Seek End
	d->status0 |= 0x08; // Not ready (�f�[�^���ǂ߂�悤�ɂȂ�܂ł́ANot Ready�Ō떂����)
}

/*********************************************************

	SENSE INTERRUPT STATUS

********************************************************* */
static void sense_interrupt_status_exec(THIS* d) {

}

/*********************************************************

	SPECIFY

********************************************************* */
static void specify_exec(THIS* d) {

}

/*********************************************************

	SENSE DEVICE STATUS

********************************************************* */
static void sense_device_status_exec(THIS* d) {
	ms_disk_drive_floppy_t* drive = &d->drive[d->driveId];
	if (drive->container == NULL) {
		// �h���C�u�����݂��Ȃ�
		d->status0 |= 0b10000000; // b7-6 = 10 (Invalid command)
	}

	d->status3 = //
		(0 * 0x80) | // Fault
		(drive->is_write_protected * 0x40) | // Write Protect
		(drive->is_disk_inserted * 0x20) | // Ready
		(drive->is_track00 * 0x10) | // Track 0
		(drive->is_double_sided * 0x08) | // Two side
		(d->value_HS * 0x04) | // Head select
		(d->value_DS); // Drive select
}

/*********************************************************

	INVALID

********************************************************* */
static void invalid_exec(THIS* d) {
	MS_LOG(1, "invalid command\n");
}


/* *********************************************************

	RESULT

********************************************************* */

/**
 * @brief CHRN�̌��ʂ�Ԃ�result phase�ł�
 * 
 * @param d 
 * @param finished 
 * @return uint8_t 
 */
static uint8_t chrn_result(THIS* d, uint8_t* finished) {
	uint8_t ret = 0;
	switch(d->result_byte_index) {
	case 0:
		ret = d->status0;
		break;
	case 1:
		ret = d->status1;
		break;
	case 2:
		ret = d->status2;
		break;
	case 3:
		ret = d->status3;
		break;
	case 4:
		ret = d->value_C;
		break;
	case 5:
		ret = d->value_H;
		break;
	case 6:
		ret = d->value_R;
		break;
	case 7:
		ret = d->value_N;
		*finished = 1;
		break;
	default:
		MS_LOG(1, "invalid result index: %d\n", d->result_byte_index);
		*finished = 1;
		break;
	}
	return ret;
}

static uint8_t sense_interrupt_status_result(THIS* d, uint8_t* finished) {
	uint8_t ret = 0;
	switch(d->result_byte_index) {
	case 0:
		ret = d->status0;
		d->status0 = 0;
		break;
	case 1:
		ret = 0; // TODO
		*finished = 1;
		break;
	default:
		MS_LOG(1, "invalid result index: %d\n", d->result_byte_index);
		*finished = 1;
		break;
	}
	return ret;
}

static uint8_t sense_device_status_result(THIS* d, uint8_t* finished) {
	uint8_t ret = 0;
	switch(d->result_byte_index) {
	case 0:
		ret = d->status3;
		*finished = 1;
		break;
	default:
		MS_LOG(1, "invalid result index: %d\n", d->result_byte_index);
		*finished = 1;
		break;
	}
	return ret;		
}

static uint8_t st0_result(THIS* d, uint8_t* finished) {
	uint8_t ret = 0;
	switch(d->result_byte_index) {
	case 0:
		ret = d->status0;
		*finished = 1;
		break;
	default:
		MS_LOG(1, "invalid result index: %d\n", d->result_byte_index);
		*finished = 1;
		break;
	}
	return ret;
}
