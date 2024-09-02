/*

		�l�r�w.�r���������������� [[ MS ]]
				[[�X�^�[�g�A�b�v�v���O���� ]]

	ver. 0.01	prpgramed by Kuni.
										1995.9.15

*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <iocslib.h>
#include <doslib.h>
#include <getopt.h>
#include "ms.h"
#include "ms_R800.h"
#include "ms_iomap.h"
#include "memmap/ms_memmap.h"
#include "vdp/ms_vdp.h"
#include "disk/ms_disk_container.h"

void ms_exit( void);

// �������֘A
ms_memmap_t* memmap = NULL;

// I/O�֘A
ms_iomap_t* iomap = NULL;

// VDP�֘A
ms_vdp_t* vdp = NULL;  // ms_vdp_shared �Ɠ����ɂȂ�͂�

// PSG�֘A
int psg_initialized = 0;
int ms_psg_init( void);
void ms_psg_deinit(void);

// Disk�֘A
ms_disk_container_t* disk_container = NULL;

/*
  Slot��ROM���Z�b�g���܂��B
	int ms_memmap_set_rom( (void *)address, (char *)filename, (int)kind, (int)slot, (int)page);

  �����F
	void *ROM		: ROM�̃A�h���X
	char *ROMName	: ROM�t�@�C����
	int size		: ROM�̃T�C�Y
	int slot		: �X���b�g�ԍ�
	int page		: �y�[�W�ԍ�

  �X���b�g�ԍ��̏ڍׁF
	slot %0000ssxx
		ss: ��{�X���b�g�ԍ�
		xx: �g���X���b�g�ԍ�

	�߂�l�F(�����Ǝ�������Ă��Ȃ���������܂���)
		0	: ����I��
		-1	: �G���[
*/
void ms_memmap_set_rom( void* address, const char* filename, int kind, int slot, int page);
void ms_memmap_register_rom( void* address, int kind, int slot, int page);

int emuLoop(unsigned int pc, unsigned int counter);

void set_system_roms(void);

void _toggleTextPlane(void);
void _setTextPlane(int textPlaneMode);
void _moveTextPlane(int cursorKeyHit);

unsigned short host_rate = 1;

volatile extern unsigned short ms_vdp_interrupt_tick;
volatile extern unsigned short ms_vdp_vsync_rate;
volatile extern unsigned int int_block_count;
volatile extern unsigned short host_delay;
volatile extern unsigned int int_skip_counter;
volatile extern unsigned int int_exec_counter;
typedef struct interrupt_history_st {
    unsigned short int_tick;
    unsigned short process_type;
    unsigned long emu_counter;
} interrupt_history_t;

volatile extern interrupt_history_t* interrupt_history_ptr;
volatile extern unsigned short interrupt_history_wr;
volatile extern unsigned short interrupt_history_rd;

void printHelpAndExit(char* progname) {
	fprintf(stderr, "Usage: %s  [-w MAX_WAIT] [-rm MAINROM_PATH] [-rs SUBROM_PATH] [-rd DISKBIOS_PATH] [-r1 ROM_PATH for slot 1][,KIND] [-r2 ROM_PATH for slot 2][,KIND] [-rNM ROM_PATH for slot N page M] [IMAGE1.DSK] [IMAGE2.DSK]..\n", progname);
	fprintf(stderr, " KIND is ROM type:\n");
	fprintf(stderr, "    NOR: Normal ROM, G8K: GENERIC 8K, A8K: ASCII 8K, A16: ASCII 16K, KON: Konami, SCC: Konami SCC\n");
	fprintf(stderr, " --vsrate vsync rate (1-60)\n");
	fprintf(stderr, "    1: every frame, 2: every 2 frames, ...\n");
	fprintf(stderr, "    default is 1.\n");
	fprintf(stderr, " --intblock block cycle (1-9999)\n");
	fprintf(stderr, "    default is 2048 cycles.\n");
	fprintf(stderr, " --hostrate host key operation rate (1-60)\n");
	fprintf(stderr, "    1: every frame, 2: every 2 frames, ...\n");
	fprintf(stderr, "    default is 3.\n");
	fprintf(stderr, " --hostdelay host key interruption delay cycle (1-9999)\n");
	fprintf(stderr, "    default is 20 cycles.\n");
	fprintf(stderr, " --disablekey\n");
	fprintf(stderr, "    disable key input for performance test.\n");
//	fprintf(stderr, " --debuglevel N\n");
//	fprintf(stderr, "    0: None, 1: Info, 2: Debug, 3: Fine.\n");
	exit(EXIT_FAILURE);
}

static unsigned char *MainROM1;
static unsigned char *MainROM2;
static unsigned char *SUBROM;
static unsigned char *SLOT1_1;
static unsigned char *SLOT1_2;
static unsigned char *ROM;

uint32_t max_wait = 0xffffffff;
int disablekey = 0;

