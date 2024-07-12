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
#include <x68k/iocs.h>
#include <x68k/dos.h>
#include "ms.h"

#define NUM_SEGMENTS 4

extern void writeSpritePattern(unsigned char* p, int offset, unsigned int pattern);
extern void writeSpriteAttribute(unsigned char* p, int offset, unsigned int attribute);

void emulater_ini(void);
int emulate( int(*)(int));
void ms_init( void);
void ms_exit( void);
void MMemSet( void *, int);
void VDPSet( void *);
void initSprite(void);

/*
  Slot��ROM���Z�b�g���܂��B
	int SetROM( (void *)address, (char *)filename, (int)kind, (int)slot, (int)page);

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
void SetROM( void* address, const char* filename, int kind, int slot, int page);
int PSG_INIT( void);

int emuLoop(int);
void allocateAndSetROM(size_t size, const char* romFileName, int param1, int param2, int param3);

void _toggleTextPlane(void);
void _setTextPlane(int textPlaneMode);

void printHelpAndExit(char* progname) {
	fprintf(stderr, "Usage: %s [-m1 MAINROM1_PATH] [-m2 MAINROM2_PATH] [-r11 ROM1_PATH] [-r12 ROM2_PATH]\n", progname);
	exit(EXIT_FAILURE);
}

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
	void *MMem, *VideoRAM, *MainROM1, *MainROM2, *SUBROM, *SLOT1_1, *ROM, *SLOT1_2;
	int a, i;
	char *mainrom1_path = "MAINROM1.ROM"; // �f�t�H���g��MAIN ROM1�p�X
	char *mainrom2_path = "MAINROM2.ROM"; // �f�t�H���g��mAIN ROM2�p�X
	char *cartridge_path[4][4]; // �J�[�g���b�W�̃p�X
	int opt;

	printf("MSX Simulator\n");

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			cartridge_path[i][j] = NULL;
		}
	}

	// �R�}���h���C���I�v�V�����̉��
	while ((opt = getopt(argc, argv, "m:r:")) != -1)
	{
		switch (opt)
		{
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

	printf("ms_init: enter\n");
	ms_init(); /* �V�X�e���̏�����				*/
	printf("ms_init: exit\n");

	MMem = new_malloc(64 * 1024 + 8 * NUM_SEGMENTS); /* �U�S�j + �W�o�C�g�����Z�O�����g��	*/
	if (MMem > (void *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		ms_exit();
	}
	MMemSet(MMem, (int)NUM_SEGMENTS); /* �A�Z���u���̃��[�`���ֈ����n��		*/

	VideoRAM = new_malloc(128 * 1024); /* �u�q�`�l �P�Q�W�j 					*/
	if (VideoRAM > (void *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		ms_exit();
	}
	printf("VDPSet: enter\n");
	VDPSet(VideoRAM); /* �A�Z���u���̃��[�`���ֈ����n��		*/
					  /* ��ʂ̏�������						*/
	printf("VDPSet: exit\n");

	/* �l�`�h�m�q�n�l�i�O��:16K, �㔼:16K */
	allocateAndSetROM(16 * 1024, mainrom1_path, 2, 0x00, 0);
	allocateAndSetROM(16 * 1024, mainrom2_path, 2, 0x00, 1);
	/* �r�t�a�q�n�l 16K */
	allocateAndSetROM(16 * 1024, "SUBROM.ROM", 2, 0x0d, 0);

	/* ��{�X���b�g1-�y�[�W1��ROM�z�u (32KBytes�Q�[���̑O��16K�Ȃǁj	*/
	//	allocateAndSetROM( 16 * 1024,"GAME1.ROM", (int)2, (int)0x04, (int)1 );
	/* ��{�X���b�g1-�y�[�W1��ROM�z�u (32KBytes�Q�[���̌㔼16K�Ȃǁj	*/
	//	allocateAndSetROM( 16 * 1024,"GAME2.ROM", (int)2, (int)0x04, (int)2 );

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if ( cartridge_path[i][j] != NULL) {
				allocateAndSetROM(16 * 1024, cartridge_path[i][j], 2, i<<2, j);
			}
		}
	}


	printf("PSG\n");
	if (PSG_INIT() != 0)
	{
		printf("�o�r�f�̏������Ɏ��s���܂���\n");
		ms_exit();
	}

	printf("X680x0 MSX�V�~�����[�^�[ with elf2x68k\n");

	printf("emu-ini\n");
	emulater_ini();
	printf("emu-ini exit\n");

	initSprite();

	emulate(emuLoop);

	printf("�I�����܂�\n");

	ms_exit();
}

