/***************************************************************************

	adhoc.c

	PSP�A�h�z�b�N�ʐM����

***************************************************************************/

#include "emumain.h"
#include <pspsdk.h>
#include <psputilsforkernel.h>
#include <pspnet.h>
#include <pspnet_adhoc.h>
#include <pspnet_adhocctl.h>
#include <pspnet_adhocmatching.h>
#include <pspwlan.h>

#define NUM_ENTRIES			32

#define PSP_LISTING			1
#define PSP_SELECTED		2
#define PSP_SELECTING		3
#define PSP_WAIT_EST		4
#define PSP_ESTABLISHED		5

#define ADHOC_TIMEOUT		30*1000000
#define ADHOC_BLOCKSIZE		0x400


/***************************************************************************
	�֐��p�����[�^ (�����̂ŁE�E�E)
***************************************************************************/

#define MATCHING_CREATE_PARAMS	\
	3,							\
	0xa,						\
	0x22b,						\
	0x800,						\
	0x2dc6c0,					\
	0x5b8d80,					\
	3,							\
	0x7a120,					\
	matchingCallback

#define MATCHING_START_PARAMS	\
	matchingId,					\
	0x10,						\
	0x2000,						\
	0x10,						\
	0x2000,						\
	strlen(matchingData) + 1,	\
	(char *)matchingData


/***************************************************************************
	���[�J���ϐ�
***************************************************************************/

static int Server;
static int pdpId;

static char g_mac[16];
static char g_mymac[16];
static int  g_unk1;
static int  g_matchEvent;
static int  g_matchOptLen;
static char g_matchOptData[1000];
static char g_matchingData[32];
static int  matchChanged;
static int  matchingId;

static struct psplist_t
{
	char name[48];
	char mac[6];
} psplist[NUM_ENTRIES];

static int max;
static int pos;


/***************************************************************************
	���[�J���֐�
***************************************************************************/

/*--------------------------------------------------------
	�v���O���X�o�[������
--------------------------------------------------------*/

static void adhoc_init_progress(int total, const char *text)
{
	char buf[32];

	load_background(WP_LOGO);
	video_copy_rect(work_frame, draw_frame, &full_rect, &full_rect);

	small_icon(6, 3, UI_COLOR(UI_PAL_TITLE), ICON_SYSTEM);
	sprintf(buf, "AdHoc - %s", game_name);
	uifont_print(32, 5, UI_COLOR(UI_PAL_TITLE), buf);

	video_copy_rect(draw_frame, work_frame, &full_rect, &full_rect);

	init_progress(total, text);
}


/*--------------------------------------------------------
	���X�g����
--------------------------------------------------------*/

static void ClearPspList(void)
{
	max = 0;
	pos = 0;
	memset(&psplist, 0, sizeof(psplist));
}


/*--------------------------------------------------------
	���X�g�ɒǉ�
--------------------------------------------------------*/

static int AddPsp(char *mac, char *name, int length)
{
	int i;

	if (max == NUM_ENTRIES) return 0;
	if (length == 1) return 0;

	for (i = 0; i < max; i++)
	{
		if (memcmp(psplist[i].mac, mac, 6) == 0)
			return 0;
	}

	memcpy(psplist[max].mac, mac, 6);

	if (length)
	{
		if (length < 47)
			strcpy(psplist[max].name, name);
		else
			strncpy(psplist[max].name, name, 47);
	}
	else
		psplist[max].name[0] = '\0';

	max++;

	return 1;
}


/*--------------------------------------------------------
	���X�g����폜
--------------------------------------------------------*/

static int DelPsp(char *mac)
{
	int i, j;

	for (i = 0; i < max; i++)
	{
		if (memcmp(psplist[i].mac, mac, 6) == 0)
		{
			if (i != max - 1)
			{
				for (j = i + 1; j < max; j++)
				{
					memcpy(psplist[j - 1].mac, psplist[j].mac, 6);
					strcpy(psplist[j - 1].name, psplist[j].name);
				}
			}

			if (pos == i) pos = 0;
			if (pos > i) pos--;
			max--;

			return 0;
		}
	}

	return -1;
}


