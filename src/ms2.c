/*

		�l�r�w.�r���������������� [[ MS ]]
				[[�X�^�[�g�A�b�v�v���O���� ]]
								
	ver. 0.01	prpgramed by Kuni.
										1995.9.15

*/

#include <stdio.h>
#include <string.h>
#include <sys/iocs.h>
#include <sys/dos.h>

#define	segments	4

int	ssp;						/* �X�[�p�[�o�C�U�[���[�h�֍s�����Ɏg�p 	*/

void main() {
void *MMem,*VideoRAM,*MainROM1,*MainROM2,*SUBROM;
int a,i;

	ssp = _iocs_b_super( 0);

	initialize();								/* �V�X�e���̏�����				*/

	MMem = _dos_malloc( 64*1024 + 8*segments);	/* �U�S�j + �W�o�C�g�����Z�O�����g��	*/
	if( MMem > (void *)0x81000000 ) {
 		printf("���������m�ۂł��܂���\n");
		ms_exit();
	}
	MMemSet( MMem, (int)segments);			/* �A�Z���u���̃��[�`���ֈ����n��		*/

	VideoRAM = _dos_malloc( 128*1024);		/* �u�q�`�l �P�Q�W�j 					*/
	if( VideoRAM > (void *)0x81000000 ) {
 		printf("���������m�ۂł��܂���\n");
		ms_exit();
	}

	VDPSet( VideoRAM);						/* �A�Z���u���̃��[�`���ֈ����n��		*/
											/* ��ʂ̏�������						*/

	mount_cartridge_zantei();
					
	if( PSG_INIT() != 0)
		printf("�o�r�f�̏������Ɏ��s���܂���\n");

	emulate();
	
}
