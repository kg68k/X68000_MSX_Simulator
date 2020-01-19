/*

		�l�r�w.�r���������������� [[ MS ]]
				[[ �����V�X�e�� ]]
								
	ver. 0.01	prpgramed by Kuni.
										1995.3.5

*/

#include <stdio.h>
#include <string.h>
#include <sys/iocs.h>
#include <sys/dos.h>
#include "ms.h"

void *MMem;						/* ���C���������̐擪�A�h���X			*/
void *VRAM;						/* �u�q�`�l�̐擪�A�h���X				*/

char fontfile[] = "font.fon";
					
char command[256] = "";
void (*commands[64])( int, char *[]);	/* ���ꂼ��̃R�}���h�̊֐��ւ̃|�C���^�z��	*/
char command_name[64][10]={"ASSIGN","ATDIR" ,"ATTRIB","BASIC"   ,"BUFFERS","CD",
							"CHDIR" ,"CHKDSK","CLS"   ,"COMMAND2","CONCAT" ,"COPY",
							"DATE"  ,"DEL"   ,"DIR"   ,"ECHO"    ,"ERA"    ,"ERASE",
							"EXIT"  ,"FORMAT","HELP"  ,"IF"      ,"MD"     ,"MKDIR",
							"MODE"  ,"MOVE"  ,"MVDIR" ,"PATH"    ,"PAUSE"  ,"RAMDISK",
							"RD"    ,"RD"    ,"REM"   ,"REN"     ,"RENAME" ,"RMDIR",
							"RNDIR" ,"SET"   ,"TIME"  ,"TYPE"    ,"UNDEL"  ,"VER",
							"VERIFY","VOL"};

int	com;
int	ssp;						/* �X�[�p�[�o�C�U�[���[�h�֍s�����Ɏg�p 	*/

int screen_mode;				/* �l�r�w�ł̃X�N���[�����[�h				*/
								/* �U�W�ł́A0�`7 �̎� 512*512    16c		*/
								/* 			 8�`12�̎� 512*512 65536c�ɂ���	*/

int width_size;					/* �����\����								*/
								
void do_command( char *command ) {
int i,j;
char *arguments[128];			/* ���͂��ꂽ�R�}���h�̃X�y�[�X���k�������ς��A	*/
								/* ���ꂼ��̕�����̐擪�A�h���X�� arguments[]	*/
	j = 0;						/* �Ƃ����|�C���^�z��֓����B					*/
	i = 0;
	while( command[i] == ' ' ) {
		i++;						/* �A������X�y�[�X���X�L�b�v				*/
	}
	arguments[j++] = command + i;	/* arguments[0] �͑������ւ̃|�C���^		*/
	for(;i<com; i++) {
		if(command[i] == ' ') {
			command[i++] ='\0';		/* �Ƃ肠���������̏I��Ƀk��������t����	*/ 
			while( command[i] == ' ' ) {
				i++;				/* �A������X�y�[�X���X�L�b�v				*/
			}
			arguments[j++] = command + i;
		}
	}
	arguments[j] = 0;				/* �Ō�̃|�C���^�̓k���|�C���^				*/
									/* j �͈����̐�								*/
	if( *arguments[0] == '\0')		/* �X�y�[�X�ȊO�������͂���Ă��Ȃ�����		*/
		return;						/* ���^�[��									*/

	for(i=0; i<64; i++) {
		if( stricmp( arguments[0], command_name[i]) == 0) {
			(*commands[i])( j, arguments);
			return;
		}
	}
	nothing();
}

void screen( int mode) {
	set_screen( mode);			/* �A�Z���u���̃��[�`�����Ăяo��	*/
	screen_mode = mode;
}

void width( int size) {
	set_width( size);
	width_size = size;
}

/* ==========================  ���C��  ==================================== */

void main() {
int crtmod;							/* ���݂̉�ʏ�� 0=256*256 1=512*512	*/
int a,i;
unsigned char sym[2] = "0";
unsigned char hex[17] = "0123456789ABCDEF";

	MMem = _dos_malloc( 0x10000);
	if( MMem > (void *)0x81000000 ) {		/* �U�S�j�̃��C�����������m��			*/
 		printf("���������m�ۂł��܂���\n");
		return;
	}

	VRAM = vdp_init( fontfile);			/* �u�q�`�l�p�̃������̂̊m�ۂȂ�		*/

	ssp = _iocs_b_super( 0);

	login( MMem, VRAM);					/* �V�X�e���̏�����				*/

	display_set();

	screen( 0);							/* �X�N���[�����[�h���O��		*/

	width( 80);

	command_ms();						/* command.ms �̎��s			*/

}
