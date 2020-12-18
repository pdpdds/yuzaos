// pdcurse1.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "curses.h"
#include <time.h>
#pragma comment(lib, "pdcurses.lib")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include "tui.h"
#define FNAME "tui.c"
#include <sprintf.h>

/**************************** strings entry box ***************************/

void address(void)
{
	const char* fieldname[6] =
	{
		"Name", "Street", "City", "State", "Country", (char *)0
	};

	char *fieldbuf[5];
	WINDOW *wbody = bodywin();
	int i, field = 50;

	for (i = 0; i < 5; i++)
		fieldbuf[i] = (char*)malloc(1 * field + 1);

	if (getstrings((char**)fieldname, fieldbuf, field) != KEY_ESC)
	{
		for (i = 0; fieldname[i]; i++)
			wprintw(wbody, "%10s : %s\n",
				fieldname[i], fieldbuf[i]);

		wrefresh(wbody);
	}

	for (i = 0; i < 5; i++)
		free(fieldbuf[i]);
}

/**************************** string entry box ****************************/

char *getfname(char *desc, char *fname, int field)
{
	char *fieldname[2];
	char *fieldbuf[1];

	fieldname[0] = desc;
	fieldname[1] = 0;
	fieldbuf[0] = fname;

	return (getstrings(fieldname, fieldbuf, field) == KEY_ESC) ? NULL : fname;
}

/**************************** a very simple file browser ******************/

void showfile(char *fname)
{
	int i, bh = bodylen();
	FILE *fp;
	char buf[MAXSTRLEN];
	bool ateof = FALSE;

	statusmsg((char*)"FileBrowser: Hit key to continue, Q to quit");

	if ((fp = fopen(fname, "r")) != NULL)   /* file available? */
	{
		while (!ateof)
		{
			clsbody();

			for (i = 0; i < bh - 1 && !ateof; i++)
			{
				if (fgets(buf, MAXSTRLEN, fp))
					bodymsg(buf);
				else
					ateof = TRUE;
			}

			switch (waitforkey())
			{
			case 'Q':
			case 'q':
			case 0x1b:
				ateof = TRUE;
			}
		}

		fclose(fp);
	}
	else
	{
		sprintf(buf, "ERROR: file '%s' not found", fname);
		errormsg(buf);
	}
}

/***************************** forward declarations ***********************/

void sub0(void), sub1(void), sub2(void), sub3(void);
void func1(void), func2(void);
void subfunc1(void), subfunc2(void);
void subsub(void);

/***************************** menus initialization ***********************/

menu MainMenu[] =
{
	{ (char*)"Asub", sub0, (char*)"Go inside first submenu" },
{ (char*)"Bsub", sub1, (char*)"Go inside second submenu" },
{ (char*)"Csub", sub2, (char*)"Go inside third submenu" },
{ (char*)"Dsub", sub3, (char*)"Go inside fourth submenu" },
{ (char*)"", (FUNC)0, (char*)"" }   /* always add this as the last item! */
};

menu SubMenu0[] =
{
	{ (char*)"Exit", DoExit, (char*)"Terminate program" },
{ (char*)"", (FUNC)0, (char*)"" }
};

menu SubMenu1[] =
{
	{ (char*)"OneBeep", func1, (char*)"Sound one beep" },
{ (char*)"TwoBeeps", func2, (char*)"Sound two beeps" },
{ (char*)"", (FUNC)0, (char*)"" }
};

menu SubMenu2[] =
{
	{ (char*)"Browse", subfunc1, (char*)"Source file lister" },
{ (char*)"Input", subfunc2, (char*)"Interactive file lister" },
{ (char*)"Address", address, (char*)"Get address data" },
{ (char*)"", (FUNC)0, (char*)"" }
};

menu SubMenu3[] =
{
	{ (char*)"SubSub", subsub, (char*)"Go inside sub-submenu" },
{ (char*)"", (FUNC)0, (char*)"" }
};

/***************************** main menu functions ************************/

void sub0(void)
{
	domenu(SubMenu0);
}

void sub1(void)
{
	domenu(SubMenu1);
}

void sub2(void)
{
	domenu(SubMenu2);
}

void sub3(void)
{
	domenu(SubMenu3);
}

/***************************** submenu1 functions *************************/

void func1(void)
{
	beep();
	bodymsg((char*)"One beep! ");
}

void func2(void)
{
	beep();
	bodymsg((char*)"Two beeps! ");
	beep();
}

/***************************** submenu2 functions *************************/

void subfunc1(void)
{
	showfile((char*)FNAME);
}

void subfunc2(void)
{
	char fname[MAXSTRLEN];

	strcpy(fname, FNAME);
	if (getfname((char*)"File to browse:", fname, 50))
		showfile(fname);
}

/***************************** submenu3 functions *************************/

void subsub(void)
{
	domenu(SubMenu2);
}

/***************************** start main menu  ***************************/

int main2(int argc, char **argv)
{
	//setlocale(LC_ALL, "");

	startmenu(MainMenu, (char*)"TUI - 'textual user interface' demonstration program");

	return 0;
}