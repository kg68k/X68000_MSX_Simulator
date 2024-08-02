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
#include <ctype.h>
#include <iocslib.h>
#include <doslib.h>
#include <getopt.h>
#include "ms.h"
#include "ms_R800.h"

#define NUM_SEGMENTS 4

extern void writeSpritePattern(unsigned char* p, int offset, unsigned int pattern);
extern void writeSpriteAttribute(unsigned char* p, int offset, unsigned int attribute);
extern int readMemFromC(int address);

void ms_exit( void);

// �������֘A
int mem_initialized = 0;
int ms_memmap_init();
void ms_memmap_deinit(void);

void ms_memmap_set_main_mem( void *, int);

// I/O�֘A
int io_initialized = 0;
int ms_iomap_init();
void ms_iomap_deinit(void);

// VDP�֘A
int vdp_initialized = 0;
int ms_vdp_init( void *);
void ms_vdp_deinit(void);

void initSprite(void);

// PSG�֘A
int psg_initialized = 0;
int ms_psg_init( void);
void ms_psg_deinit(void);

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


int emuLoop(unsigned int pc, unsigned int counter);
void allocateAndSetROM(size_t size, const char* romFileName, int param1, int param2, int param3);

void _toggleTextPlane(void);
void _setTextPlane(int textPlaneMode);

volatile extern unsigned short ms_vdp_interrupt_tick;
volatile extern unsigned short ms_vdp_vsync_rate;
volatile extern unsigned int int_block_count;
volatile extern unsigned short debug_log_level;
volatile extern unsigned short host_rate;
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
	fprintf(stderr, "Usage: %s [-m1 MAINROM1_PATH] [-m2 MAINROM2_PATH] [-r11 ROM1_PATH] [-r12 ROM2_PATH]\n", progname);
	fprintf(stderr, " --vsrate vsync rate (1-60)\n");
	fprintf(stderr, "    1: every frame, 2: every 2 frames, ...\n");
	fprintf(stderr, "    default is 1.\n");
	fprintf(stderr, " --intblock block cycle (1-9999)\n");
	fprintf(stderr, "    default is 2048 cycles.\n");
	fprintf(stderr, " --hostrate host key operation rate (1-60)\n");
	fprintf(stderr, "    1: every frame, 2: every 2 frames, ...\n");
	fprintf(stderr, "    default is 3.\n");
	fprintf(stderr, " --hostdelay host key interruption delay cycle (1-999)\n");
	fprintf(stderr, "    default is 20 cycles.\n");
	fprintf(stderr, " --disablekey\n");
	fprintf(stderr, "    disable key input for performance test.\n");
//	fprintf(stderr, " --debuglevel N\n");
//	fprintf(stderr, "    0: None, 1: Info, 2: Debug, 3: Fine.\n");
	exit(EXIT_FAILURE);
}

static unsigned char *MMem;
static unsigned char *VideoRAM;
static unsigned char *MainROM1;
static unsigned char *MainROM2;
static unsigned char *SUBROM;
static unsigned char *SLOT1_1;
static unsigned char *SLOT1_2;
static unsigned char *ROM;

int disablekey = 0;

