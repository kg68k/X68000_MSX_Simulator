/*

		�l�r�w.�r���������������� [[ MS ]]

			ver.0.01	prpgramed by Kuni.
											1994.10.24

*/


#include <stdio.h>
#include <sys/iocs.h>
#include <sys/dos.h>

char opening[5][33] = {	"Msx Simulater For X680x0\n",
						"                   ver 1.00  \n",
						"System Simulation : MSX2      \n",
						" CPU   Simulation : R800      \n",
						"  OS   Simulation : MSX-DOS2  \n"
					};

extern void *commands[];
extern char *command;

extern int ssp;
extern int com;

/*				��ʂ�������				*/
void display_set() {
int i,j;

	_iocs_apage( 0);

	_iocs_tgusemd( 0, 2);
	_iocs_crtmod( 4);
	_iocs_b_curoff();
	_iocs_g_clr_on();
	_iocs_sp_init();
	_iocs_sp_on();

	_iocs_skey_mod(0,0,0);
}

/*	�t�@�C���ꗗ��\������֐�	*/

void dir() {


}

/*
_files:
	link	a6,#0
	movem.l	d0-d7/a0-a6,-(sp)

	movea.l	8(a6),a2	* a2 = ���ʂ�Ԃ��z��ւ̃|�C���^(�t�@�C���l�[��)
	movea.l	12(a6),a4	* a4 = ���ʂ�Ԃ��z��ւ̃|�C���^(�t�@�C���̎��)
				*	0--- ���ʂ̃t�@�C��
				*	1--- ced�t�@�C��
				*	2--- �f�B���N�g��
				*	3--- �S�ēǂݍ��񂾂��Ƃ�����

	move.w	#%0011_0000,-(sp)	* �ʏ�̃t�@�C���ƃf�B���N�g��������
	pea.l	fnamebuf2
	pea.l	filebuf
	DOS	_FILES
	lea.l	10(sp),sp

loopf:	tst.l	d0
	bmi	donef

	lea.l	filebuf+30,a3
	clr.w	d2
	move.w	#23,d0
loopf1:
	move.b	(a3)+,d1
	cmpi.b	#'.',d1			* �g���q�ɂԂ�����
	beq	exp
	cmpi.b	#0,d1			* �k�������ɂԂ�����
	beq	null
	move.b	d1,(a2)+

	subq.w	#1,d0
	bne	loopf1

	clr.l	d0			* �ʏ�̃t�@�C��

nextf:
	move.l	d0,(a4)+
	lea.l	filebuf+21,a3
	move.b	(a3),d1
	and.b	#%0001_0000,d1		* �f�B���N�g�����H
	beq	normal
	move.l	#2,-4(a4)		* �f�B���N�g��������
normal:
	pea.l	filebuf
	DOS	_NFILES
	addq.l	#4,sp

	bra	loopf

donef:
	move.l	#3,(a4)			* �t�@�C���̎�ނR�́A���ׂēǂݐ؂������Ƃ�����
	movem.l	(sp)+,d0-d7/a0-a6
	unlk	a6

	rts

*	�g���q�̏���
exp:
	subq.w	#5,d0
loope:
	move.b	#' ',(a2)+
	subq.w	#1,d0
	bne	loope

	move.b	d1,(a2)+
	move.w	#4,d0
loopf2:
	move.b	(a3)+,(a2)+
	subq.w	#1,d0
	bne	loopf2

	move.b	-4(a2),d0
	cmpi.b	#'c',d0
	beq	next_e
	cmpi.b	#'C',d0
	bne	not_ced			* �P�����ڂ�'c'�ł�'C'�ł��Ȃ�
next_e:
	move.b	-3(a2),d0
	cmpi.b	#'e',d0
	beq	next_d
	cmpi.b	#'E',d0
	bne	not_ced			* �P�����ڂ�'e'�ł�'E'�ł��Ȃ�
next_d:
	move.b	-2(a2),d0
	cmpi.b	#'d',d0
	beq	next_ced
	cmpi.b	#'D',d0
	bne	not_ced			* �P�����ڂ�'d'�ł�'D'�ł��Ȃ�

next_ced:
	move.l	#1,d0			* ced �t�@�C��
	bra	nextf

not_ced:
	clr.l	d0
	bra	nextf			* �ʏ�̃t�@�C��

*	�k�������ɂԂ��������̏���
null:
	subq.w	#1,d0
loopn:
	move.b	#' ',(a2)+		* �t�@�C���̍Ō�܂ł��X�y�[�X�Ŗ��߂�
	subq.w	#1,d0
	bne	loopn

	move.b	#0,(a2)+		* �k������������

	clr.l	d0			* �ʏ�̃t�@�C��
	bra	nextf

*/

