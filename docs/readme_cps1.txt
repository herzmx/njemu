----------------------------------------------------------------------

                 CAPCOM CPS1 Emulator for PSP 1.61

                            NJ (http://neocdz.hp.infoseek.co.jp/psp/)
----------------------------------------------------------------------

<�T�v>

  PSP�p��CAPCOM CPS1(Capcom Play System)�G�~�����[�^�ł��B

----------------------------------------------------------------------
�Ή�ROM�Z�b�g�ɂ���

  MAME 0.106�ɏ��������Ă��邽�߁Azip�t�@�C������MAME 0.106��
  �v������ROM�Z�b�g���ƈ�v������K�v������܂��B
  (CPS1PSP�ł͎g�p���Ȃ��t�@�C���������Ă���Q�[��������܂����A
  ��{�I�ɂ͌��݂�MAME0.111��ROM�Z�b�g�����̂܂܎g�p�ł��܂��B)

  �S�Ă�ROM�C���[�W�t�@�C����zip�t�@�C���Ɉ��k����K�v������܂��B
  �t�H���_�ɓW�J�����t�@�C���������Ȃ����Ƃ������΁A��{�I��MAME��
  �S�������ł��B�܂��AMAME���Ή����Ă��Ȃ�ROM�Z�b�g�ɂ͑Ή����܂���B

  �t�@�C���u���E�U��Ŕ����\������Ă���Q�[���͑S�ē��삵�܂��B
  ���삵�Ȃ��ꍇ��ROM�Z�b�g���v��������̂ƈقȂ��Ă���Ƃ������Ƃł��B
  ClrMame Pro��RomCenter���̃c�[�����g���āAMAME 0.106��ROM�Z�b�g��
  ��v�����Ă��������B

  �ǂ����Ă����̃G�~�����[�^��Â�MAME��ROM�Z�b�g�𓮍삳��������
  �����̂ł���΁Arominfo.dat��notepad���ŏ���������΂����̂ł����A
  �����߂͂��܂���Brominfo.dat�̓��e�����ĈӖ����킩��Ȃ����
  �ύX���Ȃ��ł��������B

----------------------------------------------------------------------
�f�B���N�g���ݒ�

�f�B���N�g���͑S�ď���N�����Ɏ����I�ɍ쐬����܂��B

 /PSP/GAME/
      |
      +- CPS1PSP/  (root directory)
         |  |
         |  +- EBOOT.PBP    CPS1 Emulator binary
         |  +- cps1psp.ini  software config file (create by emulator)
         |
         +- config/   (directory for key config file)
         |
         +- nvram/    (directory for SRAM)
         |
         +- snap/     (directory for screen shot)
         |
         +- state/    (directory for state data)
         |
         +- roms/ (put BIOS and rom files here. (zip compressed)
         |    |
         |    +- 1941.zip   (example: 1941)
         |    +- sf2.zip    (example: Street Fighter II (parent))
         |    +- sf2j.zip   (example: Street Fighter II (clone: Japanese version))
         |    +- ...

----------------------------------------------------------------------
������@

�E�Q�[�����s���̉�ʂƃ��C�����j���[�������A�S�Ẵ��j���[��"R�g���K"
  ���������Ƃő���w���v���\�������悤�ɂȂ��Ă��܂��B
  �킩��Ȃ���΂Ƃɂ���"R�g���K"�������Ă��������B
  ����΂킩��Ǝv���̂ŁA�ڍׂ͊������܂��B

�E�Q�[���̐ݒ蓙��ύX���郁�j���[�́A�Q�[�����s����"START + SELECT"������
  ���Ƃŕ\������܂��B

�E�Q�[�����̃{�^������
  �{�^���̊��蓖�Ă͕ύX�\�ł��B�ȉ��Ƀf�t�H���g�̐ݒ�������Ă����܂��B

  �ȉ��̏ꍇ�ɂ́A�����I�Ƀ{�^���z�u����ʂɍ��킹�Ĕ��]/��]���܂��̂ŁA
  ���Ɉӎ����ĕύX����K�v�͂���܂���B
  �EDIP�X�C�b�`��Cabinet�̍��ڂ�Cooktail�ɐݒ肵���ꍇ��2 Player���쎞
  �EDIP�X�C�b�`��Flip Screen�̍��ڂ�On�ɂ����ꍇ
  �E�c��ʂ̃Q�[����Rotate Screen��Yes�ɂ����ꍇ

  ����
    Up    - Up or Analog Up
    Down  - Down or Analog Down
    Left  - Left or Analog Left
    Right - Right or Analog Right
    Start - Start
    Coin  - Select

  2�{�^���̃Q�[��
    Button 1 - Square
    Button 2 - Triangle

  3�{�^���̃Q�[��
    Button 1 - Square
    Button 2 - Triangle
    Button 3 - Cross

  �N�C�Y�Q�[�� (�����{�^���͎g�p���܂���)
    Button 1 - Square
    Button 2 - Triangle
    Button 3 - Cross
    Button 4 - Circle
    �v���C���[�؂�ւ� - L trigger

  Street Fighter II�n�̃Q�[�� (Street Fighter Zero CPS Changer ver.�܂�)
    Button 1 - Square
    Button 2 - Triangle
    Button 3 - L trigger
    Button 4 - Cross
    Button 5 - Circle
    Button 6 - R trigger

  Forgotton World / Lost World
    (���[�v���o�[��PSP�ł͂ǂ��ɂ��Ȃ�Ȃ��̂ŁAL/R�g���K�ő�p)
    Button 1 - Square
    Dial(rotate left) - L trigger
    Dial(rotate right) - R trigger

  ���ꑀ��
    START + SELECT: ���j���[���J��
    L + R + SELECT: �T�[�r�X�X�C�b�` (����̃{�^���Ɋ��蓖�Ă��\)
    L + R + START:  1P & 2P START (�T�[�r�X���j���[�Ŏg�p)

----------------------------------------------------------------------
"Raster Effects"�ɂ���

�E�Q�[���ݒ胁�j���[�ɁA�hRaster Effects"�̍��ڂ�����܂��B
  ���̍��ڂŁAStreet Fighter II���̃Q�[���Ŏg�p����Ă���Line Scroll
  ���ʂ̗L��/�������w�肵�܂��B
  ���̍��ڂ́A�G�~�����[�V�����̕��ׂ����ɍ���(�X�v���C�g��1���C��
  ���`�悵�Ȃ���΂Ȃ�Ȃ����߁A�`��񐔂��ʏ�̐��{�ɑ�����)���߁A
  �ʏ��"Off"�ɂȂ��Ă��܂��B�w�i�����ꂽ�肷��̂��C�ɂȂ�ꍇ�́A
  ���Ȃ蓮�쑬�x�������܂���"On"�ɕύX���Ă݂Ă��������B

  �Ȃ��A"Off"�̂Ƃ��ɔw�i�������͎̂d�l�ł���A�o�O�ł͂���܂���B
  Raster Effects��"On"�ɂ���ȊO�ɐ���ɕ\��������@�͂���܂���B

  Street Fighter II�͖����ɂ��Ă��܂��Ɖ�ʂ�����Ăނ�����Ԃ�
  �Ȃ�܂����AStreet Fighter Zero�͂��قǕς��Ȃ��̂ŁA�����ɂ���
  �����ǂ��ł��B

----------------------------------------------------------------------
"Auto Frameskip"�ɂ���

  �����ɂ����ꍇ�A���R�Q�[���̑S�̓I�ȑ��x�͗����܂����A���肵��
  �v���C�������ꍇ�͖����ɂ��������ǂ��ł��B

  �� 1.0���珉���l��Off�ɂȂ��Ă��܂��B

----------------------------------------------------------------------
"Video Sync"�ɂ���

  1.0�œ�������P���܂����B
  �L���ɂ���ƃT�E���h���኱�]���ɂȂ�܂����A��ʂ̂�����͉��P
  ����܂��BKing of Dragons����1�t���[�����ƂɃL�����N�^���_�ł���
  �悤�ȃQ�[���͗L���ɂ��������ǂ��Ǝv���܂��B
  �������d���Q�[���ł͊�{�I�ɖ����ɂ��Ă��������B

  Video Sync Yes���̐����̐ݒ�͈ȉ��ł��B

  Raster Effects      Off
  60fps Speed Limit   Off
  Sample Rate         11025Hz (22050Hz�ȏ�ł͑��������Ǝv���܂�)

----------------------------------------------------------------------
bootleg�ł̃Q�[���̈����ɂ���

  beta 3�܂ňꉞ�Ή����Ă��܂������A1.0�őS�č폜���܂����B
  ��{�I�Ɏ������G�~�����[�^�͐��K�̃o�[�W�������Í����Ȃǂɂ��
  ���삵�Ȃ��ꍇ�������āAbootleg�łɂ͑Ή����܂���B
  (���R�͒P����bootleg�ł�����������ł����B)
  ����\�[�X�R�[�h�͌��J���܂��̂ŁA�K�v�ł���Ίe����MAME�Ȃ���Q�l��
  �ǉ����Ă��������B

----------------------------------------------------------------------
�ύX�_

1.61
�ECPS2PSP�̕ύX�ɔ����A�o�[�W�����̂ݕύX�B
  1.60�Ɠ����ł��B

----------------------------------------------------------------------
1.60
�E���C���X�N���[���`�揈���̃R�[�h���ȗ������A�኱�œK�����s���܂����B

----------------------------------------------------------------------
1.21�`1.51
�ECPS2PSP�AMVSPSP�̕ύX�ɔ����A�o�[�W�����̂ݕύX�B
  1.20�Ɠ����ł��B

----------------------------------------------------------------------
1.20
�E�X�v���C�g�Ǘ�������ύX�B
  ���x����ɂ��A�X�g���C�_�[�򗳂Ƃ����������肵�������B
  �T�E���h22KHz�ł�60fps�œ��삷��\�t�g����������������܂���B

----------------------------------------------------------------------
1.11�`1.13
�E�o�O�C���B

----------------------------------------------------------------------
1.10

�E�����I�ȕύX�����B

----------------------------------------------------------------------
1.02
�E�\�[�X��CPS2PSP/MVSPSP�Ɠ����B

----------------------------------------------------------------------
1.01

�EPang!3�̃X�e�[�g�Z�[�u������ɍs���Ȃ������o�O���C��
�E�\�[�X�R�[�h��CPS2PSP/MVSPSP�Ɠ���
�Erominfo.dat���̃��\�[�X�t�@�C���̊g���q��.cps1�ɕύX

----------------------------------------------------------------------
1.0

�EC68K�R�A��CPS2PSP�Ɠ������̂ɕύX
�EZ80�R�A��CZ80�ɕύX
�ECZ80�̃o�O���������C��
�E��{�̏�����CPS2PSP�Ɠ����̂��̂ɍX�V
�Ebootleg�ł̃Q�[����S�č폜
�E�`�揈����S�ď������������ʁA�������x�����サ���悤�ł�
�E���C���[�̃p���b�g�����������C��
�ESave/Load state�@�\��ǉ�
�EVSYNC�L�����̏��������P
�E�f�B�b�v�X�C�b�`�̒l���Ē������A���������삷��Ǝv����l�ɕύX
�E�f�B�b�v�X�C�b�`��Demo Sound��Allow Continue�̏����l��Yes�ɕύX
�Eautofire�@�\��Rotate Screen��Yes�ɂ����ꍇ�ɋ@�\���Ȃ������o�O���C��
�Escreen shot�̉摜�`����PNG�ɕύX
�EKernel Mode�ł�ǉ�
�E���ύX����
