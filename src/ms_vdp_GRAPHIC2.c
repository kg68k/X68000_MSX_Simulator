/*s_p_GRAPHIC2

set_CRT_jpt:
	set_GRAPHIC2

VDP_G5_jpt

read_rot:
	.dc.l	r_TEXT1			* �u�q�`�l�ǂݍ��݃��[�`���̃A�h���X

write_rot:
	.dc.l	w_TEXT1			* �u�q�`�l�������݃��[�`���̃A�h���X

set_gram_add_rot:
	.dc.l	s_TEXT1

VDP_command_jpt_add:
*/


/*
 MSX�̉�ʃ��[�h���Ƃɐ؂�ւ��鏈���Q
 
 * init
 	* X68000����CRTC�p�����[�^�ݒ�Ȃǂ��s��
 * VRAM�ǂݍ���
 * VRAM��������
	* VDP���W�X�^�̒l�ƃA�h���X���r���A�ȉ��𕪊�
		* ��ʏ��������n
		* �X�v���C�g�̏�������
 * VDP�R�}���h�̎��s
 * 

 */