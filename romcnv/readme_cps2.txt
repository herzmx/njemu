�X�v���C�gROM �ϊ��c�[�� ver.2.2.0 (CPS2PSP�p)

���Ȃ�蔲���ł����A�ϊ��c�[���ł��B

---------------------------------------------------------------------------
�����k�L���b�V�����܂Ƃ߂č쐬����ꍇ

1.cps2conv_all.bat���_�u���N���b�N���ċN�����Ă��������B

2.�t�H���_�I���_�C�A���O���\�������̂ŁAROM�̃t�H���_��I�����Ă��������B

3.�ϊ���Ƃ��J�n���܂��B
  �t�@�C����������Ȃ����ꍇ�̓X�L�b�v���܂��̂ŁA�I���܂ő҂��܂��傤�B

4.cache�t�H���_���� �Q�[����.cache�Ƃ����t�@�C�����쐬����܂��̂ŁA
  /PSP/GAMES/cps2psp/cache �f�B���N�g���ɃR�s�[���Ă��������B
  ����ROM�t�@�C����/PSP/GAMES/cps2psp/roms�ɃR�s�[���Ă��������B

5.�Q�[�������s����΁A�������삵�܂��B

---------------------------------------------------------------------------
�����k�L���b�V�����Q�[�����w�肵�č쐬����ꍇ

1.cps2conv.bat���_�u���N���b�N���ċN�����Ă��������B

2.�t�@�C���I���_�C�A���O���\�������̂ŁA�ϊ��������Q�[����zip�t�@�C����
  �I�����Ă��������B

3.�ϊ���Ƃ��J�n���܂��B
  �Ȃ��A�N���[���Z�b�g�̏ꍇ�́A�eROM�Z�b�g�������t�H���_���ɂ���K�v��
  ����܂��B

4.cache�t�H���_���� �Q�[����.cache�Ƃ����t�@�C�����쐬����܂��̂ŁA
  /PSP/GAMES/cps2psp/cache �f�B���N�g���ɃR�s�[���Ă��������B
  ����ROM�t�@�C����/PSP/GAMES/cps2psp/roms�ɃR�s�[���Ă��������B

5.�Q�[�������s����΁A�������삵�܂��B

---------------------------------------------------------------------------
��L�����܂������Ȃ��ꍇ

1.cps2conv.bat�ɕϊ��������Q�[����zip�t�@�C�����h���b�O���h���b�v����
  ���������B

2.�ϊ���Ƃ��J�n���܂��B
  �Ȃ��A�N���[���Z�b�g�̏ꍇ�́A�eROM�Z�b�g�������t�H���_���ɂ���K�v��
  ����܂��B

3.cache�t�H���_���� �Q�[����.cache(cps2conv.bat�̏ꍇ)�Ƃ����t�@�C�����쐬
  ����܂��̂ŁA
  /PSP/GAMES/cps2psp/cache �f�B���N�g���ɃR�s�[���Ă��������B
  ����ROM�t�@�C����/PSP/GAMES/cps2psp/roms�ɃR�s�[���Ă��������B

4.�Q�[�������s����΁A�������삵�܂��B

---------------------------------------------------------------------------
romcnv_cps2.exe�̃R�}���h���C�������ɂ���

�E�����̏��Ԃ͖₢�܂���B
�E�p�X��n���ꍇ�́A�t���p�X�Ŏw�肵�Ă��������B�_�u���R�[�e�[�V������
  �O������񂾂ق����ǂ��ł��B

-all: �N����Ƀ_�C�A���O�Ŏw�肵���t�H���_�ɂ���t�@�C���ŁA�Ή�����Q�[��
      ��S�ĕϊ����܂��B�w�肳�ꂽ�p�X�͖������܂��B

-zip: ZIP���k�L���b�V�����쐬���܂��B

-batch: �o�b�`�����p�̃I�v�V�����ł��B�ϊ���Ɉꎞ��~���Ȃ��Ȃ�܂��B


�L�q��) avsp��qndream��ZIP���k���Amvsc��vsav�͖����k�ŕϊ��B
        vsav���I���܂ňꎞ��~���Ȃ��B

romcnv_cps2.exe "D:\roms\avsp.zip" -zip -batch
romcnv_cps2.exe "D:\roms\qndream.zip" -zip -batch
romcnv_cps2.exe "D:\roms\mvsc.zip" -batch
romcnv_cps2.exe "D:\roms\vsav.zip"

---------------------------------------------------------------------------

Linux���Ŏg�p����ꍇ
---------------------

ver.2.0.6���Linux���ł��ꉞ�ϊ��ł���悤�ɂ��܂����B
���Ɉˑ����邽�߁A�o�C�i���ł̒񋟂͂���܂���B

�\�[�X�R�[�h��K���ȃf�B���N�g���ɓW�J���A

make -f makefile.cps2 UNIX=1

�Ƃ��邱�ƂŁAUnix�nOS�p�̃o�C�i�����쐬���܂��B

�g�p���@�͊�{�I��Windows�łƓ����ł����A�_�C�A���O�ł̎w��͏o���Ȃ��̂ŁA
�R�}���h���C���ň������w�肷��K�v������܂��B

�ȉ���Linux��bash��Ŏg�p�����������Ă����܂��B

��) /home/username/romcnv/roms����ssf2.zip��ϊ�����ꍇ�B

./romcnv_cps2 /home/username/romcnv/roms/ssf2.zip

��) /home/username/romcnv/roms����ssf2.zip��zip���k�ϊ�����ꍇ�B

./romcnv_cps2 /home/username/romcnv/roms/ssf2.zip -zip

��) /home/username/romcnv/roms���̑S�Ă�rom��ϊ�����ꍇ�B

./romcnv_cps2 /home/username/romcnv/roms -all

��) /home/username/romcnv/roms���̑S�Ă�rom��zip���k�ϊ�����ꍇ�B

./romcnv_cps2 /home/username/romcnv/roms -all -zip


rom�̃f�B���N�g����romcnv_mvs�{�̂�艺�̊K�w�ɂ���ꍇ�͈ȉ��̂悤�ɂ���
�w�肷�邱�Ƃ��\�ł��B

romcnv_cps2�� /home/username/romcnv-2.0.6�ɂ���Ƃ���B

��) /home/username/romcnv-2.0.6/roms����ssf2.zip��ϊ�����ꍇ�B

./romcnv_cps2 ./roms/ssf2.zip

��) /home/username/romcnv-2.0.6/roms���̑S�Ă�rom��ϊ�����ꍇ�B

./romcnv_cps2 ./roms -all


---------------------------------------------------------------------------
�X�V����

- ver.2.2.0 -
 �Erominfo.cps2�̃t�H�[�}�b�g��ύX�B
 �E�L���b�V���t�@�C�����̂ɂ͕ύX�͂���܂��񂪁A�G�~�����[�^�̃o�[�W����
   �X�V�ɔ����A�L���b�V���t�@�C���̃o�[�W������V22�ɍX�V����Ă��邽�߁A
   �ēx�ϊ����K�v�ł��B

- ver.2.0.6 (3) -
 �E�������ɑS�ēǂݍ��߂�Q�[���͏��zip���k���s���悤�ύX�B
 �Erominfo.cps2�������C���B

- ver.2.0.6 (2) -
 �E�h���b�O&�h���b�v�ŕϊ��ł��Ȃ��Ȃ��Ă����s��̏C���B

- ver.2.0.6 -
 �E�t�H�[�������œ��삵�Ȃ��Ƃ����񍐂��ڂɕt�����̂ŁA��蒼���Ă݂܂����B
 �E���ł�MAME 0.112u2�Œǉ����ꂽ�Q�[���ɑΉ��B
 �ELinux����UNIX�nOS�̃R���\�[���œ��삷��o�C�i�����R���p�C���ł���悤��
   ���܂����B

- ver.2.0 -
 �E�o�[�W�����\�L��{�̂ɍ��킹�܂����B
 �Ezip���k�͔񐄏��Ȃ̂Ő����������B
 �Ezip���k�L���b�V���ɂ��o�[�W�������𖄂ߍ��ނ悤�ɕύX�B
 �Empang��mpangj�𕪗��B
   �����킸���Ƀf�[�^���قȂ�܂����A�����炭���{��ł�BAD DUMP��ROM���Ă�����
   �~�X�������̂ǂ��炩���Ǝv���܂��B(���{��ł͈ꕔ�ɃX�v���C�g�̌�������)
