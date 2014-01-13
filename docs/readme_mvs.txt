----------------------------------------------------------------------

                      NEOGEO Emulator for PSP 1.63

                            NJ (http://neocdz.hp.infoseek.co.jp/psp/)
----------------------------------------------------------------------

<�T�v>

PSP�p��NEOGEO(MVS/AES)�G�~�����[�^�ł��B


----------------------------------------------------------------------
�f�B���N�g���ݒ�

�f�B���N�g���͑S�ď���N�����Ɏ����I�ɍ쐬����܂��B

 /PSP/GAME/
      |
      +- MVSPSP/  (root directory)
         |  |
         |  +- EBOOT.PBP    NEOGEO Emulator binary
         |  +- mvspsp.ini   software config file (create by emulator)
         |
         +- cache/    (directory for sprite cache file)
         |  |
         |  +- mslug_cache/   (example: Metal Slug)
         |
         +- config/   (directory for key config file)
         |
         +- memcard/  (directory for memorycaard)
         |
         +- nvram/    (directory for SRAM)
         |
         +- snap/     (directory for screen shot)
         |
         +- state/    (directory for save state)
         |
         +- roms/ (put BIOS and rom files here. (zip compressed)
         |    |
         |    +- neogeo.zip   (NEOGEO BIOS)
         |    +- samsho.zip   (example: Samrai Spirits)
         |    +- ...


�E�S�Ă�ROM�C���[�W�t�@�C����zip�t�@�C���Ɉ��k����K�v������܂��B
  �t�H���_�ɓW�J�����t�@�C���������Ȃ����Ƃ������΁A��{�I��MAME��
  �S�������ł��B�܂��AMAME���Ή����Ă��Ȃ�ROM�Z�b�g�ɂ͑Ή����܂���B

�EBIOS��neogeo.zip�Ƃ����t�@�C���ɂ܂Ƃ߁Aroms�t�H���_�ɒu���Ă��������B

�E�e�Q�[����zip�t�@�C������"������MAME 0.106��ROM�Z�b�g���Ɉ�v������"
  �K�v������܂��B

�E�e�Q�[����ROM�t�@�C�����͂ǂ�Ȗ��O�ł��\���܂��񂪁A"CRC��MAME 0.106
  ��ROM�Z�b�g��CRC�ƈ�v"���Ă���K�v������܂��B

�Euni-bios����hack BIOS�ɂ��Ή����Ă��܂����A��{�I�ɂ͂����̎g�p��
  �������܂���B�ꕔ�Q�[���͓��삵�Ȃ��\��������܂��B


----------------------------------------------------------------------
�L���b�V���t�@�C���̍쐬

  ROM�ǂݍ��ݎ���"������������Ȃ�"�Ƃ����G���[���\�������ꍇ�́A
  �O���t�B�b�N�f�[�^�̃L���b�V�����쐬����K�v������܂��B
  �t����romcnv_mvs.exe�ō쐬���Ă��������B�g������romcnv_mvs.exe��
  readme_mvs.txt���Q�Ƃ��Ă��������B
  �Ȃ��A�L���b�V���t�@�C�����g�p����ꍇ�́ACPS2PSP�ƈقȂ�S�ẴQ�[��
  �Ōʂɍ쐬����K�v������܂��B

  ��1.31���L���b�V���t�@�C���̃t�H�[�}�b�g���ύX����܂����B
    ����ȑO��ZIP���k�����L���b�V���͎g�p�ł��܂���̂ŁA�S�č폜��
    �V���ɍč\�z���Ă��������B

----------------------------------------------------------------------
������@

��BIOS�̐ݒ��ʂ̓t�@�C���u���E�U���s����"L�g���K"���������Ƃŕ\��
  ����܂��B"�K�������Őݒ肵�Ă���"�Q�[�����N�����Ă��������B
  ERROR: CRC32 not correct. "Europe MVS (Ver. 2)" �̃G���[���b�Z�[�W��
  �\�������ꍇ�́A�������ݒ肳��Ă��܂���B

�E�Q�[�����s���̉�ʂƃ��C�����j���[�������A�S�Ẵ��j���[��"R�g���K"
  ���������Ƃő���w���v���\�������悤�ɂȂ��Ă��܂��B
  �킩��Ȃ���΂Ƃɂ���"R�g���K"�������Ă��������B
  ����΂킩��Ǝv���̂ŁA�ڍׂ͊������܂��B

�E�Q�[���̐ݒ蓙��ύX���郁�j���[�́A�Q�[�����s����"START + SELECT"������
  ���Ƃŕ\������܂��B

�E�Q�[�����̃{�^������
  �{�^���̊��蓖�Ă͕ύX�\�ł��B�ȉ��Ƀf�t�H���g�̐ݒ�������Ă����܂��B
  �{�^���̔z�u��NEOGEO�p�b�h�Ɠ����z�u�ł��B

    Up       - Up or Analog Up
    Down     - Down or Analog Down
    Left     - Left or Analog Left
    Right    - Right or Analog Right
    Start    - Start
    Coin     - Select
    Button A - Cross
    Button B - Circle
    Button C - Square
    Button D - Triangle

  ���ꑀ��
    START + SELECT: ���j���[���J��
    L + R + SELECT: �T�[�r�X�X�C�b�` (����̃{�^���Ɋ��蓖�Ă��\)

----------------------------------------------------------------------
BIOS��Region/Machine Mode�̕ύX�ɂ���

�E�Q�[���ݒ胁�j���[���ŕύX�ł���悤�ɂ��Ă��܂����A���S�ɓ��삷��
  �킯�ł͂���܂���B����̃Q�[���̏ꍇ�A���̐ݒ��ύX�����
  �v���e�N�g�Ɉ��������蓮�삵�Ȃ����̂�����܂��B
  �܂��AAES��BIOS��MVS�̃Q�[���𓮍삳���悤�Ƃ����ꍇ���A���l��
  �v���e�N�g�Ɉ����������ē��삵�Ȃ��ꍇ������܂��B

�E�m���ɕύX�������̂ł���΁Auni-bios v1.0/1.1/1.2/1.3/2.0���g�p����
  ���������B

----------------------------------------------------------------------
���{�ꃊ�\�[�X�t�@�C���ɂ���

�E������"resouce_jp.zip"���𓀂��Ăł���t�@�C��/�t�H���_�́A�ꕔ���
  �œ��{��\�����s���ꍇ�Ɏg�p���܂��B
  ���{��ŕ\�����s�������ꍇ�̂�mvspsp�t�H���_�ɃR�s�[���Ă��������B


----------------------------------------------------------------------
���̑�

�E�������J�[�h�̃t�@�C���̓Q�[�����Ƃɍ쐬����܂��B
  �܂��A�������J�[�h�͏�ɔF��������ԂɂȂ��Ă��܂��B

�E�ȉ��̃Q�[����MAME�ł͓��삵�܂����A���̃G�~�����[�^�ł͖��Ή��ł��B
  ������Ή��\��͂���܂���B

  svcpcb    SvC Chaos - SNK vs Capcom (JAMMA PCB)
  kof97pls  The King of Fighters '97 Plus (bootleg)
  zintrckb  Zintrick (hack / bootleg)
  mslug3b6  Metal Slug 6 (Metal Slug 3 bootleg)
  cthd2003  Crouching Tiger Hidden Dragon 2003 (The King of Fighters 2001 bootleg)
  ct2k3sp   Crouching Tiger Hidden Dragon 2003 Super Plus (The King of Fighters 2001 bootleg)
  kf2k2pls  The King of Fighters 2002 Plus (set 1, bootleg)
  kf2k2pla  The King of Fighters 2002 Plus (set 2, bootleg)
  kf2k2mp   The King of Fighters 2002 Magic Plus (bootleg)
  kf2k2mp2  The King of Fighters 2002 Magic Plus II (bootleg)
  kof10th   The King Of Fighters 10th Anniversary (The King of Fighters 2002 bootleg)
  kf2k5uni  The King of Fighters 10th Anniversary 2005 Unique (The King of Fighters 2002 bootleg)
  kf10thep  The King of Fighters 10th Anniversary Extra Plus (The King of Fighters 2002 bootleg)
  kof2k4se  The King of Fighters Special Edition 2004 (The King of Fighters 2002 bootleg)
  ms5plus   Metal Slug 5 Plus (bootleg)
  kf2k3bl   The King of Fighters 2003 (bootleg, set 1)
  kf2k3bla  The King of Fighters 2003 (bootleg, set 2)
  kf2k3pl   The King of Fighters 2004 Plus / Hero (The King of Fighters 2003 bootleg)
  kf2k3upl  The King of Fighters 2004 Ultra Plus (The King of Fighters 2003 bootleg)
  svcboot   SvC Chaos - SNK vs Capcom (MVS) (bootleg)
  svcplus   SvC Chaos - SNK vs Capcom Plus (set 1, bootleg)
  svcplusa  SvC Chaos - SNK vs Capcom Plus (set 2, bootleg)
  svcsplus  SvC Chaos - SNK vs Capcom Super Plus (bootleg)
  samsho5b  Samurai Shodown V / Samurai Spirits Zero (bootleg)
  lans2004  Lansquenet 2004 (Shock Troopers - 2nd Squad bootleg)
  ms4plus   Metal Slug 4 Plus (bootleg)


----------------------------------------------------------------------
�ύX�_

1.63.1 - ���܂�

�Emslug4�̃t���[�Y�̏����̕񍐂��������̂ŁA�ꉞ�m�F�B
  �����Ȃ�����ł������A�ȒP�ȃo�O�������̂ŏC�����Ă����܂����B
  �����񍐂������Ă������Ȃ��ł���B

----------------------------------------------------------------------
1.63 (���S�ɏI��)
�EUNIVERSE BIOS�y��DEBUG BIOS�̑Ή����~�߂܂����B
  �R�����g�A�E�g���������ł����AMVSPSP�ł͐��������삵�Ȃ��ꍇ��
  ����܂��̂ŁA�g�p���Ȃ��ł��������B

���ʓ|�Ȃ̂ŁAMVSPSP�̓h�L�������g���܂߂āA��؃����e�i���X��
  �s���܂���B�o�O�񍐂�����Ă��C���͍s���܂���̂ŁA���������������B
  �l�I�W�I�֌W�͂��������Ԉ����Ă����̂ŁA�����O����(��)�B

----------------------------------------------------------------------
1.61�`1.62
�ECPS2PSP�̕ύX�ɔ����A�o�[�W�����̂ݕύX�B
  1.60�Ɠ����ł��B

----------------------------------------------------------------------
1.60
�E���t���b�V�����[�g��60Hz������@�Ɠ���59.1856Hz�ɕύX�B
  �ȑO���̓T�E���h�Ɖ�ʂ̓���������悤�ɂȂ�����������܂���B
  ���̉e����FPS�̕\����59������ɂȂ�܂��B�܂��A����I��57fps���炢��
  ���������肵�܂����A�v�Z��̌덷�Ȃ̂ŋC�ɂ��Ȃ��悤�ɂ��Ă��������B
�E��L�ύX�ɔ����Astate data�̃o�[�W�������X�V�B�ȑO�Ƃ͌݊�����
  ����܂���B

----------------------------------------------------------------------
1.51
�Emslug4�������삷��悤�C���B

1.51.1 - �o�C�i���z�z�ŏI��
�Ememcard�t�H���_�������I�ɍ쐬����Ȃ������̂ŏC���B
�EDEBUG BIOS�g�p���ɉ�ʂ������Ă����o�O���C���B

1.51.2
�Estate load����FIX bank���Đݒ肵�Y��Ă����̂ŏC���B

----------------------------------------------------------------------
1.50
�E�ŏI�I�ɓ���𒲐��A�኱�œK���B
  MVSPSP�͂������܂肢����C�͂Ȃ��̂ŁA�o�O����������e���ŏC�����B

----------------------------------------------------------------------
1.42
�E�T�E���h���ꕔ����ɍĐ�����Ȃ��Ȃ��Ă����̂ŏC���B
�Estate load��PCM�L���b�V���ɑΉ�����̂�Y��Ă����̂ŏC���B

���I�����ɗ�����Ƃ̕񍐂�����܂����A�����ł͊m�F�ł����B
  (FW1.50��FW2.60 + eLoader 0.995�œ���m�F���Ă��܂�)

----------------------------------------------------------------------
1.41
�E�X�v���C�g�`��̃o�O���C���B
�E���J����o�C�i������T�E���h�e�X�g���폜�B�K�v�ȏꍇ�͊e����
  �R���p�C�����Ă��������B
�E���{��t�H���g�̓ǂݍ��݂Ń�������j�󂵂Ă����\��������̂�
  ����������ύX�B

----------------------------------------------------------------------
1.40
�E�^�C�}�����������ύX�������Ƃ������ŁA�������̃Q�[�����N���ł��Ȃ�
  �Ȃ��Ă����s����C���B
�Estate data�͈ȑO�̃o�[�W�����ƌ݊����͂���܂���B

----------------------------------------------------------------------
1.31�`1.33
�EPCM�L���b�V���ɑΉ��B�S�ẴQ�[���ŃT�E���h���T�|�[�g���܂����B
�E�L���b�V���t�@�C����ύX�B���k�L���b�V���t�@�C���͑ΏۊO�ƂȂ�܂��B
�E�N���ł��Ȃ��Ȃ��Ă����Q�[�������������������ߏC���B
�E���������o�O���C���B

----------------------------------------------------------------------
1.30
�E�����k�̃L���b�V���t�@�C���ɑΉ��B

----------------------------------------------------------------------
1.10�`1.21

�ECPS2PSP�̃\�[�X�ύX�ɔ����X�V�̂݁B

----------------------------------------------------------------------
ver.1.0

�ECPS1PSP/CPS2PSP�ƃ\�[�X�𓝍��B
�Erominfo�̏�����ύX�B
�E0.2.2�̃\�[�X���s���s���Ȃ̂ŁA0.1.3������̃\�[�X�����蒼���B
  ���x�͕ς���Ă��Ȃ��Ǝv���܂��B