const char *mainrom_cbios = "cbios_main_msx2_jp.rom";
const char *cbioslogo = "cbios_logo_msx2.rom";
const char *subrom_cbios = "cbios_sub.rom";

// char *mainrom_user = "MAINROM.ROM";
// char *subrom_user = "SUBROM.ROM";
// char *diskbios_user = "DISKBIOS.ROM";
char *mainrom_user = "fs-a1f_basic-bios2.rom";
char *subrom_user = "fs-a1f_msx2sub.rom";
char *diskbios_user = "fs-a1f_disk.rom";

int diskcount = 0;
char *diskimages[16];

char* separate_rom_kind(char* path, int* kind) {
	char* p = strchr(path, ',');
	if (p != NULL) {
		*p = '\0';
		p++;
		if (strcasecmp(p, "NOR") == 0) {
			*kind = ROM_TYPE_NORMAL_ROM;
		} else if (strcasecmp(p, "G8K") == 0) {
			*kind = ROM_TYPE_MEGAROM_GENERIC_8K;
		} else if (strcasecmp(p, "A8K") == 0) {
			*kind = ROM_TYPE_MEGAROM_ASCII_8K;
		} else if (strcasecmp(p, "A16") == 0) {
			*kind = ROM_TYPE_MEGAROM_ASCII_16K;
		} else if (strcasecmp(p, "KON") == 0) {
			*kind = ROM_TYPE_MEGAROM_KONAMI;
		} else if (strcasecmp(p, "SCC") == 0) {
			*kind = ROM_TYPE_MEGAROM_KONAMI_SCC;
		} else {
			*kind = -1;
		}
	}
	return path;
}