/*--------------------------------------------------------
	���X�g��\��
--------------------------------------------------------*/

static void DisplayPspList(int top, int rows)
{
	if (max == 0)
	{
		msg_printf(TEXT(WAITING_FOR_ANOTHER_PSP_TO_JOIN));
	}
	else
	{
		int i;
		char temp[20];

		video_copy_rect(show_frame, draw_frame, &full_rect, &full_rect);

		draw_scrollbar(470, 26, 479, 270, rows, max, pos);

		for (i = 0; i < rows; i++)
		{
			if ((top + i) >= max) break;

			sceNetEtherNtostr((UINT8 *)psplist[top + i].mac, temp);

			if ((top + i) == pos)
			{
				uifont_print(24, 40 + (i + 2) * 17, UI_COLOR(UI_PAL_SELECT), temp);
				uifont_print(190, 40 + (i + 2) * 17, UI_COLOR(UI_PAL_SELECT), psplist[top + i].name);
			}
			else
			{
				uifont_print(24, 40 + (i + 2) * 17, UI_COLOR(UI_PAL_NORMAL), temp);
				uifont_print(190, 40 + (i + 2) * 17, UI_COLOR(UI_PAL_NORMAL), psplist[top + i].name);
			}
		}

		video_flip_screen(1);
	}
}


/*--------------------------------------------------------
	�I�𒆂�PSP�̏����擾
--------------------------------------------------------*/

static int GetPspEntry(char *mac, char *name)
{
	if (max == 0) return -1;

	memcpy(mac, psplist[pos].mac, 6);
	strcpy(name, psplist[pos].name);

	return 1;
}


/*--------------------------------------------------------
	Matching callback
--------------------------------------------------------*/

static void matchingCallback(int unk1, int event, char *mac2, int optLen, char *optData)
{
	switch (event)
	{
	case MATCHING_JOINED:
		AddPsp(mac2, optData, optLen);
		break;

	case MATCHING_DISCONNECT:
		DelPsp(mac2);
		break;

	default:
		g_unk1        = unk1;
		g_matchEvent  = event;
		g_matchOptLen = optLen;
		strncpy(g_matchOptData, optData, optLen);
		memcpy(g_mac, mac2, sizeof(char) * 6);
		matchChanged = 1;
		break;
	}
}


/***************************************************************************
	AdHoc�C���^�t�F�[�X�֐�
***************************************************************************/

/*--------------------------------------------------------
	���W���[���̃��[�h
--------------------------------------------------------*/

