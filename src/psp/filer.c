/******************************************************************************

	filer.c

	PSP ファイルブラウザ

******************************************************************************/

#include "psp.h"
#include "font/jpnfont.h"
#include "emumain.h"

#define MAX_ENTRY 1024

#define GAME_NOT_WORK	1


/******************************************************************************
	グローバル変数
******************************************************************************/

char startupDir[MAX_PATH];


/******************************************************************************
	ローカル構造体/変数
******************************************************************************/

struct dirent
{
	u32 type;
	int icon;
	int bad;
	int not_work;
	char *title;
	char name[128];
};

static struct dirent files[MAX_ENTRY];
static SceIoDirent dir;

static struct zipname_t
{
	char zipname[16];
	char title[128];
	int flag;
} zipname[200];

static int zipname_num;


/******************************************************************************
	ローカル変数
******************************************************************************/

static char curr_dir[MAX_PATH];
static int nfiles;


/******************************************************************************
	ローカル関数
******************************************************************************/

/*--------------------------------------------------------
	zipname.datからzipファイル名データベースを読み込み
--------------------------------------------------------*/

#if (EMU_SYSTEM == CPS1)
#define EXT		"cps1"
#elif (EMU_SYSTEM == CPS2)
#define EXT		"cps2"
#elif (EMU_SYSTEM == MVS)
#define EXT		"mvs"
#endif

static int load_zipname(void)
{
	FILE *fp;
	char path[MAX_PATH], buf[256];
	int size, found = 0;

	sprintf(path, "%szipnamej." EXT, launchDir);
	if ((fp = fopen(path, "rb")) != NULL)
	{
		fclose(fp);
		if (load_jpnfont())
		{
			fp = fopen(path, "rb");
			found = 1;
		}
	}
	if (!found)
	{
		sprintf(path, "%szipname." EXT, launchDir);
	}
	if ((fp = fopen(path, "rb")) == NULL)
		return 0;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	zipname_num = 0;
	while (zipname_num < 200)
	{
		char *linebuf;
		char *name;
		char *title;
		char *flag;

		memset(buf, 0, 256);

		if (!fgets(buf, 255, fp)) break;

		linebuf = strtok(buf, "\r\n");

		zipname[zipname_num].flag = 0;

		name  = strtok(linebuf, ",\r\n");
		title = strtok(NULL, ",\r\n");
		flag  = strtok(NULL, ",\r\n");

		strcpy(zipname[zipname_num].zipname, name);
		strcpy(zipname[zipname_num].title, title);
		if (flag)
		{
			if (!strcmp(flag, "GAME_NOT_WORK"))
				zipname[zipname_num].flag = GAME_NOT_WORK;
		}
		zipname_num++;
	}

	fclose(fp);

	return 1;
}


/*--------------------------------------------------------
	zipファイル名データベースを解放
--------------------------------------------------------*/

static void free_zipname(void)
{
	free_jpnfont();
	zipname_num = 0;
}


/*--------------------------------------------------------
	zipファイル名からゲームタイトル名を取得
--------------------------------------------------------*/

static char *get_zipname(const char *name, int *flag)
{
	int i, length;
	char fname[MAX_PATH];

	strcpy(fname, name);
	*strrchr(fname, '.') = '\0';

	length = strlen(fname);
	if (length > 8) return NULL;

	for (i = 0; i < length; i++)
		fname[i] = tolower(fname[i]);

	for (i = 0; i < zipname_num; i++)
	{
		if (stricmp(fname, zipname[i].zipname) == 0)
		{
			*flag = zipname[i].flag;
			return zipname[i].title;
		}
	}
	*flag = 0;
	return NULL;
}


/*--------------------------------------------------------
	ディレクトリの存在チェック
--------------------------------------------------------*/