/*
	���C���֐�

	�N���I�v�V�����F
		-m filename		: MAIN ROM���w�肵�����̂ɕύX���܂�(32K�o�C�g)
		-rxx filename	: �X���b�g��filename���Z�b�g���܂�
			xx: 10�̈� = �X���b�g�ԍ�, 1�̈� = �y�[�W�ԍ�
			��F-r11 GAME1.ROM	: �X���b�g1-�y�[�W1��GAME1.ROM���Z�b�g				
*/
int main(int argc, char *argv[]) {
	int i, j;
	char *cartridge_path_slot1 = NULL; // �J�[�g���b�W�̃p�X
	int cartridge_kind_slot1 = -1; // �J�[�g���b�W�̎��
	char *cartridge_path_slot2 = NULL; // �J�[�g���b�W�̃p�X
	int cartridge_kind_slot2 = -1; // �J�[�g���b�W�̎��
	char *slot_path[4][4]; // �X�̃X���b�g�ɃZ�b�g����ROM�̃p�X
	int opt;
    const char* optstring = "hm:s:w:r:" ; // optstring���`���܂�
    const struct option longopts[] = {
      //{        *name,           has_arg,       *flag, val },
        {     "vsrate", required_argument,           0, 'A' },
        {   "intblock", required_argument,           0, 'B' },
        {   "hostrate", required_argument,           0, 'C' },
        {  "hostdelay", required_argument,           0, 'D' },
		{ "disablekey",       no_argument, &disablekey,  1  },
        {            0,                 0,           0,  0  }, // termination
    };
	const struct option* longopt;
    int longindex = 0;

	printf("[[ MSX Simulator MS.X]]\n");

	unsigned int mpu_type = _iocs_mpu_stat();
	if( (mpu_type & 0xf) < 3) {
		printf("MS.X �̓���ɂ� 68030�ȏオ�K�v�ł�\n");
		ms_exit();
	}

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			slot_path[i][j] = NULL;
		}
	}

	for(i=0;i<32;i++) {
		interrupt_history_ptr[i].int_tick = 0;
		interrupt_history_ptr[i].process_type = 0;
		interrupt_history_ptr[i].emu_counter = 0;
	}

	// �R�}���h���C���I�v�V�����̉��
	while ((opt = getopt_long(argc, argv, optstring, longopts, &longindex)) != -1)
	{
		switch (opt)
		{
		case 0: // �t���O���Z�b�g���ꂽ�ꍇ
			break;
		case 'h': // -h �I�v�V����
			printHelpAndExit(argv[0]);
			break;
		case 'm': // -m �I�v�V����
			if (optarg != NULL)
			{
				mainrom_user = optarg;
			}
			break;
		case 'w': // -w �I�v�V����
			if (optarg != NULL)
			{
				max_wait = atoi(optarg);
			}
			break;
		case 's': // -s �I�v�V����
			if (optarg != NULL)
			{
				subrom_user = optarg;
			}
			break;
		case 'r': // -rNN �I�v�V����
			if (strlen(optarg) == 1 && !isdigit(optarg[0]) ) {
				// -r �ɐ����ȊO�������ꍇ
				switch(optarg[0]) {
				case 'm':
					// ���C��ROM�̎w��
					if (argv[optind] != NULL)
					{
						mainrom_user = argv[optind++];
					}
					else
					{
						printf("ROM�t�@�C�������w�肳��Ă��܂���\n");
						ms_exit();
					}
					break;
				case 's':
					// �T�uROM�̎w��
					if (argv[optind] != NULL)
					{
						subrom_user = argv[optind++];
					}
					else
					{
						printf("ROM�t�@�C�������w�肳��Ă��܂���\n");
						ms_exit();
					}
					break;
				case 'd':
					// �f�B�X�NBIOS�̎w��
					if (argv[optind] != NULL)
					{
						diskbios_user = argv[optind++];
					}
					else
					{
						printf("ROM�t�@�C�������w�肳��Ă��܂���\n");
						ms_exit();
					}
					break;
				default:
					printf("�s���ȃI�v�V�����ł�\n");
					printHelpAndExit(argv[0]);
					break;
				}
			} else if (strlen(optarg) == 1 && isdigit(optarg[0]) ) {
				// -r �ɐ�����1�������ꍇ
				int slot = atoi(optarg);
				if ( slot >= 1 && slot <= 2) {
					// ���̈����iROM�t�@�C�����j���擾
					if (argv[optind] != NULL)
					{
						if (slot == 1) {
							cartridge_path_slot1 = argv[optind++];
							cartridge_path_slot1 = separate_rom_kind(cartridge_path_slot1, &cartridge_kind_slot1);
						} else {
							cartridge_path_slot2 = argv[optind++];
							cartridge_path_slot2 = separate_rom_kind(cartridge_path_slot2, &cartridge_kind_slot2);
						}
					}
					else
					{
						printf("ROM�t�@�C�������w�肳��Ă��܂���\n");
						ms_exit();
					}
				} else {
					printf("�X���b�g�ԍ���1��2���w�肵�Ă��������B\n");
					printHelpAndExit(argv[0]);
				}
			} else if (strlen(optarg) == 2 && isdigit(optarg[0]) && isdigit(optarg[1])) {
				// -r �ɐ�����2�������ꍇ
				int num = atoi(optarg);
				int slot = (num / 10);
				int page = num % 10; // 1�̈ʂ�page
				if ( slot >= 0 && slot <= 3 && page >= 0 && page <= 3) {
					// ���̈����iROM�t�@�C�����j���擾
					if (argv[optind] != NULL)
					{
						slot_path[slot][page] = argv[optind++];
					}
					else
					{
						printf("ROM�t�@�C�������w�肳��Ă��܂���\n");
						ms_exit();
					}
				} else {
					printf("�X���b�g�ԍ����s���ł�\n");
					printHelpAndExit(argv[0]);
				}
			} else {
				// -r �P�Ƃ� -r1 �Ɠ����Ƃ݂Ȃ�
				// ���̈����iROM�t�@�C�����j���擾
				if (optarg != NULL)
				{
					cartridge_path_slot1 = optarg;
					cartridge_path_slot1 = separate_rom_kind(cartridge_path_slot1, &cartridge_kind_slot1);
				}
				else
				{
					printf("ROM�t�@�C�������w�肳��Ă��܂���\n");
					ms_exit();
				}
			}
			break;
		case 'A': // --vsrate N �I�v�V����
			// VSYNC���[�g�̐ݒ�
			longopt = &longopts[longindex];
			if (longopt->has_arg == required_argument & optarg != NULL) {
				ms_vdp_vsync_rate = atoi(optarg);
				if (ms_vdp_vsync_rate < 1 || ms_vdp_vsync_rate > 61) {
					printf("VSYNC���[�g���s���ł�\n");
					printHelpAndExit(argv[0]);
				}
			} else {
				printf("VSYNC���[�g���w�肳��Ă��܂���\n");
				printHelpAndExit(argv[0]);
			}
			break;
		case 'B': // --intblock N �I�v�V����
			// ���荞�݃u���b�N�J�E���g�̐ݒ�
			longopt = &longopts[longindex];
			if (longopt->has_arg == required_argument & optarg != NULL) {
				int_block_count = atoi(optarg);
				if (int_block_count < 1 || int_block_count > 9999) {
					printf("���荞�݃u���b�N�J�E���g���s���ł�\n");
					printHelpAndExit(argv[0]);
				}
			} else {
				printf("���荞�݃u���b�N�J�E���g���w�肳��Ă��܂���\n");
				printHelpAndExit(argv[0]);
			}
			break;
		case 'C': // --hostrate N �I�v�V����
			// �z�X�g�������[�g�̐ݒ�
			longopt = &longopts[longindex];
			if (longopt->has_arg == required_argument & optarg != NULL) {
				host_rate = atoi(optarg);
				if (host_rate < 1 || host_rate > 61) {
					printf("�z�X�g�������[�g���s���ł�\n");
					printHelpAndExit(argv[0]);
				}
			} else {
				printf("�z�X�g�������[�g���w�肳��Ă��܂���\n");
				printHelpAndExit(argv[0]);
			}
			break;
		case 'D': // --hostdelay N �I�v�V����
			// �z�X�g�����x���J�E���g�̐ݒ�
			longopt = &longopts[longindex];
			if (longopt->has_arg == required_argument & optarg != NULL) {
				host_delay = atoi(optarg);
				if (host_delay < 1 || host_delay > 9999) {
					printf("�z�X�g�����x���J�E���g���s���ł�\n");
					printHelpAndExit(argv[0]);
				}
			} else {
				printf("�z�X�g�����x���J�E���g���w�肳��Ă��܂���\n");
				printHelpAndExit(argv[0]);
			}
			break;
		default: /* '?' */
			printHelpAndExit(argv[0]);
			break;
		}
	}

    // getopt�̃��[�v���I��������A�ʏ�̈�������������
	for (i = optind; i < argc; i++) {
		if (strcasestr(argv[i], ".dsk") != 0) {
			diskimages[diskcount++] = argv[i];
		}
	}

	if (_iocs_b_super(0) < 0)
	{
		printf("�X�[�p�[�o�C�U�[���[�h�Ɉڍs�ł��܂���ł���\n");
		return -1;
	}

	// CTRL+C�Œ��f���ꂽ�Ƃ��ɁAms_exit���Ăяo���悤�ɂ���
	_dos_intvcs(0xfff1, ms_exit);

	/*
	 �������V�X�e���̏�����
	 */
	memmap = ms_memmap_alloc();
	if (memmap == NULL)
	{
		printf("�������V�X�e���̏������Ɏ��s���܂���\n");
		ms_exit();
	}
	ms_memmap_init(memmap);

	/*
	 VDP�V�X�e���̏�����
	*/
	vdp = ms_vdp_alloc();
	if (vdp == NULL)
	{
		printf("VDP�V�X�e���̏������Ɏ��s���܂���\n");
		ms_exit();
	}
	ms_vdp_init(vdp);

	/*
	 I/O�V�X�e���̏�����
	 */
	iomap = ms_iomap_alloc();
	if (iomap == NULL)
	{
		printf("I/O�V�X�e���̏������Ɏ��s���܂���\n");
		ms_exit();
	}
	ms_iomap_init(iomap, vdp);


	printf("\n\n\n\n\n\n\n\n"); // TEXT��ʂ����8���C�����炢�グ�Ă���̂ŁA���̕����s������
	printf("[[ MSX Simulator MS.X]]\n");
	printf(" ���̉�ʂ� HELP �L�[�ŏ����܂�\n");

	/*
	 PSG�V�X�e���̏�����
	 */
	psg_initialized = ms_psg_init();
	if (psg_initialized == 0)
	{
		printf("�o�r�f�̏������Ɏ��s���܂���\n");
		ms_exit();
	}


	/*
	 SYSTEM ROM�̃Z�b�g
	 */
	set_system_roms();

	/* ��{�X���b�g1-�y�[�W1��ROM�z�u (32KBytes�Q�[���̑O��16K�Ȃǁj	*/
	//	allocateAndSetROM( 16 * 1024,"GAME1.ROM", (int)2, (int)0x04, (int)1 );
	/* ��{�X���b�g1-�y�[�W1��ROM�z�u (32KBytes�Q�[���̌㔼16K�Ȃǁj	*/
	//	allocateAndSetROM( 16 * 1024,"GAME2.ROM", (int)2, (int)0x04, (int)2 );

	for ( i = 0; i < 4; i++) {
		for ( j = 0; j < 4; j++) {
			if ( slot_path[i][j] != NULL) {
				printf("�X���b�g%d-�y�[�W%d��ROM���Z�b�g���܂�: %s\n", i, j, slot_path[i][j]);
				allocateAndSetROM(slot_path[i][j], ROM_TYPE_NORMAL_ROM, i, -1, j);
			}
		}
	}

	// �T�C�Y���������ʂ���ROM���Z�b�g
	if (cartridge_path_slot1 != NULL) {
		allocateAndSetROM_Cartridge(cartridge_path_slot1, 1, cartridge_kind_slot1);
	}
	if (cartridge_path_slot2 != NULL) {
		allocateAndSetROM_Cartridge(cartridge_path_slot2, 2, cartridge_kind_slot2);
	}

	printf("X680x0 MSX�V�~�����[�^�[ with elf2x68k\n");

	printf("VSYNC���[�g=%d, �z�X�g�������[�g=%d\n", ms_vdp_vsync_rate, host_rate);
	printf("VSYNC�v����...\n");
	{
		volatile int date,lastdate;
		volatile int start,end;

		date = _iocs_timeget();
		lastdate = date;
		while(date == lastdate) {	// �b���ς��u�Ԃ�҂�
			lastdate = date;
			date = _iocs_timeget();
		}
		start = ms_vdp_interrupt_tick;		// ���̂Ƃ���tick���擾
		date = _iocs_timeget();
		lastdate = date;
		while(date == lastdate) {	// �b���ς��u�Ԃ�҂�
			lastdate = date;
			date = _iocs_timeget();
		}
		end = ms_vdp_interrupt_tick;		// ���̂Ƃ���tick���擾

		printf("VSYNC�񐔂� %d �ł�\n", end - start);
	}

	// 	�S�y�[�W�� �X���b�g���O�ŏ�����
	write_port_A8(0);

	if (1) {
		ms_cpu_emulate(emuLoop, max_wait);
	} else {
		debugger();
	}

	ms_exit();
}

