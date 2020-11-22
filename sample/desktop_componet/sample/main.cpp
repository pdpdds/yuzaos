#include "svgamgr.h"
#include "play.xpm"

GuiWindow* color_win;
GuiObject* counter, * color[3], * color_text;
int kleur, symbol = 0;

static int count = 0;
static int exit_program = FALSE;


void counter_cb(GuiObject* obj, int data)
{
	GuiWindow* win = counter->win;
	int min, hour;

	if (data == 0)		/* decrease */
		count--;
	if (data == 1)		/* increase */
		count++;

	hour = count / 60;
	min = count % 60;
	sprintf(counter->label, "%02d:%02d", hour, min);
	win->x_min = 0;		/* update full window */
	win->y_min = 0;
	win->x_max = win->width - 1;
	win->y_max = win->height - 1;
	show_number(counter);
	if (data == 0)
		show_button(obj);
}


void all_dialogs_cb(GuiObject* obj, int data)
{
	GuiWinThread* win_thread = (obj->win)->win_thread;
	GuiWindow* win;
	int i;

	if (question_dialog(win_thread, NULL, "Show progress dialog box ?", DIA_QUESTION)) {
		message_dialog(win_thread, NULL, "Press OK to start.", FALSE);
		win = init_progress(win_thread, "Testing 1,2,3....");
		for (i = 0; i <= 100; i++) {
			Syscall_Sleep(40);
			show_progress(win, i / 100.0);
		}
		delete_progress(win);
	}
	if (obj->objclass == BUTTON)
		show_button(obj);
}


static GuiWindow* create_slider_win(GuiWinThread* win_thread)
{
	GuiWindow* win;
	GuiObject* obj;

	win = add_window(win_thread, NORMAL_WINDOW, 170, 50, 155, 170, "Sliders", TRUE, FALSE);
	obj = add_slider(win, NICE_VALUE_SLIDER, 20, 40, 101, FALSE);
	set_slider_maxvalue(obj, 1.0);
	obj = add_slider(win, NICE_VALUE_SLIDER, 70, 40, 101, FALSE);
	set_slider_maxvalue(obj, 10);
	obj = add_slider(win, NICE_VALUE_SLIDER, 120, 40, 101, FALSE);
	set_slider_maxvalue(obj, 100);
	obj = add_slider(win, NICE_HOR_SLIDER, 10, 150, 135, FALSE);
	create_window(win);

	return win;
}


static void window_cb(GuiObject* obj, int user_data)
{
	GuiWinThread* win_thread = (obj->win)->win_thread;
	GuiWindow* win;

	show_button(obj);
	win = create_slider_win(win_thread);
	show_window(win);
}


static void button_cb(GuiObject* obj, int data)
{
	show_button(obj);
}


static void about_cb(GuiObject* obj, int data)
{
	message_dialog((obj->win)->win_thread, NULL, "This is svgagui version 2.11 \nCopyright 2007 \nby B. Nagels", DIA_INFO);
}


static void question_cb(GuiObject* obj, int data)
{
	question_dialog((obj->win)->win_thread, NULL, "Is this a question?\nYes or no!", DIA_QUESTION);
}


static void message_cb(GuiObject* obj, int data)
{
	message_dialog((obj->win)->win_thread, NULL, "This is a message consisting\nof two lines", data);
}


void progress_cb(GuiObject* obj, int data)
{
	GuiWinThread* win_thread = (obj->win)->win_thread;
	GuiWindow* win;
	int i;

	win = init_progress(win_thread, "Testing 1,2,3....");
	for (i = 0; i <= 100; i++) {
		Syscall_Sleep(4000);
		show_progress(win, i / 100.0);
	}
	delete_progress(win);
}


static void exit_cb(GuiObject* obj, int data)
{
	exit_program = TRUE;
	//fprintf(stderr, "Goodbye.\n");
}



static void autoraise_cb(GuiObject* obj, int data)
{
	((obj->win)->win_thread)->auto_raise = obj->pressed;
}


static void enablewin_cb(GuiObject* obj, int data)
{
	if (obj->pressed)
		enable_all_windows(obj->win);
	else
		disable_all_windows_except(obj->win);
}


static void file_cb(GuiObject* obj, int data)
{
	char directory[128], filename[128], mask[20];

	sprintf(mask, "C files|*.c|Object files|*.o|All files|*|");
	directory[0] = '\0';
	CreateFileDialog((obj->win)->win_thread, "File dialog", "Open");
	if (file_dialog(directory, filename, mask))
	{

	}
	//fprintf(stderr, "Dir: %s\nFile: %s\n", directory, filename);
}