static void checkDir(const char *name)
{
	int fd, found;
	char path[MAX_PATH];

	memset(&dir, 0, sizeof(dir));

	fd = sceIoDopen(launchDir);
	found = 0;

	while (!found)
	{
		if (sceIoDread(fd, &dir) <= 0) break;

		if (dir.d_stat.st_attr == FIO_SO_IFDIR)
			if (stricmp(dir.d_name, name) == 0)
				found = 1;
	}

	sceIoDclose(fd);

	if (!found)
	{
		sprintf(path, "%s%s", launchDir, name);
		sceIoMkdir(path, 0777);
	}
}


/*--------------------------------------------------------
	起動ディレクトリのチェック
--------------------------------------------------------*/

static void checkStartupDir(void)
{
	int fd = sceIoDopen(startupDir);

	if (fd >= 0)
	{
		strcpy(curr_dir, startupDir);
		sceIoDclose(fd);
	}
	else
	{
		strcpy(startupDir, launchDir);
		strcat(startupDir, "roms/");
	}
}


/*--------------------------------------------------------
	ディレクトリエントリを取得
--------------------------------------------------------*/

static void getDir(const char *path)
{
	static int fd;
	int i, j;

	memset(&dir, 0, sizeof(dir));
	memset(files, 0, sizeof(files));

	nfiles = 0;

	if (strcmp(path, "ms0:/") != 0)
	{
		strcpy(files[nfiles].name, "..");
		files[nfiles].type = 0;
		files[nfiles].icon = ICON_UPPERDIR;
		files[nfiles].title = files[nfiles].name;
		nfiles++;
	}

	fd = sceIoDopen(path);

	while (nfiles < MAX_ENTRY)
	{
		char *ext;

		if (sceIoDread(fd, &dir) <= 0) break;

		if (dir.d_name[0] == '.') continue;
#if (EMU_SYSTEM == MVS)
		if (stricmp(dir.d_name, "neogeo.zip") == 0) continue;
#endif

		if ((ext = strrchr(dir.d_name, '.')) != NULL)
		{
			if (stricmp(ext, ".zip") == 0)
			{
				strcpy(files[nfiles].name, dir.d_name);
				if ((files[nfiles].title = get_zipname(dir.d_name, &files[nfiles].not_work)) == NULL)
					files[nfiles].title = files[nfiles].name;
				files[nfiles].type = 2;
				files[nfiles].icon = ICON_ZIPFILE;
				nfiles++;
				continue;
			}
		}

		if (dir.d_stat.st_attr == FIO_SO_IFDIR)
		{
#if (EMU_SYSTEM == MVS)
			if (stricmp(dir.d_name, "memcard") == 0) continue;
#endif
			if (stricmp(dir.d_name, "cache") == 0) continue;
			if (stricmp(dir.d_name, "config") == 0) continue;
			if (stricmp(dir.d_name, "snap") == 0) continue;
			if (stricmp(dir.d_name, "nvram") == 0) continue;
			if (stricmp(dir.d_name, "font") == 0) continue;
			if (stricmp(dir.d_name, "state") == 0) continue;
			strcpy(files[nfiles].name, dir.d_name);
			strcat(files[nfiles].name, "/");
			files[nfiles].type = 1;
			files[nfiles].icon = ICON_FOLDER;
			files[nfiles].title = files[nfiles].name;
			nfiles++;
		}
	}

	sceIoDclose(fd);

	// ソートするアイテム数は数十個程度なので、バブルソートの方が速いと思う。
	for (i = 0; i < nfiles - 1; i++)
	{
		for (j = i + 1; j < nfiles; j++)
		{
			if (files[i].type > files[j].type)
			{
				struct dirent tmp;

				tmp = files[i];
				files[i] = files[j];
				files[j] = tmp;
			}
		}
	}

	for (i = 0; i < nfiles - 1; i++)
	{
		for (j = i + 1; j < nfiles; j++)
		{
			if (files[i].type != files[j].type) break;

			if (strcmp(files[i].title, files[j].title) > 0)
			{
				struct dirent tmp;

				tmp = files[i];
				files[i] = files[j];
				files[j] = tmp;
			}
		}
	}

	for (i = 0; i < nfiles; i++)
	{
		if (files[i].icon == ICON_ZIPFILE)
		{
			if ((files[i].title = get_zipname(files[i].name, &files[i].not_work)) != NULL)
			{
				files[i].bad = 0;
			}
			else
			{
				files[i].title = files[i].name;
				files[i].bad = 1;
			}
		}
		else
		{
			files[i].title = files[i].name;
			files[i].not_work = 0;
			files[i].bad = 0;
		}
	}
}