void ms_exit() {
	printf("�I�����܂��B�����L�[�������Ă��������B\n");
	_iocs_b_keyinp();

	_iocs_crtmod(0x10);

	if( disk_container != NULL ) {
		ms_disk_container_deinit(disk_container);
		new_free(disk_container);
	}
	if ( psg_initialized ) {
		ms_psg_deinit();
	}
	if ( vdp != NULL ) {
		ms_vdp_deinit(vdp);
		new_free(vdp);
	}
	if ( iomap != NULL ) {
		ms_iomap_deinit(iomap);
		new_free(iomap);
	}
	if ( memmap != NULL ) {
		ms_memmap_deinit(memmap);
		new_free(memmap);
	}
	exit(0);
}

/*
 X68000�� _BITSNS �œ�����L�[�}�g���N�X��MSX�̃L�[�}�g���N�X�ɕϊ����邽�߂̃}�b�s���O

 - X68000���̃}�g���N�X
	- X68000���n���h�u�b�N �up287 _BITSNS�v �Q��
 - MSX���̃}�g���N�X
	- MSX�f�[�^�p�b�N p12 �u2.5 �L�[�{�[�h�v �Q��
 */
unsigned short KEY_MAP[][8] = {
	//   BIT0  BIT1  BIT2  BIT3  BIT4  BIT5  BIT6  BIT7
	//0 [    ][ESC ][1   ][2   ][3   ][4   ][5   ][6   ] 
	{   0xf00,0xf10,0xf11,0xf12,0xf13,0xf14,0xf15,0xf16},
	//1 [7   ][8   ][9   ][0   ][-   ][^   ][\   ][BS  ] 
	{   0xf17,0xf18,0xf19,0x001,0x104,0x108,0x110,0x720},
	//2 [TAB ][Q   ][W   ][E   ][R   ][T   ][Y   ][U   ] 
	{   0x708,0x440,0x510,0x304,0x480,0x502,0x540,0x504},
	//3 [I   ][O   ][P   ][@   ][ [  ][RET ][ A  ][ S  ] 
	{   0x340,0x410,0x420,0x120,0x140,0x780,0x240,0x501},
	//4 [ D  ][ F  ][ G  ][ H  ][ J  ][ K  ][ L  ][ ;+ ] 
	{   0x302,0x308,0x310,0x320,0x380,0x401,0x402,0x180},
	//5 [ :* ][ ]  ][ Z  ][ X  ][ C  ][ V  ][ B  ][ N  ] 
	{   0x201,0x203,0x580,0x520,0x301,0x508,0x280,0x408},
	//6 [ M  ][ ,< ][ .> ][ /  ][ _  ][ SP ][HOME][DEL ]  HOME=CLS
	{   0x404,0x204,0x208,0x210,0x220,0x801,0x802,0x808}, 
	//7 [RUP ][RDWN][UNDO][LEFT][UP  ][RIGT][DOWN][CLR ]
	{   0xf00,0xf00,0xf00,0xff1,0xff2,0xff4,0xff3,0xf00},
	//8 [(/) ][(*) ][(-) ][(7) ][(8) ][(9) ][(+) ][(4) ]  TEN KEYs
	{   0x902,0x904,0xa20,0xa04,0xa08,0xa10,0x901,0x980},
	//9 [(5) ][(6) ][(=) ][(1) ][(2) ][(3) ][ENTR][(0) ]  TEN KEYs
	{   0xa01,0xa02,0xf00,0x910,0x920,0x940,0xf00,0x908},
	//a [(,) ][(.) ][KIGO][TORK][HELP][XF1 ][XF2 ][XF3 ]  �o�^=�G�~�����[�^�I��, XF1=GRAPH
	{   0xa40,0xa80,0xffd,0xfff,0xffe,0x604,0xf00,0xf00}, 
	//b [XF4 ][XF5 ][KANA][ROME][CODE][CAPS][INS ][HIRA]
	{   0xf00,0xf00,0x610,0xf00,0xf00,0xf00,0x804,0xf00},
	//c [ZENK][BRAK][COPY][ F1 ][ F2 ][ F3 ][ F4 ][ F5 ]  BRAK=STOP
	{   0xf00,0x710,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//d [ F6 ][ F7 ][ F8 ][ F9 ][ F10][    ][    ][    ]  F6=DebugLevel
	{   0xffc,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//e [SHFT][CTRL][OPT1][OPT2][    ][    ][    ][    ]  OPT1=Disk Change
	{   0xff0,0x602,0xff8,0xff9,0xf00,0xf00,0xf00,0xf00},
	//f [    ][    ][    ][    ][    ][    ][    ][    ]
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00}
};

extern unsigned char* KEYSNS_tbl_ptr;

int divider = 0;

void dump(unsigned int page, unsigned int pc_8k);

void sync_keyboard_leds();

int emuLoop(unsigned int pc, unsigned int counter) {
	static int emuLoopCounter = 0;
	static uint8_t last_bitsns[16];

	int i,j;
	unsigned short map;
	unsigned char X, Y;
	int hitkey = 0;
	int shiftKeyPress = 0;
	int cursorKeyHit = 0; // 1=LEFT, 2=UP, 3=DOWN, 4=RIGHT
	int f6KeyHit = 0;
	int kigoKeyHit = 0;
	int helpKeyHit = 0;
	int disk_change = -1; // 0=Eject, 1=Disk1, 2=Disk2, 3=Disk3, 4=Disk4

	emuLoopCounter++;

	if( vdp != NULL) {
		ms_vdp_vsync_draw(vdp);
	}

	if(emuLoopCounter % host_rate != 0) {
		return 0;
	}

	sync_keyboard_leds();

	for ( i = 0x00; i < 0x0f; i++)
	{
		KEYSNS_tbl_ptr[i] = 0xff;
	}
	for ( i = 0x00; i < 0x0f; i++)
	{
		if (disablekey == 1 && i != 0x0a)
		{
			continue;
		}
		//int v = _iocs_bitsns(i);
		int v = ((uint8_t*)0x800)[i];	// IOCS BITSNS�̃��[�N�G���A���ڎQ��
		int mask = 1;
		for ( j = 0; j < 8; j++) {
			if (v & mask) {
				hitkey = 1;
				map = KEY_MAP[i][j];
				Y = (map & 0xf00) >> 8;
				X = (map & 0xff);
				if (Y == 0xf)
				{
					int opt1_pressed = ((uint8_t*)0x800)[0xe] & 0x04;
					// ����L�[
					switch (X)
					{
					// *********************************************
					// 0x10-0x1f: Disk Change (Opt.1��Press���K�v)
					case 0x10: // ESC�L�[ 0x704 (Eject)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 0;
							}
						} else {
							Y = 0x7;
							X = 0x04;
						}
						break;
					case 0x11: // 1�L�[ 0x002 (Disk1)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 1;
							}
						} else {
							Y = 0x0;
							X = 0x02;
						}
						break;
					case 0x12: // 2�L�[ 0x004 (Disk2)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 2;
							}
						} else {
							Y = 0x0;
							X = 0x04;
						}
						break;
					case 0x13: // 3�L�[ 0x008 (Disk3)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 3;
							}
						} else {
							Y = 0x0;
							X = 0x08;
						}
						break;
					case 0x14: // 4�L�[ 0x010 (Disk4)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 4;
							}
						} else {
							Y = 0x0;
							X = 0x10;
						}
						break;
					case 0x15: // 5�L�[ 0x020 (Disk5)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 5;
							}
						} else {
							Y = 0x0;
							X = 0x20;
						}
						break;
					case 0x16: // 6�L�[ 0x040 (Disk6)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 6;
							}
						} else {
							Y = 0x0;
							X = 0x40;
						}
						break;
					case 0x17: // 7�L�[ 0x080 (Disk7)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 7;
							}
						} else {
							Y = 0x0;
							X = 0x80;
						}
						break;
					case 0x18: // 8�L�[ 0x101 (Disk8)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 8;
							}
						} else {
							Y = 0x1;
							X = 0x01;
						}
						break;
					case 0x19: // 9�L�[ 0x102 (Disk9)
						if(opt1_pressed) {
							if( !(last_bitsns[i] & mask)) { // edge detect
								disk_change = 9;
							}
						} else {
							Y = 0x1;
							X = 0x02;
						}
						break;
					// *********************************************
					case 0xf0: // SHIFT�L�[ 0x601
						Y = 0x6;
						X = 0x01;
						shiftKeyPress = 1;
						break;
					case 0xf1: // LEFT�L�[ 0x810
						Y = 0x8; 
						X = 0x10;
						if(!(last_bitsns[i] & mask)) { // edge detect
							cursorKeyHit = 1;
						}
						break;
					case 0xf2: // UP�L�[ 0x820
						Y = 0x8; 
						X = 0x20;
						if(!(last_bitsns[i] & mask)) { // edge detect
							cursorKeyHit = 2;
						}
						break;
					case 0xf3: // DOWN�L�[ 0x840
						Y = 0x8; 
						X = 0x40;
						if(!(last_bitsns[i] & mask)) { // edge detect
							cursorKeyHit = 3;
						}
						break;
					case 0xf4: // RIGHT�L�[ 0x880
						Y = 0x8; 
						X = 0x80;
						if(!(last_bitsns[i] & mask)) { // edge detect
							cursorKeyHit = 4;
						}
						break;
					case 0xf8: // OPT.1
						break;
					case 0xf9: // OPT.2
						break;
					case 0xfc: // F6�L�[�������ƁA�f�o�b�O���x���ύX
						if(!(last_bitsns[i] & mask)) { // edge detect
							f6KeyHit = 1;
						}
						break;
					case 0xfd: // �L���L�[�������ƁAPC���ӂ̃������_���v
						if(!(last_bitsns[i] & mask)) { // edge detect
							kigoKeyHit = 1;
						}
						break;
					case 0xfe: // HELP�L�[�������ƃe�L�X�g�\���؂�ւ�
						if(!(last_bitsns[i] & mask)) { // edge detect
							helpKeyHit = 1;
						}
						break;
					case 0xff: // �o�^�L�[�ɂ��I��
						_setTextPlane(1);
						return 1;
					default:
						break;
					}
				}
				if (Y != 0xf) { // else�ɂ��Ă̓_��
					KEYSNS_tbl_ptr[Y] &= ~X;
				}
			}
			mask <<= 1;
		}
	}
	if (hitkey)
	{
		// �������͂�ǂݎ̂ĂāA�L�[�o�b�t�@����ɂ���
		_dos_kflushio(0xff);
	}

	if (f6KeyHit) {
		if(shiftKeyPress) {
			// �V�t�g�L�[�Ɠ��������̏ꍇ�́A�f�o�b�O���O���x����������
			debug_log_level = max(0, debug_log_level - 1);
		} else {
			// ����ȊO�̏ꍇ�́A�f�o�b�O���O���x�����グ��
			debug_log_level = min(3, debug_log_level + 1);
		}
		printf("�f�o�b�O���O���x��=%d\n", debug_log_level);
	}

	if (kigoKeyHit) {
		printf("\n");
		printf("loop count=%08d\ncycle=%08ld wait=%ld\n", emuLoopCounter, cpu_cycle_last, cpu_cycle_wait);
		printf("COUNTER=%08x, inttick=%08d\n", counter, ms_vdp_interrupt_tick);
		dump(pc >> 16, pc & 0x1fff);
	}

	if (helpKeyHit) {
		_toggleTextPlane();
	}

	if (shiftKeyPress && cursorKeyHit ) {
		_moveTextPlane(cursorKeyHit);
	}

	if( disk_change == 0) {
		printf("�f�B�X�N��r�o���܂�\n");
		disk_container->eject_disk(disk_container);
	} else if (disk_change > 0) {
		printf("�f�B�X�N��ύX���܂�\n");
		disk_container->change_disk(disk_container, disk_change - 1);
	}

	for(i=0;i<16;i++) {
		last_bitsns[i] = ((uint8_t*)0x800)[i];	// IOCS BITSNS�̃��[�N�G���A���ڎQ�Ƃ��A��O�̒l���o���Ă���
	}

	return 0;
}

