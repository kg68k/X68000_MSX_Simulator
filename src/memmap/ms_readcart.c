/*

		�l�r�w.�r���������������� [[ MS ]]
				[[ �J�[�g���b�W�ǂݍ��݃��[�`�� ]]
											programed by Kuni.
														1995.12.06
*/

#include <stdio.h>
#include <string.h>
//#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <iocslib.h>
#include <doslib.h>
#include "ms_memmap.h"

void read_cartridge( char *, void *, int, int, int);
void ms_memmap_set_cartridge( void *, int, int, int);

/*	�J�[�g���b�W�t�@�C�����������郋�[�`��			*/

void mount_cartridge( ) {
//DIR directory;
//DIR *p_directory;

/*	p_directory = opendir();	*/

/*	if( (ccf_buff = _dos_malloc( ccf_length=filelength( ccf_fh))) >(void *)0x81000000 ) { */
/*		printf("���������m�ۂł��܂���B\n");	*/
/*		return;	*/
/*	}	*/

	
}

void mount_cartridge_zantei() {
	read_cartridge( "MAINROM1.ROM", 0, 0x00, 0, 2);
	read_cartridge( "MAINROM2.ROM", 0, 0x00, 1, 2);
	read_cartridge( "SUBROM.ROM", 0, 0x0d, 0, 2);
}

int filelength(int fh) {
	struct stat st;
	if( fstat(fh, &st) == -1) {
		return -1;
	}
	return st.st_size;
}

/*
 *	�T�[�`���ꂽ�t�@�C�������ۂɓǂݍ��ރ��[�`��
 *
 *	path_crt	�J�[�g���b�W�t�@�C���̃p�X��
 *	ccf_buff	�ǂݍ��񂾃R���t�B�O�t�@�C���̃A�h���X�B�O�̎��R���t�B�O�t�@�C���͂Ȃ��B
 *	location	�ǂݍ��ރX���b�g�ʒu�B��{�X���b�g�~�S�{�g���X���b�g
 *	page		�ǂݍ��ރy�[�W�Bccf�t�@�C��������ꍇ�͂�����̎w���D�悷��B
 *	kind		�J�[�g���b�W�̎�ށBccf�t�@�C��������΂����炪�D��	
 */
void read_cartridge(char *path_crt, void *ccf_buff, int location, int page, int kind){
	int crt_fh;									/* �J�[�g���b�W�t�@�C���� fh ������	*/
	int ccf_fh;									/* �R���t�B�O�t�@�C���� fh ������	*/
	int crt_length;
	void *crt_buff;

	crt_fh = open( path_crt, O_RDONLY | O_BINARY);
	if( ccf_fh == -1) {
		printf("�t�@�C����������܂���B\n");
		return;
	}
	if( ccf_buff == 0) {
		crt_length = filelength(crt_fh);
		if(crt_length == -1) {
			printf("�t�@�C���̒������擾�ł��܂���B\n");
			return;
		}
		if( ( crt_buff = (void*)_dos_malloc( crt_length + MS_MEMMAP_HEADER_LENGTH ) ) == NULL) {
			printf("���������m�ۂł��܂���B\n");
			return;
		}
		read( crt_fh, crt_buff + MS_MEMMAP_HEADER_LENGTH, crt_length);
		ms_memmap_set_cartridge( crt_buff, location, page, kind);	/* �A�Z���u���̃��[�`����	*/

		close( crt_fh);

		return;
	}

}