int pspSdkLoadAdhocModules(void)
{
	int modID;

	modID = pspSdkLoadStartModule("flash0:/kd/ifhandle.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (modID < 0)
		return modID;

	modID = pspSdkLoadStartModule("flash0:/kd/memab.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (modID < 0)
		return modID;

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet_adhoc_auth.prx", PSP_MEMORY_PARTITION_KERNEL);
	if (modID < 0)
		return modID;

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet.prx", PSP_MEMORY_PARTITION_USER);
	if (modID < 0)
		return modID;
	else
		pspSdkFixupImports(modID);

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet_adhoc.prx", PSP_MEMORY_PARTITION_USER);
	if (modID < 0)
		return modID;
	else
		pspSdkFixupImports(modID);

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet_adhocctl.prx", PSP_MEMORY_PARTITION_USER);
	if (modID < 0)
		return modID;
	else
		pspSdkFixupImports(modID);

	modID = pspSdkLoadStartModule("flash0:/kd/pspnet_adhoc_matching.prx", PSP_MEMORY_PARTITION_USER);
	if (modID < 0)
		return modID;
	else
		pspSdkFixupImports(modID);

	sceKernelDcacheWritebackAll();
	sceKernelIcacheInvalidateAll();

	return 0;
}


/*--------------------------------------------------------
	������
--------------------------------------------------------*/

#if JAPANESE_UI
#if (EMU_SYSTEM == CPS1)
#define PRODUCT	"CP1J"
#elif (EMU_SYSTEM == CPS2)
#define PRODUCT	"CP2J"
#elif (EMU_SYSTEM == MVS)
#define PRODUCT	"MVSJ"
#elif (EMU_SYSTEM == NCDZ)
#define PRODUCT	"NCDJ"
#endif
#else
#if (EMU_SYSTEM == CPS1)
#define PRODUCT	"CP1E"
#elif (EMU_SYSTEM == CPS2)
#define PRODUCT	"CP2E"
#elif (EMU_SYSTEM == MVS)
#define PRODUCT	"MVSE"
#elif (EMU_SYSTEM == NCDZ)
#define PRODUCT	"NCDE"
#endif
#endif

int adhocInit(const char *matchingData)
{
	struct productStruct product;
	int error = 0, state = 0;
	char mac[20], buf[256];

	video_set_mode(32);

	Server = 0;

	g_unk1        = 0;
	g_matchEvent  = 0;
	g_matchOptLen = 0;
	matchChanged  = 0;
	memset(g_mac, 0, sizeof(g_mac));
	memset(g_mymac, 0, sizeof(g_mymac));

	sprintf(product.product, PRODUCT "00%d%d%d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);
	product.unknown = 0;

	ClearPspList();

	if (strlen(matchingData) == 0)
		return -1;

	strcpy(g_matchingData, matchingData);

	adhoc_init_progress(10, TEXT(CONNECTING));

	if ((error = sceNetInit(0x20000, 0x20, 0x1000, 0x20, 0x1000)) == 0)
	{
		update_progress();
		if ((error = sceNetAdhocInit()) == 0)
		{
			update_progress();
			if ((error = sceNetAdhocctlInit(0x2000, 0x20, &product)) == 0)
			{
				update_progress();
				if ((error = sceNetAdhocctlConnect((int *)"")) == 0)
				{
					update_progress();
					do
					{
						if ((error = sceNetAdhocctlGetState(&state)) != 0) break;
						sceKernelDelayThread(1000000/60);

					} while (state != 1);

					if (!error)
					{
						update_progress();

						sceWlanGetEtherAddr(mac);
						update_progress();

						if ((pdpId = sceNetAdhocPdpCreate(mac, 0x309, 0x400, 0)) > 0)
						{
							update_progress();
							if ((error = sceNetAdhocMatchingInit(0x20000)) == 0)
							{
								update_progress();
								if ((matchingId = sceNetAdhocMatchingCreate(MATCHING_CREATE_PARAMS)) >= 0)
								{
									update_progress();
									if ((error = sceNetAdhocMatchingStart(MATCHING_START_PARAMS)) == 0)
									{
										update_progress();
										show_progress(TEXT(CONNECTED));
										return 0;
									}
									sceNetAdhocMatchingDelete(matchingId);
								}
								error = 2;
								sceNetAdhocMatchingTerm();
							}
							sceNetAdhocPdpDelete(pdpId, 0);
						}
						error = 1;
					}
					sceNetAdhocctlDisconnect();
				}
				sceNetAdhocctlTerm();
			}
			sceNetAdhocTerm();
		}
		sceNetTerm();
	}

	switch (error)
	{
	case 1:  sprintf(buf, "%s (PDP ID = %08x)", TEXT(FAILED), pdpId); break;
	case 2:  sprintf(buf, "%s (Matching ID = %08x)", TEXT(FAILED), matchingId); break;
	default: sprintf(buf, "%s (Error Code = %08x)", TEXT(FAILED), error); break;
	}

	show_progress(buf);

	pad_wait_clear();
	pad_wait_press(PAD_WAIT_INFINITY);

	return -1;
}


/*--------------------------------------------------------
	�ؒf
--------------------------------------------------------*/

int adhocTerm(void)
{
	adhoc_init_progress(5, TEXT(DISCONNECTING));

	sceNetAdhocctlDisconnect();
	update_progress();

	sceNetAdhocPdpDelete(pdpId, 0);
	update_progress();

	sceNetAdhocctlTerm();
	update_progress();

	sceNetAdhocTerm();
	update_progress();

	sceNetTerm();
	update_progress();

	show_progress(TEXT(DISCONNECTED));

	return 0;
}

/*--------------------------------------------------------
	�ؒf
--------------------------------------------------------*/

static void adhocDisconnect(void)
{
	adhoc_init_progress(8, TEXT(DISCONNECTING));

	sceNetAdhocMatchingStop(matchingId);
	update_progress();

	sceNetAdhocMatchingDelete(matchingId);
	update_progress();

	sceNetAdhocMatchingTerm();
	update_progress();

	sceNetAdhocctlDisconnect();
	update_progress();

	sceNetAdhocPdpDelete(pdpId, 0);
	update_progress();

	sceNetAdhocctlTerm();
	update_progress();

	sceNetAdhocTerm();
	update_progress();

	sceNetTerm();
	update_progress();

	show_progress(TEXT(DISCONNECTED));
}

/*--------------------------------------------------------
	SSID���w�肵�čĐڑ�
--------------------------------------------------------*/

int adhocReconnect(char *ssid)
{
	int error = 0, state = 1;
	char mac[20], buf[256];

	adhoc_init_progress(6, TEXT(DISCONNECTING));

	sceNetAdhocMatchingStop(matchingId);
	update_progress();

	sceNetAdhocMatchingDelete(matchingId);
	update_progress();

	sceNetAdhocMatchingTerm();
	update_progress();

	sceNetAdhocPdpDelete(pdpId, 0);
	update_progress();

	sceNetAdhocctlDisconnect();
	update_progress();

	do
	{
		if ((error = sceNetAdhocctlGetState(&state)) != 0) break;
			sceKernelDelayThread(1000000/60);
	} while (state == 1);

	update_progress();
	show_progress(TEXT(DISCONNECTED));

	adhoc_init_progress(4, TEXT(CONNECTING));

	if ((error = sceNetAdhocctlConnect((int *)ssid)) == 0)
	{
		update_progress();
		do
		{
			if ((error = sceNetAdhocctlGetState(&state)) != 0) break;
			sceKernelDelayThread(1000000/60);
		} while (state != 1);

		if (!error)
		{
			update_progress();

			sceWlanGetEtherAddr(mac);
			memcpy(g_mymac, mac, 6);
			update_progress();

			if ((pdpId = sceNetAdhocPdpCreate(mac, 0x309, 0x800, 0)) > 0)
			{
				update_progress();
				show_progress(TEXT(CONNECTED));
				if (Server)
				{
					// �قړ����ɑ���M����������ƃt���[�Y���邽�߁A
					// �T�[�o���������x�点�ă^�C�~���O�����炷
					sceKernelDelayThread(1*1000000);
				}
				return 0;
			}
			error = 1;
		}
		sceNetAdhocctlDisconnect();

		if (state == 1)
		{
			do
			{
				if ((error = sceNetAdhocctlGetState(&state)) != 0) break;
				sceKernelDelayThread(1000000/60);
			} while (state == 1);
		}
	}

	sceNetAdhocctlTerm();
	sceNetAdhocTerm();
	sceNetTerm();

	switch (error)
	{
	case 1:  sprintf(buf, "%s (PDP ID = %08x)", TEXT(FAILED), pdpId); break;
	default: sprintf(buf, "%s (Error Code = %08x)", TEXT(FAILED), error); break;
	}

	show_progress(buf);

	pad_wait_clear();
	pad_wait_press(PAD_WAIT_INFINITY);

	return -1;
}


/*--------------------------------------------------------
	�ڑ���̑I��
--------------------------------------------------------*/

int adhocSelect(void)
{
	int top = 0;
	int rows = 11;
	int currentState = PSP_LISTING;
	int prev_max = 0;
	int update = 1;
	char mac[16];
	char name[64];
	char temp[64];
	char ssid[10];
	char title[32];

	sprintf(title, "AdHoc - %s", game_name);
	msg_screen_init(WP_LOGO, ICON_SYSTEM, title);

	while (1)
	{
		pad_update();

		msg_set_text_color(0xffff);

		switch (currentState)
		{
		case PSP_LISTING:
			Server = 0;
			if (update)
			{
				msg_screen_init(WP_LOGO, ICON_SYSTEM, title);
				msg_printf(TEXT(SELECT_A_SERVER_TO_CONNECT_TO));
				msg_printf("\n");
				DisplayPspList(top, rows);
				update = 0;
			}
			if (pad_pressed(PSP_CTRL_UP))
			{
				if (pos > 0) pos--;
				update = 1;
			}
			else if (pad_pressed(PSP_CTRL_DOWN))
			{
				if (pos < max - 1) pos++;
				update = 1;
			}
			else if (pad_pressed(PSP_CTRL_CIRCLE))
			{
				if (GetPspEntry(mac, name) > 0)
				{
					if (strcmp(name, g_matchingData) == 0)
					{
						currentState = PSP_SELECTING;
						sceNetAdhocMatchingSelectTarget(matchingId, mac, 0, 0);
						update = 1;
					}
				}
			}
			else if (pad_pressed(PSP_CTRL_TRIANGLE))
			{
				msg_set_text_color(0xffffffff);
				adhocDisconnect();
				pad_wait_clear();
				return -1;
			}
			if (matchChanged)
			{
				if (g_matchEvent == MATCHING_SELECTED)
				{
					memcpy(mac, g_mac, 6);
					strcpy(name, g_matchOptData);
					currentState = PSP_SELECTED;
				}
				update = 1;
			}
			break;

		case PSP_SELECTING:
			if (update)
			{
				msg_screen_init(WP_LOGO, ICON_SYSTEM, title);
				sceNetEtherNtostr((UINT8 *)mac, temp);
				msg_printf(TEXT(WAITING_FOR_x_TO_ACCEPT_THE_CONNECTION), temp);
				msg_printf(TEXT(TO_CANCEL_PRESS_CROSS));
				update = 0;
			}
			if (pad_pressed(PSP_CTRL_CROSS))
			{
				sceNetAdhocMatchingCancelTarget(matchingId, mac);
				currentState = PSP_LISTING;
				update = 1;
			}
			if (matchChanged)
			{
				switch (g_matchEvent)
				{
				case MATCHING_SELECTED:
					sceNetAdhocMatchingCancelTarget(matchingId, mac);
					break;

				case MATCHING_ESTABLISHED:
					currentState = PSP_ESTABLISHED;
					break;

				case MATCHING_REJECTED:
					currentState = PSP_LISTING;
					break;
				}
				update = 1;
			}
			break;

		case PSP_SELECTED:
			Server = 1;
			if (update)
			{
				msg_screen_init(WP_LOGO, ICON_SYSTEM, title);
				sceNetEtherNtostr((UINT8 *)mac, temp);
				msg_printf(TEXT(x_HAS_REQUESTED_A_CONNECTION), temp);
				msg_printf(TEXT(TO_ACCEPT_THE_CONNECTION_PRESS_CIRCLE_TO_CANCEL_PRESS_CIRCLE));
				update = 0;
			}
			if (pad_pressed(PSP_CTRL_CROSS))
			{
				sceNetAdhocMatchingCancelTarget(matchingId, mac);
				currentState = PSP_LISTING;
				update = 1;
			}
			else if (pad_pressed(PSP_CTRL_CIRCLE))
			{
				sceNetAdhocMatchingSelectTarget(matchingId, mac, 0, 0);
				currentState = PSP_WAIT_EST;
				update = 1;
			}
			if (matchChanged)
			{
				if (g_matchEvent == MATCHING_CANCELED)
				{
					currentState = PSP_LISTING;
				}
				update = 1;
			}
			break;

		case PSP_WAIT_EST:
			if (matchChanged)
			{
				if (g_matchEvent == MATCHING_ESTABLISHED)
				{
					currentState = PSP_ESTABLISHED;
				}
				update = 1;
			}
			break;
		}

		matchChanged = 0;
		if (currentState == PSP_ESTABLISHED)
			break;

		if (top > max - rows) top = max - rows;
		if (top < 0) top = 0;
		if (pos >= top + rows) top = pos - rows + 1;
		if (pos < top) top = pos;

		if (max != prev_max)
		{
			prev_max = max;
			update = 1;
		}

		sceDisplayWaitVblankStart();
	}

	msg_set_text_color(0xffffffff);

	if (Server) sceWlanGetEtherAddr(mac);

	sceNetEtherNtostr((UINT8 *)mac, temp);

	ssid[0] = temp[ 9];
	ssid[1] = temp[10];
	ssid[2] = temp[12];
	ssid[3] = temp[13];
	ssid[4] = temp[15];
	ssid[5] = temp[16];
	ssid[6] = '\0';

	if (adhocReconnect(ssid) < 0)
		return -1;

	return (Server ? 1 : 0);
}


/*--------------------------------------------------------
	�f�[�^���M
--------------------------------------------------------*/

int adhocSend(void *buffer, int length)
{
	if (sceNetAdhocPdpSend(pdpId, g_mac, 0x309, buffer, length, 0, 1) < 0)
		return 0;
	return length;
}


/*--------------------------------------------------------
	�f�[�^��M
--------------------------------------------------------*/

int adhocRecv(void *buffer, int length)
{
	UINT16 port = 0;
	char mac[6];

	if (sceNetAdhocPdpRecv(pdpId, mac, &port, buffer, &length, 0, 1) < 0)
		return 0;
	return length;
}


/*--------------------------------------------------------
	�f�[�^���M��҂�
--------------------------------------------------------*/

int adhocSendBlocking(void *buffer, int length)
{
	if (sceNetAdhocPdpSend(pdpId, g_mac, 0x309, buffer, length, ADHOC_TIMEOUT, 0) < 0)
		return 0;
	return length;
}


/*--------------------------------------------------------
	�f�[�^��M��҂�
--------------------------------------------------------*/

int adhocRecvBlocking(void *buffer, int length)
{
	UINT16 port = 0;
	char mac[6];

	if (sceNetAdhocPdpRecv(pdpId, mac, &port, buffer, &length, ADHOC_TIMEOUT, 0) < 0)
		return 0;
	return length;
}


/*--------------------------------------------------------
	�f�[�^��M��҂�(�^�C���A�E�g�w��)
--------------------------------------------------------*/

int adhocRecvBlockingTimeout(void *buffer, int length, int timeout)
{
	UINT16 port = 0;
	char mac[6];

	if (sceNetAdhocPdpRecv(pdpId, mac, &port, buffer, &length, timeout, 0) < 0)
		return 0;
	return length;
}


/*--------------------------------------------------------
	�f�[�^�𑗐M���Aack����M����܂ő҂�
--------------------------------------------------------*/

int adhocSendRecvAck(void *buffer, int length)
{
	int ack_data  = 0;
	int tempLen   = length;
	int sentCount = 0;
	UINT8 *buf = (UINT8 *)buffer;

	do
	{
		if (tempLen > ADHOC_BLOCKSIZE) tempLen = ADHOC_BLOCKSIZE;

		adhocSendBlocking(buf, tempLen);

		if (adhocRecvBlocking(&ack_data, sizeof(int)) == 0)
			return 0;

		if (ack_data != tempLen)
			return 0;

		buf += ADHOC_BLOCKSIZE;
		sentCount += ADHOC_BLOCKSIZE;
		tempLen = length - sentCount;
	} while (sentCount < length);

	return length;
}


/*--------------------------------------------------------
	�f�[�^�̎�M��҂��Aack�𑗐M����
--------------------------------------------------------*/

int adhocRecvSendAck(void *buffer, int length)
{
	int tempLen   = length;
	int rcvdCount = 0;
	UINT8 *buf = (UINT8 *)buffer;

	do
	{
		if (tempLen > ADHOC_BLOCKSIZE) tempLen = ADHOC_BLOCKSIZE;

		if (adhocRecvBlocking(buf, tempLen) == 0)
			return 0;

		adhocSendBlocking(&tempLen, sizeof(int));

		buf += ADHOC_BLOCKSIZE;
		rcvdCount += ADHOC_BLOCKSIZE;
		tempLen = length - rcvdCount;
	} while (rcvdCount < length);

	return length;
}