// �������_���v
void dump(unsigned int page, unsigned int pc_8k) {
	int i,j;
	int pcbase,pc;
	int data;
	unsigned int current_int_skip_counter = int_skip_counter;
	static unsigned int last_int_skip_counter = 0;
	int rd,wr;
	char process_type_char[] = {'E','S','D','?'};

	wr = interrupt_history_wr;
	for(i=0;i<8;i++) {
		unsigned int tick;
		unsigned int process_type; // 0: EI��ԂŊ��荞�݂���������, 1: EI���������X�L�b�v, 2: DI��ԂŊ��荞�݂��X�L�b�v
		unsigned int counter;
		rd = (interrupt_history_rd + i) % 32;
		if (rd == wr) {
			break;
		}
		tick = interrupt_history_ptr[rd].int_tick;
		process_type = interrupt_history_ptr[rd].process_type;
		if (process_type > 2 ) {
			process_type = 3;
		}
		counter = interrupt_history_ptr[rd].emu_counter;
		printf("[%04x]%c:%06d >",tick,process_type_char[process_type],counter);
	}
	printf("\n");

	pc = (page << 13) | (pc_8k & 0x1fff);
	pcbase = (pc & 0xfff0);
	printf("PAGE=%04x, PC=%04x, EXEC=%04d SKIP=%04d (+%04d)\n", page, (pc&0xffff), int_exec_counter, current_int_skip_counter,current_int_skip_counter - last_int_skip_counter);
	last_int_skip_counter = current_int_skip_counter;
	for (i = -1; i < 2; i++)
	{
		printf("%04x: ", pcbase + i * 16);
		for (j = 0; j < 16; j++)
		{
			data = ms_memmap_read8(pcbase + i * 16 + j);
			printf("%02x ", data);
		}
		printf("\n");
	}
}