static void select_color_cb(GuiObject* obj, int data)
{
	int r, g, b;

	kleur = data;
	get_gui_palette(kleur, &r, &g, &b);
	set_slider_position(color[0], (int)(r));
	set_slider_position(color[1], (int)(g));
	set_slider_position(color[2], (int)(b));
	win_fillbox(color_win, 100, 70, 40, 30, kleur);
	win_3dbox(color_win, DOWN_FRAME, 100, 70, 40, 30);
	show_window(color_win);
}

void default_cb(GuiObject* obj, int data)
{
	if (question_dialog((obj->win)->win_thread, NULL, "Restore default color setting?", DIA_QUESTION)) {
		set_default_palette();
		select_color_cb(NULL, kleur);
	}
}


void load_cb(GuiObject* obj, int data)
{
	FILE *invoer;
	int i, r, g, b;
	char hulp[50] = { 0, };

	//sprintf(hulp, "%s/.svgaguicolors", getenv("HOME"));
	invoer = fopen(hulp, "r");
	if (invoer != NULL) {
		for (i = 0; i < 25; i++) {
			fscanf(invoer, "%d %d %d", &r, &g, &b);
			set_gui_palette(i, r, g, b);
		}
		select_color_cb(NULL, kleur);
		fclose(invoer);
		message_dialog((obj->win)->win_thread, NULL, "Color settings loaded", DIA_INFO);
	}
}


void save_cb(GuiObject* obj, int data)
{
	FILE *uitvoer;
	int i, r, g, b;
	char hulp[50] = { 0, };

	//sprintf(hulp, "%s/.svgaguicolors", getenv("HOME"));
	uitvoer = fopen(hulp, "w");
	if (uitvoer != NULL) {
		for (i = 0;i < 25;i++) {
			get_gui_palette(i, &r, &g, &b);
			fprintf(uitvoer, "%2d %2d %2d\n", r, g, b);
		}
		fclose(uitvoer);
		message_dialog((obj->win)->win_thread, NULL, "Color settings saved", DIA_INFO);
	}
}

