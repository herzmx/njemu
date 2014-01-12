/******************************************************************************

	me.c

	PSP MediaEngine制御

******************************************************************************/

#include "psp.h"


/******************************************************************************
	ローカル構造体
******************************************************************************/

struct me_core_struct
{
	int start;
	int end;
	void (*func)(int p);
	int param;
};

static struct me_core_struct ALIGN_DATA ME_CORE;
static volatile struct me_core_struct *me_core = NULL;


/******************************************************************************
	プロトタイプ
******************************************************************************/

void me_stub(void);
void me_stub_end(void);


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	MediaEngineメイン関数
--------------------------------------------------------*/

static void me_main(int p)
{
	volatile struct me_core_struct *data = (volatile struct me_core_struct *)p;

	while (1)
	{
		while (data->start == 0);
		data->start = 0;
		data->func(data->param);
		data->end = 1;
	}
}


/*--------------------------------------------------------
	MediaEngineのキャッシュをクリア
--------------------------------------------------------*/

static void me_sceKernelDcacheWritebackInvalidateAll(void)
{
	__asm__ __volatile__(
		"cache 20, 0($0)\n"
		"cache 20, 0($0)\n"
		"cache 20, 64($0)\n"
		"cache 20, 64($0)\n"
		"cache 20, 128($0)\n"
		"cache 20, 128($0)\n"
		"cache 20, 192($0)\n"
		"cache 20, 192($0)\n"
		"cache 20, 256($0)\n"
		"cache 20, 256($0)\n"
		"cache 20, 320($0)\n"
		"cache 20, 320($0)\n"
		"cache 20, 384($0)\n"
		"cache 20, 384($0)\n"
		"cache 20, 448($0)\n"
		"cache 20, 448($0)\n"
		"cache 20, 512($0)\n"
		"cache 20, 512($0)\n"
		"cache 20, 576($0)\n"
		"cache 20, 576($0)\n"
		"cache 20, 640($0)\n"
		"cache 20, 640($0)\n"
		"cache 20, 704($0)\n"
		"cache 20, 704($0)\n"
		"cache 20, 768($0)\n"
		"cache 20, 768($0)\n"
		"cache 20, 832($0)\n"
		"cache 20, 832($0)\n"
		"cache 20, 896($0)\n"
		"cache 20, 896($0)\n"
		"cache 20, 960($0)\n"
		"cache 20, 960($0)\n"
		"cache 20, 1024($0)\n"
		"cache 20, 1024($0)\n"
		"cache 20, 1088($0)\n"
		"cache 20, 1088($0)\n"
		"cache 20, 1152($0)\n"
		"cache 20, 1152($0)\n"
		"cache 20, 1216($0)\n"
		"cache 20, 1216($0)\n"
		"cache 20, 1280($0)\n"
		"cache 20, 1280($0)\n"
		"cache 20, 1344($0)\n"
		"cache 20, 1344($0)\n"
		"cache 20, 1408($0)\n"
		"cache 20, 1408($0)\n"
		"cache 20, 1472($0)\n"
		"cache 20, 1472($0)\n"
		"cache 20, 1536($0)\n"
		"cache 20, 1536($0)\n"
		"cache 20, 1600($0)\n"
		"cache 20, 1600($0)\n"
		"cache 20, 1664($0)\n"
		"cache 20, 1664($0)\n"
		"cache 20, 1728($0)\n"
		"cache 20, 1728($0)\n"
		"cache 20, 1792($0)\n"
		"cache 20, 1792($0)\n"
		"cache 20, 1856($0)\n"
		"cache 20, 1856($0)\n"
		"cache 20, 1920($0)\n"
		"cache 20, 1920($0)\n"
		"cache 20, 1984($0)\n"
		"cache 20, 1984($0)\n"
		"cache 20, 2048($0)\n"
		"cache 20, 2048($0)\n"
		"cache 20, 2112($0)\n"
		"cache 20, 2112($0)\n"
		"cache 20, 2176($0)\n"
		"cache 20, 2176($0)\n"
		"cache 20, 2240($0)\n"
		"cache 20, 2240($0)\n"
		"cache 20, 2304($0)\n"
		"cache 20, 2304($0)\n"
		"cache 20, 2368($0)\n"
		"cache 20, 2368($0)\n"
		"cache 20, 2432($0)\n"
		"cache 20, 2432($0)\n"
		"cache 20, 2496($0)\n"
		"cache 20, 2496($0)\n"
		"cache 20, 2560($0)\n"
		"cache 20, 2560($0)\n"
		"cache 20, 2624($0)\n"
		"cache 20, 2624($0)\n"
		"cache 20, 2688($0)\n"
		"cache 20, 2688($0)\n"
		"cache 20, 2752($0)\n"
		"cache 20, 2752($0)\n"
		"cache 20, 2816($0)\n"
		"cache 20, 2816($0)\n"
		"cache 20, 2880($0)\n"
		"cache 20, 2880($0)\n"
		"cache 20, 2944($0)\n"
		"cache 20, 2944($0)\n"
		"cache 20, 3008($0)\n"
		"cache 20, 3008($0)\n"
		"cache 20, 3072($0)\n"
		"cache 20, 3072($0)\n"
		"cache 20, 3136($0)\n"
		"cache 20, 3136($0)\n"
		"cache 20, 3200($0)\n"
		"cache 20, 3200($0)\n"
		"cache 20, 3264($0)\n"
		"cache 20, 3264($0)\n"
		"cache 20, 3328($0)\n"
		"cache 20, 3328($0)\n"
		"cache 20, 3392($0)\n"
		"cache 20, 3392($0)\n"
		"cache 20, 3456($0)\n"
		"cache 20, 3456($0)\n"
		"cache 20, 3520($0)\n"
		"cache 20, 3520($0)\n"
		"cache 20, 3584($0)\n"
		"cache 20, 3584($0)\n"
		"cache 20, 3648($0)\n"
		"cache 20, 3648($0)\n"
		"cache 20, 3712($0)\n"
		"cache 20, 3712($0)\n"
		"cache 20, 3776($0)\n"
		"cache 20, 3776($0)\n"
		"cache 20, 3840($0)\n"
		"cache 20, 3840($0)\n"
		"cache 20, 3904($0)\n"
		"cache 20, 3904($0)\n"
		"cache 20, 3968($0)\n"
		"cache 20, 3968($0)\n"
		"cache 20, 4032($0)\n"
		"cache 20, 4032($0)\n"
		"cache 20, 4096($0)\n"
		"cache 20, 4096($0)\n"
		"cache 20, 4160($0)\n"
		"cache 20, 4160($0)\n"
		"cache 20, 4224($0)\n"
		"cache 20, 4224($0)\n"
		"cache 20, 4288($0)\n"
		"cache 20, 4288($0)\n"
		"cache 20, 4352($0)\n"
		"cache 20, 4352($0)\n"
		"cache 20, 4416($0)\n"
		"cache 20, 4416($0)\n"
		"cache 20, 4480($0)\n"
		"cache 20, 4480($0)\n"
		"cache 20, 4544($0)\n"
		"cache 20, 4544($0)\n"
		"cache 20, 4608($0)\n"
		"cache 20, 4608($0)\n"
		"cache 20, 4672($0)\n"
		"cache 20, 4672($0)\n"
		"cache 20, 4736($0)\n"
		"cache 20, 4736($0)\n"
		"cache 20, 4800($0)\n"
		"cache 20, 4800($0)\n"
		"cache 20, 4864($0)\n"
		"cache 20, 4864($0)\n"
		"cache 20, 4928($0)\n"
		"cache 20, 4928($0)\n"
		"cache 20, 4992($0)\n"
		"cache 20, 4992($0)\n"
		"cache 20, 5056($0)\n"
		"cache 20, 5056($0)\n"
		"cache 20, 5120($0)\n"
		"cache 20, 5120($0)\n"
		"cache 20, 5184($0)\n"
		"cache 20, 5184($0)\n"
		"cache 20, 5248($0)\n"
		"cache 20, 5248($0)\n"
		"cache 20, 5312($0)\n"
		"cache 20, 5312($0)\n"
		"cache 20, 5376($0)\n"
		"cache 20, 5376($0)\n"
		"cache 20, 5440($0)\n"
		"cache 20, 5440($0)\n"
		"cache 20, 5504($0)\n"
		"cache 20, 5504($0)\n"
		"cache 20, 5568($0)\n"
		"cache 20, 5568($0)\n"
		"cache 20, 5632($0)\n"
		"cache 20, 5632($0)\n"
		"cache 20, 5696($0)\n"
		"cache 20, 5696($0)\n"
		"cache 20, 5760($0)\n"
		"cache 20, 5760($0)\n"
		"cache 20, 5824($0)\n"
		"cache 20, 5824($0)\n"
		"cache 20, 5888($0)\n"
		"cache 20, 5888($0)\n"
		"cache 20, 5952($0)\n"
		"cache 20, 5952($0)\n"
		"cache 20, 6016($0)\n"
		"cache 20, 6016($0)\n"
		"cache 20, 6080($0)\n"
		"cache 20, 6080($0)\n"
		"cache 20, 6144($0)\n"
		"cache 20, 6144($0)\n"
		"cache 20, 6208($0)\n"
		"cache 20, 6208($0)\n"
		"cache 20, 6272($0)\n"
		"cache 20, 6272($0)\n"
		"cache 20, 6336($0)\n"
		"cache 20, 6336($0)\n"
		"cache 20, 6400($0)\n"
		"cache 20, 6400($0)\n"
		"cache 20, 6464($0)\n"
		"cache 20, 6464($0)\n"
		"cache 20, 6528($0)\n"
		"cache 20, 6528($0)\n"
		"cache 20, 6592($0)\n"
		"cache 20, 6592($0)\n"
		"cache 20, 6656($0)\n"
		"cache 20, 6656($0)\n"
		"cache 20, 6720($0)\n"
		"cache 20, 6720($0)\n"
		"cache 20, 6784($0)\n"
		"cache 20, 6784($0)\n"
		"cache 20, 6848($0)\n"
		"cache 20, 6848($0)\n"
		"cache 20, 6912($0)\n"
		"cache 20, 6912($0)\n"
		"cache 20, 6976($0)\n"
		"cache 20, 6976($0)\n"
		"cache 20, 7040($0)\n"
		"cache 20, 7040($0)\n"
		"cache 20, 7104($0)\n"
		"cache 20, 7104($0)\n"
		"cache 20, 7168($0)\n"
		"cache 20, 7168($0)\n"
		"cache 20, 7232($0)\n"
		"cache 20, 7232($0)\n"
		"cache 20, 7296($0)\n"
		"cache 20, 7296($0)\n"
		"cache 20, 7360($0)\n"
		"cache 20, 7360($0)\n"
		"cache 20, 7424($0)\n"
		"cache 20, 7424($0)\n"
		"cache 20, 7488($0)\n"
		"cache 20, 7488($0)\n"
		"cache 20, 7552($0)\n"
		"cache 20, 7552($0)\n"
		"cache 20, 7616($0)\n"
		"cache 20, 7616($0)\n"
		"cache 20, 7680($0)\n"
		"cache 20, 7680($0)\n"
		"cache 20, 7744($0)\n"
		"cache 20, 7744($0)\n"
		"cache 20, 7808($0)\n"
		"cache 20, 7808($0)\n"
		"cache 20, 7872($0)\n"
		"cache 20, 7872($0)\n"
		"cache 20, 7936($0)\n"
		"cache 20, 7936($0)\n"
		"cache 20, 8000($0)\n"
		"cache 20, 8000($0)\n"
		"cache 20, 8064($0)\n"
		"cache 20, 8064($0)\n"
		"cache 20, 8128($0)\n"
		"cache 20, 8128($0)\n"
	);
}