/*--------------------------------------------------------
	パス文字列を表示領域に合わせて修正
--------------------------------------------------------*/

static void modify_display_path(char *path, char *org_path, int max_width)
{
	if (uifont_get_string_width(org_path) > max_width)
	{
		int i, j, num_dir = 0;
		char temp[MAX_PATH], *dir[256];

		strcpy(temp, org_path);

		dir[num_dir++] = strtok(temp, "/");

		while ((dir[num_dir] = strtok(NULL, "/")) != NULL)
			num_dir++;

		j = num_dir - 1;

		do
		{
			j--;

			path[0] = '\0';

			for (i = 0; i < j; i++)
			{
				strcat(path, dir[i]);
				strcat(path, "/");
			}

			strcat(path, ".../");
			strcat(path, dir[num_dir - 1]);
			strcat(path, "/");

		} while (uifont_get_string_width(path) > max_width);
	}
	else strcpy(path, org_path);
}


/******************************************************************************
	グローバル関数
******************************************************************************/

/*--------------------------------------------------------
	ファイルの存在チェック
--------------------------------------------------------*/

int file_exist(const char *path)
{
	int found;
	FILE *fp;

	fp = fopen(path, "rb");
	found = (fp) ? 1 : 0;
	fclose(fp);

	return found;
}


/*--------------------------------------------------------
	指定したパターンのファイルを削除
--------------------------------------------------------*/

void delete_files(const char *dirname, const char *pattern)
{
	int fd, i, len1, len2;
	char path[MAX_PATH];

	memset(&dir, 0, sizeof(dir));

	sprintf(path, "%s%s/", launchDir, dirname);

	fd = sceIoDopen(path);
	len1 = strlen(pattern);

	while (1)
	{
		if (sceIoDread(fd, &dir) <= 0) break;

		len2 = strlen(dir.d_name);

		for (i = 0; i < len2; i++)
		{
			if (strnicmp(&dir.d_name[i], pattern, len1) == 0)
			{
				char path2[MAX_PATH];

				sprintf(path2, "%s%s", path, dir.d_name);
				sceIoRemove(path2);
			}
		}
	}

	sceIoDclose(fd);
}


/*--------------------------------------------------------
	ステートファイルを検索
--------------------------------------------------------*/

#ifdef SAVE_STATE

void find_state_file(u8 *slot)
{
	int fd, len;
	char path[MAX_PATH], pattern[16];

	memset(&dir, 0, sizeof(dir));

	sprintf(path, "%sstate", launchDir);
	sprintf(pattern, "%s.sv", game_name);

	len = strlen(pattern);
	fd = sceIoDopen(path);

	while (sceIoDread(fd, &dir) > 0)
	{
		if (strnicmp(dir.d_name, pattern, len) == 0)
		{
			int number = dir.d_name[len] - '0';

			if (number >= 0 && number <= 9)
				slot[number] = 1;
		}
	}

	sceIoDclose(fd);
}

#endif


/*--------------------------------------------------------
	アプリケーション終了画面を表示
--------------------------------------------------------*/

void show_exit_screen(void)
{
	if (Loop == LOOP_EXIT)
	{
		video_clear_screen();
		boxfill(0, 0, SCR_WIDTH - 1, SCR_HEIGHT - 1, COLOR_DARKGRAY);
		uifont_print_center(129, COLOR_WHITE, "Please wait...");
		video_flip_screen(1);
	}
}


/*--------------------------------------------------------
	ファイルブラウザ実行
--------------------------------------------------------*/