static uint8_t keyboard_led_counter = 0;
static uint8_t last_led_caps = 0;
static uint8_t last_led_kana = 0;

void sync_keyboard_leds() {
	if (keyboard_led_counter == 0) {
		keyboard_led_counter = 3;
		if( last_led_caps != ms_peripherals_led_caps) {
			ms_iocs_ledctrl(0x3, ms_peripherals_led_caps);
			last_led_caps = ms_peripherals_led_caps;
		}
		if( last_led_kana != ms_peripherals_led_kana) {
			ms_iocs_ledctrl(0x0, ms_peripherals_led_kana);
			last_led_kana = ms_peripherals_led_kana;
		}
	} else {
		keyboard_led_counter--;
	}
}

// �e�L�X�g�\���؂�ւ�
unsigned short* VCON_R02 = (unsigned short*)0x00e82600;

static unsigned short debug_log_level_bup;

void _toggleTextPlane(void) {
	static int textPlaneMode = 1;

	textPlaneMode = (textPlaneMode + 1) % 2;
	if (textPlaneMode == 0) {
		// �e�L�X�g�\��OFF�ɂ��鎞�ɁA�f�o�b�O���O��OFF�ɂ���
		debug_log_level_bup = debug_log_level;
		debug_log_level = 0;
	}
	else {
		debug_log_level = debug_log_level_bup;
	}
	_setTextPlane(textPlaneMode);
}