/******************************************************************************
	グローバル関数
******************************************************************************/

/*--------------------------------------------------------
	MediaEngine初期化
--------------------------------------------------------*/

void me_init(void)
{
	me_core = (volatile struct me_core_struct *)PSP_UNCACHE_PTR(&ME_CORE);

	sceKernelDcacheWritebackInvalidateAll();
	me_core->start = 0;
	me_core->end   = 0;
	me_core->func  = 0;
	me_core->param = 0;

	me_sceKernelDcacheWritebackInvalidateAll();

	memcpy((void *)0xbfc00040, me_stub, (int)(me_stub_end - me_stub));

	_sw((u32)me_main, 0xbfc00600);
	_sw((u32)me_core, 0xbfc00604);

	sceKernelDcacheWritebackAll();

	sceSysregMeResetEnable();
	sceSysregMeBusClockEnable();
	sceSysregMeResetDisable();
}


/*--------------------------------------------------------
	MediaEngine停止
--------------------------------------------------------*/

void me_exit(void)
{
	me_sceKernelDcacheWritebackInvalidateAll();
	sceSysregMeBusClockDisable();
	me_core = NULL;
}


/*--------------------------------------------------------
	MediaEngine処理開始
--------------------------------------------------------*/

void me_start(void (*func)(int p), int param)
{
	me_sceKernelDcacheWritebackInvalidateAll();

	if (me_core)
	{
		me_core->end   = 0;
		me_core->func  = func;
		me_core->param = param;
		me_core->start = 1;
	}
}


/*--------------------------------------------------------
	MediaEngine処理停止
--------------------------------------------------------*/

void me_stop(void)
{
	if (me_core)
	{
		while (me_core->end == 0);
		sceKernelDelayThread(1000); //go to sleep for 1ms
	}
}