static void set_color_cb(GuiObject* obj, int data)
{
	int r, g, b;

	r = (int)(color[0]->position);
	g = (int)(color[1]->position);
	b = (int)(color[2]->position);
	set_gui_palette(kleur, r, g, b);
}
static void create_color_choice(GuiWindow* win)
{
	GuiObject *obj, *pd;

	pd = add_choice(win, 6, 24, 130);
	  obj = add_item(pd, "Background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, BACKGROUND);
	  obj = add_item(pd, "Window background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, WIN_BACK);
	  obj = add_item(pd, "Title background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, TITLE_BACK);
	  obj = add_item(pd, "Title foreground", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, TITLE_FORE);
	  obj = add_item(pd, "Active title background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, ACTIVE_TITLE_BACK);
	  obj = add_item(pd, "Active title foreground", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, ACTIVE_TITLE_FORE);
	  obj = add_item(pd, "Normal text", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, TEXT_NORMAL);
	  obj = add_item(pd, "Browser background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, BROWSER_BACK);
	  obj = add_item(pd, "Browser foreground", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, BROWSER_FORE);
	  obj = add_item(pd, "Input background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, INPUT_BACK);
	  obj = add_item(pd, "Input foreground", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, INPUT_FORE);
	  obj = add_item(pd, "Active input background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, ACTIVE_INPUT_BACK);
	  obj = add_item(pd, "Button background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, BUTTON_BACK);
	  obj = add_item(pd, "Button foreground", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, BUTTON_FORE);
	  obj = add_item(pd, "Slider background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, SLIDER_BACK);
	  obj = add_item(pd, "Slider foreground", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, SLIDER_FORE);
	  obj = add_item(pd, "Choice background", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, CHOICE_BACK);
	  obj = add_item(pd, "Number foreground", NORMAL_ITEM);
	  set_object_callback(obj, select_color_cb);
	  set_object_user_data(obj, NUMBER_FORE);
	create_choice(pd);
}


void create_color_win(GuiWinThread* win_thread)
{
	int r, g, b;
	GuiWindow *win;
	GuiObject *obj;

	win = add_window(win_thread, NORMAL_WINDOW, 340, 50, 160, 135, "Colors", FALSE, FALSE);
	  color_win = win;
	  create_color_choice(win);
	  add_text(win, NORMAL_TEXT, 98, 55, "Example:");

	  obj = add_slider(win, NICE_VALUE_SLIDER, 10, 60, 64, FALSE);

		color[0] = obj;
		set_object_callback(obj, set_color_cb);
		set_slider_maxvalue(obj, 63);
		set_object_color(obj, WIN_BACK, SLIDER_BACK, RED);

	  obj = add_slider(win, NICE_VALUE_SLIDER, 40, 60, 64, FALSE);
		color[1] = obj;
		set_object_callback(obj, set_color_cb);
		set_slider_maxvalue(obj, 63);
		set_object_color(obj, WIN_BACK, SLIDER_BACK, GREEN);
	  obj = add_slider(win, NICE_VALUE_SLIDER, 70, 60, 64, FALSE);
		color[2] = obj;
		set_object_callback(obj, set_color_cb);
		set_slider_maxvalue(obj, 63);
		set_object_color(obj, WIN_BACK, SLIDER_BACK, BLUE);

	create_window(win);

	// set the current color (sliders etc.)
	get_gui_palette(kleur, &r, &g, &b);
	set_slider_position(color[0], (int)(r));
	set_slider_position(color[1], (int)(g));
	set_slider_position(color[2], (int)(b));
	win_fillbox(color_win, 100, 70, 40, 30, kleur);
	win_3dbox(color_win, DOWN_FRAME, 100, 70, 40, 30);
}

void CreateButtonBar(GuiWinThread* win_thread)
{
	GuiObject* obj, * pd, * popup;
	GuiWindow* win;
	char text[] = "This is a test of the browser function.\n\
Using the sliders you are able to read\n\
this text. Scrolling up and down and left and right.\n\
This\n\
is\n\
a\n\
test.\n\
Testing 1...2";

	/* create the button bar */
	win = add_window(win_thread, NO_TITLE_WINDOW, 0, 0, guiscreen.width, 42, "", FALSE, FALSE);

	pd = add_pulldown(win, 4, 2, "File");
	obj = add_item(pd, "Open", NORMAL_ITEM);
	set_object_callback(obj, file_cb);
	popup = add_item(pd, "Import", POPUP_ITEM);
	add_item(popup, "Test 1", NORMAL_ITEM);
	//	    add_item(popup, "Test 2", NORMAL_ITEM);
	//add_item(popup, "Test 3", NORMAL_ITEM);
	create_pulldown(popup);
	popup = add_item(pd, "Export", POPUP_ITEM);
	obj = add_item(popup, "Inactive", NORMAL_ITEM);
	set_object_active(obj, FALSE);
	add_item(popup, "Check 1", CHECK_ITEM);
	add_item(popup, "Check 2", CHECK_ITEM);
	create_pulldown(popup);
	add_pd_separator(pd);
	add_item(pd, "Check1", CHECK_ITEM);
	obj = add_item(pd, "Check2", CHECK_ITEM);
	set_object_pressed(obj, TRUE);
	obj = add_item(pd, "This is a test", NORMAL_ITEM);
	set_object_active(obj, FALSE);
	add_pd_separator(pd);
	obj = add_item(pd, "Exit", NORMAL_ITEM);
	set_object_callback(obj, exit_cb);
	create_pulldown(pd);
	pd = add_pulldown(win, 35, 2, "Color");
	obj = add_item(pd, "Load settings", NORMAL_ITEM);
	set_object_callback(obj, load_cb);
	obj = add_item(pd, "Save settings", NORMAL_ITEM);
	set_object_callback(obj, save_cb);
	obj = add_item(pd, "Default colors", NORMAL_ITEM);
	set_object_callback(obj, default_cb);
	create_pulldown(pd);
	pd = add_pulldown(win, 71, 2, "Other");
	obj = add_item(pd, "Show question dialog", NORMAL_ITEM);
	set_object_callback(obj, question_cb);
	popup = add_item(pd, "Show message dialog", POPUP_ITEM);
	obj = add_item(popup, "Question", NORMAL_ITEM);
	set_object_callback(obj, message_cb);
	set_object_user_data(obj, DIA_QUESTION);
	obj = add_item(popup, "Exclamation", NORMAL_ITEM);
	set_object_callback(obj, message_cb);
	set_object_user_data(obj, DIA_EXCLAMATION);
	obj = add_item(popup, "Stop", NORMAL_ITEM);
	set_object_callback(obj, message_cb);
	set_object_user_data(obj, DIA_STOP);
	obj = add_item(popup, "Info", NORMAL_ITEM);
	set_object_callback(obj, message_cb);
	set_object_user_data(obj, DIA_INFO);
	create_pulldown(popup);
	obj = add_item(pd, "Show progress dialog", NORMAL_ITEM);
	set_object_callback(obj, progress_cb);
	create_pulldown(pd);
	pd = add_pulldown(win, 112, 2, "Help");
	add_item(pd, "Help index", NORMAL_ITEM);
	obj = add_item(pd, "About", NORMAL_ITEM);
	set_object_callback(obj, about_cb);
	create_pulldown(pd);

	obj = add_button(win, PIXMAP_BUTTON, 300, 20, 44, 20, "");
	set_object_image(obj, play_xpm);
	set_object_info(obj, "Info testje");
	set_object_callback(obj, button_cb);
	obj = add_button(win, NORMAL_BUTTON, 350, 20, 44, 20, "OK");
	set_object_callback(obj, all_dialogs_cb);
	set_object_color(obj, DARKGREEN, RED, YELLOW);
	set_object_info(obj, "Show all dialogs");
	win->enterobj = obj;
	obj = add_button(win, NORMAL_BUTTON, 405, 20, 40, 20, "Down");
	set_object_callback(obj, counter_cb);
	set_object_user_data(obj, 0);
	set_object_info(obj, "Decrease counter");
	obj = add_button(win, NORMAL_BUTTON, 445, 20, 40, 20, "Up");
	set_object_callback(obj, counter_cb);
	set_object_user_data(obj, 1);
	set_object_info(obj, "Increase counter");
	//obj->wait_for_mouse = FALSE;
	obj = add_button(win, NORMAL_BUTTON, guiscreen.width - 50, 20, 44, 20, "Quit");
	set_object_info(obj, "Quit");
	set_object_callback(obj, exit_cb);
	add_button(win, LIGHT_BUTTON, 246, 20, 50, 20, "Light");
	obj = add_button(win, LIGHT_BUTTON, 192, 20, 50, 20, "Light");
	set_object_color(obj, DARKGREEN, RED, WHITE);
	obj = add_button(win, NORMAL_BUTTON, 5, 20, 100, 20, "Slider Window");
	set_object_callback(obj, window_cb);
	obj = add_button(win, CHECK_BUTTON, 110, 23, 0, 0, "Auto raise");
	set_object_callback(obj, autoraise_cb);
	set_object_pressed(obj, TRUE);
	obj = add_button(win, CHECK_BUTTON, guiscreen.width - 140, 23, 0, 0, "Enable win");
	set_object_callback(obj, enablewin_cb);
	set_object_pressed(obj, TRUE);
	create_window(win);
	win_3dline(win, 2, 16, guiscreen.width - 4, 16);

	win = add_window(win_thread, NORMAL_WINDOW, 0, 50, 150, 125, "Special buttons", FALSE, FALSE);
	obj = add_button(win, CHECK_BUTTON, 5, 23, 0, 0, "Check 1");
	obj->pressed = TRUE;
	add_button(win, CHECK_BUTTON, 5, 43, 0, 0, "Check 2");
	add_button(win, CHECK_BUTTON, 5, 63, 0, 0, "Check 3");
	add_button(win, RADIO_BUTTON, 5, 83, 0, 0, "Radio 1");
	add_button(win, RADIO_BUTTON, 5, 103, 0, 0, "Radio 2");
	create_window(win);

	win = add_window(win_thread, NORMAL_WINDOW, 0, 230, 250, 100, "Plain text (always on top)", FALSE, TRUE);
	obj = add_text(win, NORMAL_TEXT, 5, 25, "The Quick Fox Jumps Over The Lazy Dog.");
	set_object_color(obj, WIN_BACK, BLACK, WHITE);
	set_object_info(obj, "With a label!");
	obj = add_text(win, NORMAL_TEXT, 5, 38, "This is some red text.");
	set_object_color(obj, WIN_BACK, BLACK, RED);
	add_text(win, NORMAL_TEXT, 5, 60, "This is normal text.");
	add_text(win, EMBOSSED_TEXT, 5, 73, "This is embossed text.");
	create_window(win);

	create_color_win(win_thread);
	create_slider_win(win_thread);


	win = add_window(win_thread, NORMAL_WINDOW, 180, 340, 150, 107, "Browser", FALSE, FALSE);
	obj = add_browser(win, 5, 21, 127, 68, TRUE);
	set_browser_text(obj, text);
	create_window(win);

	win = add_window(win_thread, NORMAL_WINDOW, 0, 340, 165, 130, "Different inputs", FALSE, FALSE);
	add_text(win, NORMAL_TEXT, 5, 25, "Normal");
	add_input(win, NORMAL_INPUT, 55, 25, 100, 256);
	add_text(win, NORMAL_TEXT, 5, 47, "Float");
	add_input(win, FLOAT_INPUT, 55, 47, 100, 256);
	add_text(win, NORMAL_TEXT, 5, 69, "Integer");
	add_input(win, INT_INPUT, 55, 69, 100, 256);
	counter = add_number(win, NORMAL_NUMBER, 15, 98, 10, "00:00");
	create_window(win);
	win_3dline(win, 3, 90, 160, 90);
	win_fillbox(win, 5, 95, 90, 30, BLACK);
	update_object(counter);

	win = add_window(win_thread, NORMAL_WINDOW, 340, 340, 250, 105, "Colored text", FALSE, FALSE);
	obj = add_text(win, NORMAL_TEXT, 5, 25, "black");
	set_object_color(obj, WIN_BACK, BLACK, BLACK);
	obj = add_text(win, NORMAL_TEXT, 5, 40, "darkgrey");
	set_object_color(obj, WIN_BACK, BLACK, DARKGREY);
	obj = add_text(win, NORMAL_TEXT, 5, 55, "grey");
	set_object_color(obj, WIN_BACK, BLACK, GREY);
	obj = add_text(win, NORMAL_TEXT, 5, 70, "lightgrey");
	set_object_color(obj, WIN_BACK, BLACK, LIGHTGREY);
	obj = add_text(win, NORMAL_TEXT, 5, 85, "white");
	set_object_color(obj, WIN_BACK, BLACK, WHITE);
	obj = add_text(win, NORMAL_TEXT, 80, 25, "darkred");
	set_object_color(obj, WIN_BACK, BLACK, DARKRED);
	obj = add_text(win, NORMAL_TEXT, 80, 40, "red");
	set_object_color(obj, WIN_BACK, BLACK, RED);
	obj = add_text(win, NORMAL_TEXT, 80, 55, "darkblue");
	set_object_color(obj, WIN_BACK, BLACK, DARKBLUE);
	obj = add_text(win, NORMAL_TEXT, 80, 70, "blue");
	set_object_color(obj, WIN_BACK, BLACK, BLUE);
	obj = add_text(win, NORMAL_TEXT, 80, 85, "lightblue");
	set_object_color(obj, WIN_BACK, BLACK, LIGHTBLUE);
	obj = add_text(win, NORMAL_TEXT, 160, 25, "darkgreen");
	set_object_color(obj, WIN_BACK, BLACK, DARKGREEN);
	obj = add_text(win, NORMAL_TEXT, 160, 40, "green");
	set_object_color(obj, WIN_BACK, BLACK, GREEN);
	obj = add_text(win, NORMAL_TEXT, 160, 55, "darkyellow");
	set_object_color(obj, WIN_BACK, BLACK, DARKYELLOW);
	obj = add_text(win, NORMAL_TEXT, 160, 70, "yellow");
	set_object_color(obj, WIN_BACK, BLACK, YELLOW);
	create_window(win);
}

/*void CreateButtonBar(GuiWinThread* win_thread)
{
	GuiObject* listbox;
	GuiWindow* win;
	GuiObject* obj = NULL;
}*/
void CreateListBox(GuiWinThread* win_thread);
GuiWindow* CreateThreadView(GuiWinThread* win_thread);
GuiWindow* CreateTerminal(GuiWinThread* win_thread);

static void init_interface(GuiWinThread* win_thread)
{


	//CreateListBox(win_thread);
	CreateButtonBar(win_thread);
	//CreateThreadView(win_thread);
	//CreateTerminal(win_thread);
	/*GuiWindow* window = create_file_dialog(win_thread, "aaaa", "aaaa");
	char directory[128], filename[128], mask[20];

	sprintf(mask, "C files|*.c|Object files|*.o|All files|*|");
	directory[0] = '\0';
	//create_file_dialog(window->win_thread, "File dialog", "Open");
	if (file_dialog(directory, filename, mask))
	{

	}*/
}



GuiWinThread* m_pWinThread;
DWORD WINAPI StartGDI(LPVOID parameter)
{
	GuiObject* obj = NULL;
	bool loop = true;
	while (loop)
	{
		obj = do_windows(m_pWinThread);
		if (obj != 0 && obj == (obj->win)->kill)
			delete_window(obj->win, TRUE);
		
		Syscall_Sleep(1);
	}
	return 0;
}

bool CreateWidget(QWORD windowId, int width, int height)
{
	int type = SVGALIB;
	init_svgagui(windowId);
	open_screen(type, width - 1, height - 1, 256, "SVGAGui");
	//마우스 이미지만 초기화
	init_mouse();
	kleur = BACKGROUND;
	m_pWinThread = create_window_thread();
	init_interface(m_pWinThread);

	ShowWindowThread(m_pWinThread);
	Syscall_CreateThread(StartGDI, "GDI", 0, 16, 0);

	return true;
}



int main(int argc, char** argv)
{
	QWORD qwWindowID;
	int iMouseX, iMouseY;
	int iWindowWidth, iWindowHeight;
	EVENT stReceivedEvent;
	EVENT tempEvent = { 0 };
	MOUSEEVENT* pstMouseEvent;
	KEYEVENT* pstKeyEvent;
	WINDOWEVENT* pstWindowEvent;

	Syscall_GetCursorPosition(&iMouseX, &iMouseY);

	iWindowWidth = 640;
	iWindowHeight = 480;

	RECT rect;
	rect.left = iMouseX - 10;
	rect.top = iMouseY - WINDOW_TITLEBAR_HEIGHT / 2;
	rect.right = rect.left + iWindowWidth;
	rect.bottom = rect.top + iWindowHeight;
	Syscall_CreateWindow(&rect, "Widget Window Example", WINDOW_FLAGS_DEFAULT | WINDOW_FLAGS_RESIZABLE, &qwWindowID);


	// 윈도우를 생성하지 못했으면 실패
	if (qwWindowID == WINDOW_INVALIDID)
	{
		return 0;
	}

	CreateWidget(qwWindowID, iWindowWidth, iWindowHeight);

	while (1)
	{

		// 이벤트 큐에서 이벤트를 수신
		if (Syscall_ReceiveEventFromWindowQueue(&qwWindowID, &stReceivedEvent) == FALSE)
		{
			Syscall_Sleep(1);
			continue;
		}

		SVGA_SetEvent(stReceivedEvent);

		// 수신된 이벤트를 타입에 따라 나누어 처리
		switch (stReceivedEvent.qwType)
		{
			// 마우스 이벤트 처리
		case EVENT_MOUSE_MOVE:
		case EVENT_MOUSE_LBUTTONDOWN:
		case EVENT_MOUSE_LBUTTONUP:
		case EVENT_MOUSE_RBUTTONDOWN:
		case EVENT_MOUSE_RBUTTONUP:
		case EVENT_MOUSE_MBUTTONDOWN:
		case EVENT_MOUSE_MBUTTONUP:
			// 여기에 마우스 이벤트 처리 코드 넣기
			pstMouseEvent = &(stReceivedEvent.stMouseEvent);

			break;

		// 키 이벤트 처리
		case EVENT_KEY_DOWN:
		case EVENT_KEY_UP:
			// 여기에 키보드 이벤트 처리 코드 넣기
			pstKeyEvent = &(stReceivedEvent.stKeyEvent);
			break;

			// 윈도우 이벤트 처리
		case EVENT_WINDOW_SELECT:
		case EVENT_WINDOW_DESELECT:
		case EVENT_WINDOW_MOVE:
		case EVENT_WINDOW_RESIZE:
		case EVENT_WINDOW_CLOSE:
			// 여기에 윈도우 이벤트 처리 코드 넣기
			pstWindowEvent = &(stReceivedEvent.stWindowEvent);

			//------------------------------------------------------------------
			// 윈도우 닫기 이벤트이면 윈도우를 삭제하고 루프를 빠져나가 태스크를 종료
			//------------------------------------------------------------------
			if (stReceivedEvent.qwType == EVENT_WINDOW_CLOSE)
			{
				// 윈도우 삭제
				Syscall_DeleteWindow(&qwWindowID);
				return 1;
			}
			break;

			// 그 외 정보
		default:
			// 여기에 알 수 없는 이벤트 처리 코드 넣기
			break;
		}

	}

	return 0;
}