void _moveTextPlane(int cursorKeyHit) {
	static uint16_t x = 0;
	static uint16_t y = 0;
	int diff = 64;

	switch (cursorKeyHit)
	{
	case 1:
		// ��
		if (x >= diff) {
			x -= diff;
		} else {
			x = 0;
		}
		break;
	case 2:
		// ��
		if (y >= diff) {
			y -= diff;
		} else {
			y = 0;
		}
		break;
	case 3:
		// ��
		if (y <= 512 - 192 - diff) { // �K��
			y += diff;
		} else {
			y = 512 - 192;
		}
		break;
	case 4:
		// �E
		if (x <= 512 - 192 - diff) { // �K��
			x += diff;
		} else {
			x = 512 - 192;
		}
		break;
	}
	CRTR_10 = x;
	CRTR_11 = y;
}

void _setTextPlane(int textPlaneMode) {
	switch (textPlaneMode)
	{
	case 0:
		// �e�L�X�g�\��OFF
		*VCON_R02 &= 0xffdf;
		vdp->tx_active = 0;
		break;
	case 1:
		// �e�L�X�g�\��ON
		*VCON_R02 |= 0x0020;
		vdp->tx_active = 1;
		break;
	}
}

int file_exists(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

void set_system_roms() {
	if (file_exists(mainrom_user) && file_exists(subrom_user)) {
		// Load user-provided ROMs
		printf("�w�肳�ꂽBIOS ROM��������܂����B%s �� %s���g�p���܂��B\n", mainrom_user, subrom_user);
		allocateAndSetROM(mainrom_user, ROM_TYPE_NORMAL_ROM, 0x00, -1, 0);
		allocateAndSetROM(subrom_user, ROM_TYPE_NORMAL_ROM, 0x03, 1, 0);
		if (file_exists(diskbios_user)) {
			printf("�w�肳�ꂽ�f�B�X�NBIOS ROM��������܂����B%s���g�p���܂��B\n", diskbios_user);
			int i;
			for(i=0;i<diskcount;i++) {
				printf("�f�B�X�N�C���[�W %d: %s\n", i, diskimages[i]);
			}
			// �f�B�X�N�R���e�i�̏�����
			disk_container = ms_disk_container_alloc();
			if (disk_container == NULL) {
				printf("���������m�ۂł��܂���B\n");
				ms_exit();
				return;
			}
			ms_disk_container_init(disk_container, diskcount, diskimages);

			allocateAndSetDISKBIOSROM(diskbios_user, disk_container);
		}
    } else {
        // Load default CBIOS ROMs
		printf("CBIOS ROM���g�p���܂��B\n");
		allocateAndSetROM(mainrom_cbios, ROM_TYPE_NORMAL_ROM, 0x00, -1, 0);
		allocateAndSetROM(cbioslogo, ROM_TYPE_NORMAL_ROM, 0x00, -1, 2);
		allocateAndSetROM(subrom_cbios, ROM_TYPE_NORMAL_ROM, 0x03, 1, 0);
    }
}

extern int filelength(int fh);

/*
	for debug
*/
void dumpsp_c(uint8_t *const registers, uint32_t sp, uint32_t a0, uint32_t a4, uint32_t d7, uint32_t d4, uint32_t d6)
{
	int i;
	for (i = 0; i < 16; i++)
	{
		printf("%02x ", registers[i]);
	}
	printf("   : SP=%04x a0=%08x a4=%08x d7=%08x d4=%08x d6=%08x\n", sp, a0, a4, d7, d4, d6);
}