/*
	���C���֐�

	�N���I�v�V�����F
		-m1 filename	: MAIN ROM���w�肵�����̂ɕύX���܂�(�O��16K)
		-m2 filename	: MAIN ROM���w�肵�����̂ɕύX���܂�(�㔼16K)
		-rxx filename	: �X���b�g��filename���Z�b�g���܂�
			xx: 10�̈� = �X���b�g�ԍ�, 1�̈� = �y�[�W�ԍ�
			��F-r11 GAME1.ROM	: �X���b�g1-�y�[�W1��GAME1.ROM���Z�b�g				
*/
int main(int argc, char *argv[]) {
	int i, j;
	char *mainrom1_path = "MAINROM1.ROM"; // �f�t�H���g��MAIN ROM1�p�X
	char *mainrom2_path = "MAINROM2.ROM"; // �f�t�H���g��mAIN ROM2�p�X
	char *cartridge_path[4][4]; // �J�[�g���b�W�̃p�X
	int opt;
    const char* optstring = "hm:r:" ; // optstring���`���܂�
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

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			cartridge_path[i][j] = NULL;
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
		case 'm': // -mN �I�v�V����
			if (strlen(optarg) == 1 && isdigit(optarg[0]))
			{ 
				if (argv[optind] != NULL)
				{
					if (optarg[0] == '1')
					{
						mainrom1_path = argv[optind++];
					} else if (optarg[0] == '2')
					{
						mainrom2_path = argv[optind++];
					}
				}
			}
			break;
		case 'r': // -rNN �I�v�V����
			if (strlen(optarg) == 2 && isdigit(optarg[0]) && isdigit(optarg[1])) {
				// -r �ɑ���2���̐������擾
				int num = atoi(optarg);
				int slot = (num / 10);
				int page = num % 10; // 1�̈ʂ�page
				if ( slot >= 0 && slot <= 3 && page >= 0 && page <= 3) {
					// ���̈����iROM�t�@�C�����j���擾
					if (argv[optind] != NULL)
					{
						cartridge_path[slot][page] = argv[optind++];
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
				if (host_delay < 1 || host_delay > 999) {
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

	if (_iocs_b_super(0) < 0)
	{
		printf("�X�[�p�[�o�C�U�[���[�h�Ɉڍs�ł��܂���ł���\n");
		return -1;
	}

	/*
	 �������V�X�e���̏�����
	 */
	mem_initialized = ms_memmap_init();
	if (mem_initialized == 0)
	{
		printf("�������V�X�e���̏������Ɏ��s���܂���\n");
		ms_exit();
	}

	MMem = new_malloc(64 * 1024 + 8 * NUM_SEGMENTS); /* �U�S�j + �W�o�C�g�����Z�O�����g��	*/
	if (MMem > (unsigned char *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		ms_exit();
	}
	ms_memmap_set_main_mem(MMem, (int)NUM_SEGMENTS); /* �A�Z���u���̃��[�`���ֈ����n��		*/

	/*
	 I/O�V�X�e���̏�����
	 */
	io_initialized = ms_iomap_init();
	if (io_initialized == 0)
	{
		printf("I/O�V�X�e���̏������Ɏ��s���܂���\n");
		ms_exit();
	}

	/*
	 VDP�V�X�e���̏�����
	*/
	VideoRAM = new_malloc(128 * 1024); /* �u�q�`�l �P�Q�W�j 					*/
	if (VideoRAM > (unsigned char *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		ms_exit();
	}
	vdp_initialized = ms_vdp_init(VideoRAM);
	if (vdp_initialized == 0)
	{
		printf("VDP�V�X�e���̏������Ɏ��s���܂���\n");
		ms_exit();
	}

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
	 ROM�̃Z�b�g
	 */

	/* �l�`�h�m�q�n�l�i�O��:16K, �㔼:16K */
	allocateAndSetROM(16 * 1024, mainrom1_path, 2, 0x00, 0);
	allocateAndSetROM(16 * 1024, mainrom2_path, 2, 0x00, 1);
	/* �r�t�a�q�n�l 16K */
	allocateAndSetROM(16 * 1024, "SUBROM.ROM", 2, 0x0d, 0);

	/* ��{�X���b�g1-�y�[�W1��ROM�z�u (32KBytes�Q�[���̑O��16K�Ȃǁj	*/
	//	allocateAndSetROM( 16 * 1024,"GAME1.ROM", (int)2, (int)0x04, (int)1 );
	/* ��{�X���b�g1-�y�[�W1��ROM�z�u (32KBytes�Q�[���̌㔼16K�Ȃǁj	*/
	//	allocateAndSetROM( 16 * 1024,"GAME2.ROM", (int)2, (int)0x04, (int)2 );

	for ( i = 0; i < 4; i++) {
		for ( j = 0; j < 4; j++) {
			if ( cartridge_path[i][j] != NULL) {
				allocateAndSetROM(16 * 1024, cartridge_path[i][j], 2, i<<2, j);
			}
		}
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

	printf("emu-ini\n");
	ms_cpu_init();
	printf("emu-ini exit\n");

	initSprite();

	if (1) {
		ms_cpu_emulate(emuLoop);
	} else {
		debugger();
	}

	printf("�I�����܂�\n");

	_iocs_crtmod(0x10);

	ms_exit();
}

void ms_exit() {
	if ( psg_initialized ) {
		ms_psg_deinit();
	}
	if ( vdp_initialized ) {
		ms_vdp_deinit();
	}
	if ( io_initialized ) {
		ms_iomap_deinit();
	}
	if ( mem_initialized ) {
		ms_memmap_deinit();
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
	//0 [    ][ESC ][1   ][2   ][3   ][4   ][5   ][6   ] 
	{   0xf00,0x704,0x002,0x004,0x008,0x010,0x020,0x040},
	//1 [7   ][8   ][9   ][0   ][-   ][^   ][\   ][BS  ] 
	{   0x080,0x101,0x102,0x001,0x104,0x108,0x110,0x720},
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
	//7 [RUP ][RDWN][UNDO][LEFT][UkP  ][RIGT][DOWN][CLR ]
	{   0xf00,0xf00,0xf00,0x810,0x820,0x880,0x840,0xf00},
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
	//e [SHFT][CTRL][OPT1][OPT2][    ][    ][    ][    ]  OPT1=GRAPH
	{   0x601,0x602,0x604,0xf00,0xf00,0xf00,0xf00,0xf00},
	//f [    ][    ][    ][    ][    ][    ][    ][    ]
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00}
};

extern unsigned char* KEYSNS_tbl_ptr;

int divider = 0;

void dump(unsigned int page, unsigned int pc_16k);

int emuLoop(unsigned int pc, unsigned int counter) {
	static int emuLoopCounter = 0;
	int i,j;
	unsigned short map;
	unsigned char X, Y;
	int hitkey = 0;
	static int f6KeyHit = 0, f6KeyHitLast = 0;
	static int kigoKeyHit = 0, kigoKeyHitLast = 0;
	static int helpKeyHit = 0, helpKeyHitLast = 0;
	

	emuLoopCounter++;

	kigoKeyHit = 0;
	helpKeyHit = 0;

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
		int v = _iocs_bitsns(i);
		for ( j = 0; j < 8; j++)
		{
			if ((v & 1) == 1)
			{
				hitkey = 1;
				map = KEY_MAP[i][j];
				Y = (map & 0xf00) >> 8;
				X = (map & 0xff);
				if (Y == 0xf)
				{
					// ����L�[
					switch (X)
					{
					case 0xfc: // F6�L�[�������ƁA�f�o�b�O���x���ύX
						f6KeyHit = 1;
						break;
					case 0xfd: // �L���L�[�������ƁAPC���ӂ̃������_���v
						kigoKeyHit = 1;
						break;
					case 0xfe: // HELP�L�[�������ƃe�L�X�g�\���؂�ւ�
						helpKeyHit = 1;
						break;
					case 0xff: // �o�^�L�[�ɂ��I��
						_setTextPlane(1);
						return 1;
					default:
						break;
					}
				}
				else
				{
					KEYSNS_tbl_ptr[Y] &= ~X;
				}
			}
			v >>= 1;
		}
	}

	if (hitkey)
	{
		// �������͂�ǂݎ̂ĂāA�L�[�o�b�t�@����ɂ���
		_dos_kflushio(0xff);
	}

	if (f6KeyHit && !f6KeyHitLast)
	{
		debug_log_level = (debug_log_level + 1) % 4;
		printf("�f�o�b�O���O���x��=%d\n", debug_log_level);
	}
	f6KeyHitLast = f6KeyHit;

	if (kigoKeyHit && !kigoKeyHitLast)
	{
		printf("\n");
		printf("emu loop counter=%08d\n", emuLoopCounter);
		printf("COUNTER=%08x, inttick=%08d\n", counter, ms_vdp_interrupt_tick);
		dump(pc >> 16, pc & 0x3fff);
	}
	kigoKeyHitLast = kigoKeyHit;

	if (helpKeyHit && !helpKeyHitLast)
	{
		_toggleTextPlane();
	}
	helpKeyHitLast = helpKeyHit;

	return 0;
}

// �������_���v
void dump(unsigned int page, unsigned int pc_16k) {
	int i,j;
	int pcbase,pc;
	int data;
	unsigned int current_int_skip_counter = int_skip_counter;
	static unsigned int last_int_skip_counter = 0;
	int rd,wr;
	char process_type_char[] = {'E','S','D','?'};

	wr = interrupt_history_wr;
	for(i=0;i<32;i++) {
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

	pc = (page << 14) | (pc_16k & 0x3fff);
	pcbase = (pc & 0xfff0);
	printf("PAGE=%04x, PC=%04x, EXEC=%04d SKIP=%04d (+%04d)\n", page, (pc&0xffff), int_exec_counter, current_int_skip_counter,current_int_skip_counter - last_int_skip_counter);
	last_int_skip_counter = current_int_skip_counter;
	for (i = -1; i < 2; i++)
	{
		printf("%04x: ", pcbase + i * 16);
		for (j = 0; j < 16; j++)
		{
			data = readMemFromC(pcbase + i * 16 + j);
			printf("%02x ", data);
		}
		printf("\n");
	}
}

// �e�L�X�g�\���؂�ւ�
unsigned short* VCON_R02 = (unsigned short*)0x00e82600;

void _toggleTextPlane(void) {
	static int textPlaneMode = 1;

	textPlaneMode = (textPlaneMode + 1) % 2;
	_setTextPlane(textPlaneMode);
}

extern short tx_active;

void _setTextPlane(int textPlaneMode) {
	switch (textPlaneMode)
	{
	case 0:
		// �e�L�X�g�\��OFF
		*VCON_R02 &= 0xffdf;
		tx_active = 0;
		break;
	case 1:
		// �e�L�X�g�\��ON
		*VCON_R02 |= 0x0020;
		tx_active = 1;
		break;
	}
}

void allocateAndSetROM(size_t size, const char *romFileName, int kind, int slot, int page)
{
	void *ROM = new_malloc(size + 8);
	if (ROM > (void *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		ms_exit();
	}
	ms_memmap_set_rom(ROM, romFileName, kind, slot, page);
}

/*
 	�X�v���C�g�̏���

	MS.X�́AMSX��256�h�b�g��X68000��512�h�b�g�Ɋg�債�Ă���
	���̂��߁AMSX��8x8�h�b�g�̃X�v���C�g�́AX68000���16x16�h�b�g�ɂȂ�
	MSX�̃X�v���C�g�p�^�[���͍ő�256��`���邱�Ƃ��ł��邪�A
	X68000��16x16�h�b�g�̃X�v���C�g�p�^�[�����ő�128������`�ł����A��������Ȃ��B
	�����ŁAMSX�̃X�v���C�g�͍ő�32�����\���ł��Ȃ����Ƃ𗘗p���A
	X68000�̃X�v���C�g�p�^�[���́A���ݕ\�����̃X�v���C�g�݂̂��`���邱�Ƃɂ���B
	 
 */

unsigned short* X68_SP_PALETTE = (unsigned short*)0x00e82200; // �X�v���C�g/�e�L�X�g�p���b�g
unsigned short* X68_SSR = (unsigned short*)0x00eb0000; // �X�v���C�g�X�N���[�����W�X�^
unsigned int* X68_PCG = (unsigned int*)0x00eb8000;

unsigned int* X68_PCG_buffer;
extern unsigned char sprite_size;	// 0: 8x8, 1: 16x16

/*
 �X�v���C�g�̏�����
 TODO: VDP�֘A�̃\�[�X�𕪗�����
 */
void initSprite(void) {
	int i;

	// X68000�� 1�X�v���C�g(16x16)�p�^�[��������128�o�C�g���K�v
	// MSX�� 256��`�ł��邪�AX68000��128������`�ł��Ȃ����߁A��������ɒ�`�̈������Ă���
	// �\�����ɓ]������悤�ɂ��Ă���
	X68_PCG_buffer = (unsigned int*)malloc( 256 * 32 * sizeof(unsigned int)); 
	if (X68_PCG_buffer > (unsigned int *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		ms_exit();
	}
	// PCG�o�b�t�@�̏�����
	for ( i = 0; i < 256 * 32; i++) {
		X68_PCG_buffer[i] = 0;
	}
	// �X�v���C�g�p���b�g�̏�����
//	for (int i = 1; i < 256; i++) {
//		X68_SP_PALETTE[i] = i;
//	}
}

/*
 �X�v���C�g�p�^�[���W�F�l���[�^�e�[�u���ւ̏�������
     offset: �p�^�[���W�F�l���[�^�e�[�u���̃x�[�X�A�h���X����̃I�t�Z�b�g�o�C�g
     pattern: �������ރp�^�[��(����8bit�̂ݎg�p)
*/
void writeSpritePattern(unsigned char* p, int offset, unsigned int pattern) {
	int i,j;
	int ptNum = offset / 8; // MSX�̃X�v���C�g�p�^�[���ԍ�
	int pLine = offset % 8; // �p�^�[���̉��s�ڂ� 
	int pcgLine = pLine * 2; // MSX��1���C����X68000�ł�2���C��
	unsigned int pLeft=0,pRight=0; // 1���C���̍�4�h�b�g�ƉE4�h�b�g�� X68000��8x8�̃p�^�[��2�ɕϊ�

    // �E�[�̃h�b�g���珈��
	for(i =0; i < 4;i++) {
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
	// �p�^�[���W�F�l���[�^�e�[�u���ւ̏�������
	X68_PCG_buffer[ptNum * 32 + pcgLine+0 + 0] = pLeft;
	X68_PCG_buffer[ptNum * 32 + pcgLine+1 + 0] = pLeft;
	X68_PCG_buffer[ptNum * 32 + pcgLine+0 + 16] = pRight;
	X68_PCG_buffer[ptNum * 32 + pcgLine+1 + 16] = pRight;
}

void writeSpriteAttribute(unsigned char* p, int offset, unsigned int attribute) {
	int i,j;
	int plNum = (offset / 4) % 32; // MSX�̃X�v���C�g�v���[���ԍ�
	int type = offset % 4; // �����̎��

	switch(type) {
		case 0: // Y���W
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+1] = (((attribute + 1 ) & 0xff) * 2) + (i%2)*16 + 16; // MSX��Y���W��2�{, MSX��1���C�����ɕ\�������̂�+1, X68000�̃X�v���C�g�̌��_��(16,16)�Ȃ̂ł��炷
			}
			break;
		case 1: // X���W
			for( i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+0] = ((attribute & 0xff) * 2) + (i/2)*16 + 16; // MSX��X���W��2�{
			}
			// TODO EC�r�b�g�ɂ��ʒu�␳����
			break;
		case 2: // �p�^�[���ԍ�
		case 3: // ����
			// �p�^�[���ԍ��A�J���[���ύX���ꂽ��A���O�Ƀo�b�t�@�ɓW�J���Ă������p�^�[����]��
			unsigned int ptNum = p[(offset & 0x1fffc)+2];
			unsigned int color = p[(offset & 0x1fffc)+3] & 0xf;
			unsigned int colorex = color << 28 | color << 24 | color << 20 | color << 16 | color << 12 | color << 8 | color << 4 | color; // MSX��4�h�b�g��(X68000����2�{��8�h�b�g�Ɋg��)
			if (sprite_size == 0) { // 8x8
				for( i = 0; i < 32; i++) { 
					X68_PCG[plNum*32*4+i] = X68_PCG_buffer[(ptNum & 0xff)*32+i] & colorex;
				}
			} else { // 16x16
				for( i = 0; i < 32*4; i++) { 
					X68_PCG[plNum*32*4+i] = X68_PCG_buffer[(ptNum & 0xfc)*32+i] & colorex;
				}
			}
			break;
		default:
			break;
	}
	if (sprite_size == 0) {
		// 8x8���[�h
		X68_SSR[plNum*4+2] = 0x100 + plNum; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
		X68_SSR[plNum*4+3] = 0x0003; // �X�v���C�g�\��ON
	} else {
		// 16x16���[�h�� X68000��� 32x32�ɂȂ�̂ŁA16x16�̃X�v���C�g��4���ׂĕ\������
		for( i=0; i<4; i++) {
			X68_SSR[plNum*16+i*4+2] = 0x100 + plNum*4+i; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
			X68_SSR[plNum*16+i*4+3] = 0x0003; // �X�v���C�g�\��ON
		}
	}

}

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