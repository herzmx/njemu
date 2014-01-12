/******************************************************************************

	psp.c

	PSPメイン

******************************************************************************/

#include "psp.h"


#ifdef KERNEL_MODE
PSP_MODULE_INFO(PBPNAME_STR, 0x1000, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(0);
#else
PSP_MODULE_INFO(PBPNAME_STR, 0, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
#endif

/******************************************************************************
	グローバル変数
******************************************************************************/

volatile int Loop;
volatile int Sleep;
char launchDir[MAX_PATH];


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	Exit Callback
--------------------------------------------------------*/

static SceKernelCallbackFunction ExitCallback(int arg1, int arg2, void *arg)
{
	Loop = LOOP_EXIT;
	return 0;
}

/*--------------------------------------------------------
	Power Callback
--------------------------------------------------------*/

static SceKernelCallbackFunction PowerCallback(int unknown, int pwrflags, void *arg)
{
	int cbid;

	if (pwrflags & PSP_POWER_CB_POWER_SWITCH)
	{
		Sleep = 1;
	}
	else if (pwrflags & PSP_POWER_CB_RESUME_COMPLETE)
	{
		Sleep = 0;
	}

	cbid = sceKernelCreateCallback("Power Callback", (void *)PowerCallback, NULL);

	scePowerRegisterCallback(0, cbid);

	return 0;
}

/*--------------------------------------------------------
	コールバックスレッド作成
--------------------------------------------------------*/

static int CallbackThread(SceSize args, void *argp)
{
	int cbid;

	cbid = sceKernelCreateCallback("Exit Callback", (void *)ExitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);

	cbid = sceKernelCreateCallback("Power Callback", (void *)PowerCallback, NULL);
	scePowerRegisterCallback(0, cbid);

	sceKernelSleepThreadCB();

	return 0;
}


/*--------------------------------------------------------
	コールバックスレッド設定
--------------------------------------------------------*/

static int SetupCallbacks(void)
{
	SceUID thread_id = 0;

	thread_id = sceKernelCreateThread("Update Thread", CallbackThread, 0x10, 0xFA0, 0, NULL);
	if (thread_id >= 0)
	{
		sceKernelStartThread(thread_id, 0, 0);
	}

	Loop  = LOOP_EXIT;
	Sleep = 0;

	return thread_id;
}


/*--------------------------------------------------------
	main()
--------------------------------------------------------*/

#ifdef KERNEL_MODE

static SceUID main_thread;

void main_thread_set_priority(int priority)
{
	sceKernelChangeThreadPriority(main_thread, priority);
}

static int user_main(SceSize args, void *argp)
{
	SetupCallbacks();

	set_cpu_clock(PSPCLOCK_222);

	pad_init();
	video_init();
	file_browser();
	video_exit(1);

	return 0;
}

int main(int argc, char *argv[])
{
	char *p;

	memset(launchDir, 0, sizeof(launchDir));
	strncpy(launchDir, argv[0], MAX_PATH - 1);
	if ((p = strrchr(launchDir, '/')) != NULL)
	{
		*(p + 1) = '\0';
	}

#ifdef MEDIA_ENGINE
	me_init();
#endif

	main_thread = sceKernelCreateThread("User Mode Thread",
								user_main,
								0x11,
								256 * 1024,
								PSP_THREAD_ATTR_USER,
								NULL);

	sceKernelStartThread(main_thread, 0, 0);
	sceKernelWaitThreadEnd(main_thread, NULL);

#ifdef MEDIA_ENGINE
	me_exit();
#endif

	sceKernelExitGame();

	return 0;
}

#else

int main(int argc, char *argv[])
{
	char *p;

	SetupCallbacks();

	memset(launchDir, 0, sizeof(launchDir));
	strncpy(launchDir, argv[0], MAX_PATH - 1);
	if ((p = strrchr(launchDir, '/')) != NULL)
	{
		*(p + 1) = '\0';
	}

	set_cpu_clock(PSPCLOCK_222);

	pad_init();
	video_init();
	file_browser();
	video_exit(1);

	sceKernelExitGame();

	return 0;
}

#endif /* KERNEL_MODE */