/*
 X68000�� _BITSNS �œ�����L�[�}�g���N�X��MSX�̃L�[�}�g���N�X�ɕϊ����邽�߂̃}�b�s���O

 - X68000���̃}�g���N�X
	- X68000���n���h�u�b�N �up287 _BITSNS�v �Q��
 - MSX���̃}�g���N�X
	- MSX�f�[�^�p�b�N p12 �u2.5 �L�[�{�[�h�v �Q��
 */
unsigned short KEY_MAP[][8] = {
	//  [    ][ESC ][1   ][2   ][3   ][4   ][5   ][6   ] 
	{   0xf00,0x704,0x002,0x004,0x008,0x010,0x020,0x040},
	//  [7   ][8   ][9   ][0   ][-   ][^   ][\   ][BS  ] 
	{   0x080,0x101,0x102,0x001,0x104,0x108,0x110,0x720},
	//  [TAB ][Q   ][W   ][E   ][R   ][T   ][Y   ][U   ] 
	{   0x708,0x440,0x510,0x304,0x480,0x502,0x540,0x504},
	//  [I   ][O   ][P   ][@   ][ [  ][RET ][ A  ][ S  ] 
	{   0x340,0x410,0x420,0x120,0x140,0x780,0x240,0x501},
	//  [ D  ][ F  ][ G  ][ H  ][ J  ][ K  ][ L  ][ ;+ ] 
	{   0x302,0x308,0x310,0x320,0x380,0x401,0x402,0x180},
	//  [ :* ][ ]  ][ Z  ][ X  ][ C  ][ V  ][ B  ][ N  ] 
	{   0x201,0x203,0x580,0x520,0x301,0x508,0x280,0x408},
	//  [ M  ][ ,< ][ .> ][ /  ][ _  ][ SP ][HOME][DEL ]  HOME=CLS
	{   0x404,0x204,0x208,0x210,0x220,0x801,0x802,0x808}, 
	//  [RUP ][RDWN][UNDO][LEFT][UP  ][RIGT][DOWN][CLR ]
	{   0xf00,0xf00,0xf00,0x810,0x820,0x880,0x840,0xf00},
	//  [(/) ][(*) ][(-) ][(7) ][(8) ][(9) ][(+) ][(4) ]  TEN KEYs
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [(5) ][(6) ][(=) ][(1) ][(2) ][(3) ][ENTR][(0) ]  TEN KEYs
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [(,) ][(.) ][KIGO][TORK][HELP][XF1 ][XF2 ][XF3 ]  �o�^=�G�~�����[�^�I��
	{   0xf00,0xf00,0xf00,0xfff,0xffe,0xf00,0xf00,0xf00},
	//  [XF4 ][XF5 ][KANA][ROME][CODE][CAPS][INS ][HIRA]
	{   0xf00,0xf00,0x610,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [ZENK][BRAK][COPY][ F1 ][ F2 ][ F3 ][ F4 ][ F5 ]  BRAK=STOP
	{   0xf00,0x710,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [ F6 ][ F7 ][ F8 ][ F9 ][ F10][    ][    ][    ]
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [SHFT][CTRL][OPT1][OPT2][    ][    ][    ][    ]  OPT1=GRAPH
	{   0x601,0x602,0x604,0xf00,0xf00,0xf00,0xf00,0xf00},
	//  [    ][    ][    ][    ][    ][    ][    ][    ]
	{   0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00,0xf00}
};

extern unsigned char* KEYSNS_tbl_ptr;

int divider = 0;

int emuLoop(int pc) {
	unsigned short map;
	unsigned char X, Y;
	int hitkey = 0;
	static int helpKeyHit = 0;
	static int helpKeyHitLast = 0;

	helpKeyHit = 0;

	for (int i = 0; i < 0x0f; i++)
	{
		KEYSNS_tbl_ptr[i] = 0xff;
	}
	for (int i = 0; i < 0x0f; i++)
	{
		int v = _iocs_bitsns(i);
		for (int j = 0; j < 8; j++)
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

	if (helpKeyHit && !helpKeyHitLast)
	{
		_toggleTextPlane();
	}
	helpKeyHitLast = helpKeyHit;

	return 0;
}

// �e�L�X�g�\���؂�ւ�
unsigned short* VCON_R02 = (unsigned short*)0x00e82600;

void _toggleTextPlane(void) {
	static int textPlaneMode = 1;

	textPlaneMode = (textPlaneMode + 1) % 2;
	_setTextPlane(textPlaneMode);
}

void _setTextPlane(int textPlaneMode) {
	switch (textPlaneMode)
	{
	case 0:
		// �e�L�X�g�\��OFF
		*VCON_R02 &= 0xffdf;
		break;
	case 1:
		// �e�L�X�g�\��ON
		*VCON_R02 |= 0x0020;
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
	SetROM(ROM, romFileName, kind, slot, page);
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

void initSprite(void) {
	// X68000�� 1�X�v���C�g(16x16)�p�^�[��������128�o�C�g���K�v
	// MSX�� 256��`�ł��邪�AX68000��128������`�ł��Ȃ����߁A��������ɒ�`�̈������Ă���
	// �\�����ɓ]������悤�ɂ��Ă���
	printf("%d\n", sizeof(unsigned int));
	X68_PCG_buffer = (unsigned int*)malloc( 256 * 32 * sizeof(unsigned int)); 
	if (X68_PCG_buffer > (unsigned int *)0x81000000)
	{
		printf("���������m�ۂł��܂���\n");
		ms_exit();
	}
	// PCG�o�b�t�@�̏�����
	for (int i = 0; i < 256 * 32; i++) {
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
	int ptNum = offset / 8; // MSX�̃X�v���C�g�p�^�[���ԍ�
	int pLine = offset % 8; // �p�^�[���̉��s�ڂ� 
	int pcgLine = pLine * 2; // MSX��1���C����X68000�ł�2���C��
	unsigned int pLeft=0,pRight=0; // 1���C���̍�4�h�b�g�ƉE4�h�b�g�� X68000��8x8�̃p�^�[��2�ɕϊ�

    // �E�[�̃h�b�g���珈��
	for(int i =0; i < 4;i++) {
		pRight >>= 8;
		if(pattern & 1) {
			pRight |= (0xff000000);
		}
		pattern >>= 1;
	}
    // �c��̍�4�h�b�g�̏���
	for(int i =0; i < 4;i++) {
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
	int plNum = (offset / 4) % 32; // MSX�̃X�v���C�g�v���[���ԍ�
	int type = offset % 4; // �����̎��

	switch(type) {
		case 0: // Y���W
			for( int i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+1] = ((attribute & 0xff) * 2) + (i%2)*16; // MSX��Y���W��2�{
			}
			break;
		case 1: // X���W
			for( int i=0; i<4; i++) {
				X68_SSR[plNum*16+i*4+0] = ((attribute & 0xff) * 2) + (i/2)*16; // MSX��X���W��2�{
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
				for(int i = 0; i < 32; i++) { 
					X68_PCG[plNum*32*4+i] = X68_PCG_buffer[(ptNum & 0xff)*32+i] & colorex;
				}
			} else { // 16x16
				for(int i = 0; i < 32*4; i++) { 
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
		for( int i=0; i<4; i++) {
			X68_SSR[plNum*16+i*4+2] = 0x100 + plNum*4+i; // �p���b�g0x10-0x1f���g�p����̂� 0x100�𑫂�
			X68_SSR[plNum*16+i*4+3] = 0x0003; // �X�v���C�g�\��ON
		}
	}

}