void assign() {
}

void atdir() {
}

void attrib() {
}

void basic() {
}

void buffers() {
}

void chdir() {
}

void chkdsk() {
}


void exit() {
/*	_iocs_b_super( ssp);			*/
	_iocs_b_consol(0, 0, 64,32);
	_dos_exit();
}

void nothing() {						/* �Ȃɂ����Ȃ��R�}���h			*/
char errmes[] = "\ncommand or file not found\n";

	print( errmes);

	return;
}

/****************** �R�}���h�ݒ� ******************/
/*
void *commands[64];			 ���ꂼ��̃R�}���h�̊֐��ւ̃|�C���^�z��	
char command_name[64][10]={"ASSIGN","ATDIR" ,"ATTRIB","BASIC"   ,"BUFFERS","CD",
							"CHDIR" ,"CHKDSK","CLS"   ,"COMMAND2","CONCAT" ,"COPY",
							"DATE"  ,"DEL"   ,"DIR"   ,"ECHO"    ,"ERA"    ,"ERASE",
							"EXIT"  ,"FORMAT","HELP"  ,"IF"      ,"MD"     ,"MKDIR",
							"MODE"  ,"MOVE"  ,"MVDIR" ,"PATH"    ,"PAUSE"  ,"RAMDISK",
							"RD"    ,"RD"    ,"REM"   ,"REN"     ,"RENAME" ,"RMDIR",
							"RNDIR" ,"SET"   ,"TIME"  ,"TYPE"    ,"UNDEL"  ,"VER",
							"VERIFY","VOL"};
*/

void command_init() {
int i;

	for(i=0;i<64;i++)
		commands[i] = nothing;

	commands[ 0] = assign;
	commands[ 1] = atdir;
	commands[ 2] = attrib;
	commands[ 3] = basic;
	commands[ 4] = buffers;
	commands[ 5] = chdir;
	commands[ 6] = chdir;
	commands[18] = exit;
}

void command_ms() {
int i,a;

	command_init();						/* �R�}���h�ւ̃|�C���^�z��̏������Ȃ�	*/

	for( i=0; i<5; i++) {
		print( &opening[i]);
	}

	for(;;) {
		print("A>");
		com = 0;							/* ���ݓ��͂���Ă��镶���� 	*/

		while(1) {
			a = _dos_inkey();
			if( a >= 32 ) {
				if(com < 255)
					command[com++] = (char)a;
				put( a);					/* �����\��						*/
			} else {
				switch (a) {
					case  8: if( com != 0) {
								com--;
								put( a);
							}
							break;
					case 13: goto next;
				}
			}
			if ((_iocs_bitsns( 0) & 2) == 2)	/* �d�����L�[��������Ă�����I��		*/
				/* exit();*/
				goto end;
		}

	next:
		command[com++] = '\0';				/* ������̍Ō�� null �������	*/
		print("\n");						/* ���s							*/
		do_command( command);
	}
end:
	_iocs_b_super( ssp);
	_iocs_b_consol(0, 0, 64,32);
}