void file_browser(void)
{
	int i, sel = 0, rows = 11, top = 0;
	int run_emulation = 0, update = 1, prev_sel = 0;
	char *p;

	Loop = LOOP_BROWSER;

	ui_popup_reset(POPUP_MENU);

	memset(files, 0, sizeof(files));
	memset(zipname, 0, sizeof(zipname));
	zipname_num = 0;

	strcpy(curr_dir, launchDir);
	strcat(curr_dir, "roms/");
	strcpy(startupDir, curr_dir);
	load_settings();

	ui_fill_frame(draw_frame, UI_PAL_BG2);
	draw_dialog(240-164, 136-40, 240+164, 136+40);
	uifont_print_center(136-20, 255,255,120, APPNAME_STR " " VERSION_STR "  by NJ");
	uifont_print_center(136+10, 220,220,220, "http://neocdz.hp.infoseek.co.jp/psp/");
	video_flip_screen(1);

#if USE_CACHE
	checkDir("cache");
#endif
	checkDir("roms");
	checkDir("config");
	checkDir("nvram");
	checkDir("snap");
#if (EMU_SYSTEM == MVS)
	checkDir("memcard");
#endif
#ifdef SAVE_STATE
	checkDir("state");
#endif

	if (!load_zipname())
	{
		fatalerror("Could not open zipname." EXT);
		show_fatal_error();
		show_exit_screen();
		goto error;
	}
 	checkStartupDir();
	getDir(curr_dir);

	pad_wait_press(3000);

	load_background();

	while (Loop)
	{
		if (run_emulation)
		{
			int len;

			run_emulation = 0;

			strcpy(game_dir, curr_dir);
			strcpy(game_name, files[sel].name);
			*strchr(game_name, '.') = '\0';
			len = strlen(game_name);
			for (i = 0; i < len; i++)
				game_name[i] = tolower(game_name[i]);

			*strrchr(game_dir, '/') = '\0';
#if USE_CACHE
			strcpy(cache_dir, launchDir);
			strcat(cache_dir, "cache");
#endif

			free_zipname();
			set_cpu_clock(psp_cpuclock);

#ifdef SOUND_TEST
			if (sound_test)
				soundtest_main();
			else
#endif
				emu_main();
			set_cpu_clock(PSPCLOCK_222);

			if (Loop)
			{
#ifdef KERNEL_MODE
				main_thread_set_priority(0x11);
#endif
				load_zipname();
				load_background();
				getDir(curr_dir);
				update = 1;
			}
			else break;
		}

		if (update)
		{
			char path[MAX_PATH];

			modify_display_path(path, curr_dir, 368);

			show_background();
			small_icon(6, 3, UI_COLOR(UI_PAL_TITLE), ICON_MEMSTICK);
			uifont_print(32, 5, UI_COLOR(UI_PAL_TITLE), path);

			for (i = 0; i < rows; i++)
			{
				if (top + i >= nfiles) break;

				if (top + i == sel)
				{
					boxfill_alpha(4, 37 + i * 20, 464, 56 + i * 20, UI_COLOR(UI_PAL_FILESEL), 8);
					small_icon(6, 38 + i * 20, UI_COLOR(UI_PAL_SELECT), files[sel].icon);
					if (files[sel].bad)
						uifont_print(36, 40 + i * 20, COLOR_RED, files[sel].title);
					else if (files[sel].not_work)
						uifont_print(36, 40 + i * 20, COLOR_GRAY, files[sel].title);
					else
						uifont_print(36, 40 + i * 20, UI_COLOR(UI_PAL_SELECT), files[sel].title);
				}
				else
				{
					small_icon(6, 38 + i * 20, UI_COLOR(UI_PAL_NORMAL), files[top + i].icon);
					if (files[top + i].bad)
						uifont_print(36, 40 + i * 20, COLOR_DARKRED, files[top + i].title);
					else if (files[top + i].not_work)
						uifont_print(36, 40 + i * 20, COLOR_DARKGRAY, files[top + i].title);
					else
						uifont_print(36, 40 + i * 20, UI_COLOR(UI_PAL_NORMAL), files[top + i].title);
				}
			}

			draw_scrollbar(469, 26, 479, 270, rows, nfiles, sel);

			update  = draw_battery_status(1);
			update |= ui_show_popup(1);
			video_flip_screen(1);
		}
		else
		{
			update  = draw_battery_status(0);
			update |= ui_show_popup(0);
			video_wait_vsync();
		}

		prev_sel = sel;

		if (pad_pressed(PSP_CTRL_UP))
		{
			sel--;
		}
		else if (pad_pressed(PSP_CTRL_DOWN))
		{
			sel++;
		}
		else if (pad_pressed(PSP_CTRL_LEFT))
		{
			sel -= rows;
			if (sel < 0) sel = 0;
		}
		else if (pad_pressed(PSP_CTRL_RIGHT))
		{
			sel += rows;
			if (sel >= nfiles) sel = nfiles - 1;
		}
		else if (pad_pressed(PSP_CTRL_CIRCLE))
		{
			switch (files[sel].icon)
			{
			case ICON_UPPERDIR:
				if (strcmp(curr_dir, "ms0:/") != 0)
				{
					char old_dir[MAX_PATH];

					*strrchr(curr_dir, '/') = '\0';
					p = strrchr(curr_dir, '/') + 1;
					strcpy(old_dir, p);
					strcat(old_dir,  "/");
					*p = '\0';

					getDir(curr_dir);
					sel = 0;
					prev_sel = -1;

					for (i = 0; i < nfiles; i++)
					{
						if (!strcmp(old_dir, files[i].name))
						{
							sel = i;
							top = sel - 3;
							break;
						}
					}
				}
				break;

			case ICON_FOLDER:
				strcat(curr_dir, files[sel].name);
				getDir(curr_dir);
				sel = 0;
				prev_sel = -1;
				pad_wait_clear();
				break;

			case ICON_ZIPFILE:
				if (!files[sel].bad)
				{
					if (files[sel].not_work)
					{
						messagebox(MB_GAMENOTWORK);
					}
					else if (messagebox(MB_LAUNCHZIPFILE))
					{
						run_emulation = 1;
					}
					update = 1;
				}
				break;
			}

			pad_wait_clear();
		}
		else if (pad_pressed(PSP_CTRL_TRIANGLE))
		{
			if (messagebox(MB_EXITEMULATOR))
			{
				Loop = LOOP_EXIT;
				show_exit_screen();
				break;
			}
			update = 1;
		}
#ifdef SOUND_TEST
		else if (pad_pressed(PSP_CTRL_SQUARE))
		{
			if (files[sel].icon == ICON_ZIPFILE)
			{
				if (messagebox(MB_SOUNDTEST))
				{
					sound_test = 1;
					run_emulation = 1;
				}
				update = 1;
				pad_wait_clear();
			}
		}
#endif
		else if (pad_pressed(PSP_CTRL_SELECT))
		{
			if (files[sel].icon == ICON_FOLDER)
			{
				if (messagebox(MB_SETSTARTUPDIR))
					sprintf(startupDir, "%s%s", curr_dir, files[sel].name);
			}
		}
#if (EMU_SYSTEM == MVS)
		else if (pad_pressed(PSP_CTRL_LTRIGGER))
		{
			strcpy(game_dir, curr_dir);
			*strrchr(game_dir, '/') = '\0';
			bios_select(0);
			update = 1;
		}
#endif
		else if (pad_pressed(PSP_CTRL_RTRIGGER))
		{
			help(HELP_FILEBROWSER);
			update = 1;
		}

		if (top > nfiles - rows) top = nfiles - rows;
		if (top < 0) top = 0;
		if (sel >= nfiles) sel = 0;
		if (sel < 0) sel = nfiles - 1;
		if (sel >= top + rows) top = sel - rows + 1;
		if (sel < top) top = sel;

		if (prev_sel != sel) update = 1;

		pad_update();
	}

	save_settings();
error:
	free_zipname();
}
