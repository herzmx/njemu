/******************************************************************************

	psp.c

	PSPメイン

******************************************************************************/

#include "psp.h"
#include "homehook.h"


#ifdef KERNEL_MODE
PSP_MODULE_INFO(PBPNAME_STR, PSP_MODULE_KERNEL, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(0);
#else
PSP_MODULE_INFO(PBPNAME_STR, PSP_MODULE_USER, VERSION_MAJOR, VERSION_MINOR);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);
#endif


/******************************************************************************
	グローバル変数
******************************************************************************/

volatile int Loop;
volatile int Sleep;
char launchDir[MAX_PATH];
int psp_cpuclock;
int devkit_version;


/******************************************************************************
	グローバル関数
******************************************************************************/

/*------------------------------------------------------
	CPUクロック設定
------------------------------------------------------*/

void set_cpu_clock(int value)
{
	switch (value)
	{
	case PSPCLOCK_266: scePowerSetClockFrequency(266, 266, 133); break;
	case PSPCLOCK_333: scePowerSetClockFrequency(333, 333, 166); break;
	default: scePowerSetClockFrequency(222, 222, 111); break;
	}
}


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	Exit Callback
--------------------------------------------------------*/

#if !defined(KERNEL_MODE) && !defined(PSP_SLIM)
static SceKernelCallbackFunction ExitCallback(int arg1, int arg2, void *arg)
{
	Loop = LOOP_EXIT;
	return 0;
}
#endif


/*--------------------------------------------------------
	Power Callback
--------------------------------------------------------*/

static SceKernelCallbackFunction PowerCallback(int unknown, int pwrflags, void *arg)
{
	int cbid;

	if (pwrflags & PSP_POWER_CB_POWER_SWITCH)
	{
#ifdef PSP_SLIM
#if (EMU_SYSTEM == CPS2) || (EMU_SYSTEM == MVS)
		char path[MAX_PATH];
		SceUID fd;

		sprintf(path, "%sresume.bin", launchDir);

		if ((fd = sceIoOpen(path, PSP_O_WRONLY|PSP_O_CREAT, 0777)) >= 0)
		{
			sceIoWrite(fd, (void *)(PSP2K_MEM_TOP + 0x1c00000), 0x400000);
			sceIoClose(fd);
		}
#endif
#endif
		Sleep = 1;
	}
	else if (pwrflags & PSP_POWER_CB_RESUME_COMPLETE)
	{
#ifdef PSP_SLIM
#if (EMU_SYSTEM == CPS2) || (EMU_SYSTEM == MVS)
		char path[MAX_PATH];
		SceUID fd;

		sprintf(path, "%sresume.bin", launchDir);

		if ((fd = sceIoOpen(path, PSP_O_RDONLY, 0777)) >= 0)
		{
			sceIoRead(fd, (void *)(PSP2K_MEM_TOP + 0x1c00000), 0x400000);
			sceIoClose(fd);
			sceIoRemove(path);
		}
#endif
#endif
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

#if !defined(KERNEL_MODE) && !defined(PSP_SLIM)
	cbid = sceKernelCreateCallback("Exit Callback", (void *)ExitCallback, NULL);
	sceKernelRegisterExitCallback(cbid);
#endif

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

	thread_id = sceKernelCreateThread("Update Thread", CallbackThread, 0x12, 0xFA0, 0, NULL);
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

static volatile int home_active;
static UINT32 home_button;


u32 readHomeButton(void)
{
	return home_button;
}


static int home_button_thread(SceSize args, void *argp)
{
	SceCtrlData paddata;

	home_active = 1;

	while (home_active)
	{
		sceCtrlPeekBufferPositive(&paddata, 1);
		home_button = paddata.Buttons & PSP_CTRL_HOME;
		sceKernelDelayThread(200);
	}

	sceKernelExitThread(0);

	return 0;
}


static int user_main(SceSize args, void *argp)
{
	devkit_version = sceKernelDevkitVersion();

	SetupCallbacks();

	set_cpu_clock(PSPCLOCK_222);

	ui_text_init();
	pad_init();

	video_set_mode(32);
	video_init();

	file_browser();

	video_exit();

	sceKernelExitThread(0);

	return 0;
}

int main(int argc, char *argv[])
{
	SceUID main_thread;
	SceUID home_thread;
	char *p;

	memset(launchDir, 0, sizeof(launchDir));
	strncpy(launchDir, argv[0], MAX_PATH - 1);
	if ((p = strrchr(launchDir, '/')) != NULL)
	{
		*(p + 1) = '\0';
	}

#ifdef ADHOC
	pspSdkLoadAdhocModules();
#endif

	home_thread = sceKernelCreateThread("Home Button Thread",
								home_button_thread,
								0x11,
								0x200,
								0,
								NULL);

	main_thread = sceKernelCreateThread("User Mode Thread",
								user_main,
								0x11,
								256 * 1024,
								PSP_THREAD_ATTR_USER,
								NULL);

	sceKernelStartThread(home_thread, 0, 0);

	sceKernelStartThread(main_thread, 0, 0);
	sceKernelWaitThreadEnd(main_thread, NULL);

	home_active = 0;
	sceKernelWaitThreadEnd(home_thread, NULL);

	sceKernelExitGame();

	return 0;
}

#else

#ifndef PSP_SLIM
u32 readHomeButton(void)
{
	SceCtrlData paddata;

	sceCtrlPeekBufferPositive(&paddata, 1);

	if ((paddata.Buttons & (PSP_CTRL_START | PSP_CTRL_SELECT)) == (PSP_CTRL_START | PSP_CTRL_SELECT))
	{
		return PSP_CTRL_HOME;
	}
	return 0;
}
#endif


int main(int argc, char *argv[])
{
	char *p;
#ifdef PSP_SLIM
	SceUID modID;
#endif

	memset(launchDir, 0, sizeof(launchDir));
	strncpy(launchDir, argv[0], MAX_PATH - 1);
	if ((p = strrchr(launchDir, '/')) != NULL)
	{
		*(p + 1) = '\0';
	}

	devkit_version = sceKernelDevkitVersion();

	SetupCallbacks();

	set_cpu_clock(PSPCLOCK_222);

	// AHMAN
#ifdef ADHOC
	pspSdkLoadAdhocModules();
#endif

	ui_text_init();
	pad_init();

	video_set_mode(32);
	video_init();

#ifdef PSP_SLIM
	if ((modID = pspSdkLoadStartModule("homehook.prx", PSP_MEMORY_PARTITION_KERNEL)) >= 0)
	{
		initHomeButton(devkit_version);

		file_browser();
		video_exit();
	}
	else
	{
		small_font_printf(0, 0, "Error 0x%08X start homehook.prx.", modID);
		video_flip_screen(1);
		sceKernelDelayThread(5*1000*1000);
	}
#else
	file_browser();
	video_exit();
#endif

	sceKernelExitGame();

	return 0;
}

#endif /* KERNEL_MODE */
