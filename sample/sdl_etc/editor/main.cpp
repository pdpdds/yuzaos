/*
** SDL Code Edit
**
** Copyright (C) 2018 Wang Renxin
**
** A code edit widget in plain SDL.
**
** For the latest info, see https://github.com/paladin-t/sdl_code_edit/
*/

#ifdef _MSC_VER
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif /* _CRT_SECURE_NO_WARNINGS */
#endif /* _MSC_VER */

#define NOMINMAX
#include "code_edit.h"
#include <SDL.h>
#include "SDL2_gfxPrimitives.h"
//#include <sstream>
#include <ostringstream>

#ifndef WINDOW_WIDTH
#	define WINDOW_WIDTH 640
#endif /* WINDOW_WIDTH */
#ifndef WINDOW_HEIGHT
#	define WINDOW_HEIGHT 480
#endif /* WINDOW_HEIGHT */

#ifndef WIDGET_BORDER_X
#	define WIDGET_BORDER_X 8
#endif /* WIDGET_BORDER_X */
#ifndef WIDGET_BORDER_Y
#	define WIDGET_BORDER_Y 20
#endif /* WIDGET_BORDER_Y */

#ifndef SCROLL_BAR_SIZE
#	define SCROLL_BAR_SIZE 8
#endif /* SCROLL_BAR_SIZE */

template<typename T> T clamp(T v, T lo, T hi) {
	assert(lo <= hi);
	if (v < lo) v = lo;
	if (v > hi) v = hi;

	return v;
}

template<typename T> bool intersects(T px, T py, T x0, T y0, T x1, T y1) {
	return (px > eastl::min(x0, x1) && px < eastl::max(x0, x1)) && (py > eastl::min(y0, y1) && py < eastl::max(y0, y1));
}

/*static void openTerminal(void) {
	long hConHandle;
	HANDLE lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE* fp = nullptr;

	::AllocConsole();

	::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	coninfo.dwSize.Y = 500;
	::SetConsoleScreenBufferSize(::GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	lStdHandle = ::GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
	fp = _fdopen((intptr_t)hConHandle, "w");
	*stdout = *fp;
	setvbuf(stdout, nullptr, _IONBF, 0);

	lStdHandle = ::GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
	fp = _fdopen((intptr_t)hConHandle, "r");
	*stdin = *fp;
	setvbuf(stdin, nullptr, _IONBF, 0);

	lStdHandle = ::GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
	fp = _fdopen((intptr_t)hConHandle, "w");
	*stderr = *fp;
	setvbuf(stderr, nullptr, _IONBF, 0);

	std::ios::sync_with_stdio();

	freopen("CON", "w", stdout);
	freopen("CON", "r", stdin);
	freopen("CON", "w", stderr);
}*/

struct CodeEditAdapter : public CodeEdit {
private:
	struct ScrollBar {
	private:
		CodeEdit::Vec2 _point0;
		CodeEdit::Vec2 _point1;

		bool _mouseDown = false;
		float _mouseDownOffset = 0.0f;
		CodeEdit::Vec2 _mouseDownPos;

	public:
		const CodeEdit::Vec2 &point0(void) const { return _point0; }
		void point0(const CodeEdit::Vec2 &val) { _point0 = val; }
		const CodeEdit::Vec2 &point1(void) const { return _point1; }
		void point1(const CodeEdit::Vec2 &val) { _point1 = val; }

		const CodeEdit::Vec2 &getMouseDownPos(void) const { return _mouseDownPos; }
		float getMouseDownOffset(void) const { return _mouseDownOffset; }
		bool isMouseDown(void) const { return _mouseDown; }
		void releaseMouseDown(CodeEditAdapter* edit) {
			_mouseDown = false;
			edit->setActive(0);
		}
		void checkMouseDown(CodeEditAdapter* edit, const CodeEdit::Vec2 &pos, float offset) {
			if (_mouseDown)
				return;
			if (edit->getActive())
				return;
			if (intersects(pos.x, pos.y, _point0.x, _point0.y, _point1.x, _point1.y)) {
				_mouseDownOffset = offset;
				_mouseDownPos = CodeEdit::Vec2(pos.x, pos.y);
				_mouseDown = true;
				edit->setActive((intptr_t)this);
			}
		}
	};

private:
	eastl::string _cache;

	SDL_Renderer* _renderer = nullptr;

	int _width = 0, _height = 0;
	intptr_t _active = 0;

	ScrollBar _horizontalScrollBar, _verticalScrollBar;

	int _mouseClickedCount = 0;

public:
	CodeEditAdapter() {
	}
	virtual ~CodeEditAdapter() override {
	}

	void initialize(SDL_Renderer* rnd) {
		_renderer = rnd;
	}

	void text(const char* txt, size_t len) {
		setText(txt);
	}
	const char* text(size_t* len) {
		_cache = getText();
		if (len)
			*len = _cache.length();

		return _cache.c_str();
	}

	int width(void) const {
		return _width;
	}
	int height(void) const {
		return _height;
	}

	intptr_t getActive(void) const {
		return _active;
	}
	void setActive(intptr_t val) {
		_active = val;
	}

	void onEvent(SDL_Event* evt) {
		switch (evt->type) {
		case SDL_WINDOWEVENT: {
				switch (evt->window.event) {
				case SDL_WINDOWEVENT_SHOWN:
				case SDL_WINDOWEVENT_RESIZED: {
						setWidgetPos(CodeEdit::Vec2(WIDGET_BORDER_X, WIDGET_BORDER_Y));

						SDL_GetRendererOutputSize(_renderer, &_width, &_height);
						setWidgetSize(
							CodeEdit::Vec2(
								(float)_width - WIDGET_BORDER_X * 2 - 2 - SCROLL_BAR_SIZE,
								(float)_height - WIDGET_BORDER_Y * 2 - 2 - SCROLL_BAR_SIZE
							)
						);
					}
					break;
				}
			}
			break;
		case SDL_TEXTINPUT: {
				addInputCharactersUtf8(evt->text.text);
			}
			break;
		case SDL_MOUSEBUTTONUP: {
				_mouseClickedCount = evt->button.clicks;
			}
			break;
		case SDL_MOUSEWHEEL: {
				const CodeEdit::Vec2 &wndSize = getWidgetSize();
				const CodeEdit::Vec2 &contentSize = getContentSize();
				float y = getScrollY();
				if (evt->wheel.y < 0)
					y += 32.0f;
				else if (evt->wheel.y > 0)
					y -= 32.0f;
				y = clamp(y, 0.0f, contentSize.y <= wndSize.y ? 0.0f : contentSize.y - wndSize.y);
				setScrollY(y);
			}
			break;
		default:
			break;
		}
	}

	void render(void) {
		setFrameCount(getFrameCount() + 1);

		updateKeyStates();
		updateMouseStates(_mouseClickedCount, nullptr, nullptr);
		_mouseClickedCount = 0;

		CodeEdit::render(_renderer);

		handleKeys();

		horizontalScrollBar();
		verticalScrollBar();
	}

private:
	void handleKeys(void) {
		// Marks all changes saved when pressed Ctrl+S. Demo code rather than saving anything on the disk.
		if (isKeyCtrlDown() && isKeyDown(SDLK_s))
			setChangesSaved();
	}

	void horizontalScrollBar(void) {
		scrollBar(
			_horizontalScrollBar, 0,
			[&] (void) -> float { return getScrollX(); },
			[&] (float val) -> void { setScrollX(val); }
		);
	}
	void verticalScrollBar(void) {
		scrollBar(
			_verticalScrollBar, 1,
			[&] (void) -> float { return getScrollY(); },
			[&] (float val) -> void { setScrollY(val); }
		);
	}

	void scrollBar(ScrollBar &bar, size_t comp, eastl::function<float (void)> getScroll, eastl::function<void (float)> setScroll) {
		assert(comp == 0 || comp == 1);

		const CodeEdit::Vec2 &wndPos = getWidgetPos();
		const CodeEdit::Vec2 &wndSize = getWidgetSize();
		const CodeEdit::Vec2 &contentSize = getContentSize();

		const float barSize = eastl::max(eastl::min((wndSize[comp] / contentSize[comp]) * wndSize[comp], wndSize[comp]), SCROLL_BAR_SIZE * 2.0f);
		const float percent = clamp(getScroll() / (contentSize[comp] - wndSize[comp]), 0.0f, 1.0f);
		const float slide = wndSize[comp] - barSize;
		const float offset = slide * percent;

		if (comp == 0) {
			bar.point0(CodeEdit::Vec2(wndPos.x + offset, wndPos.y + wndSize.y + 1));
			bar.point1(CodeEdit::Vec2(wndPos.x + offset + barSize, wndPos.y + wndSize.y + SCROLL_BAR_SIZE));
		} else {
			bar.point0(CodeEdit::Vec2(wndPos.x + wndSize.x + 1, wndPos.y + offset));
			bar.point1(CodeEdit::Vec2(wndPos.x + wndSize.x + SCROLL_BAR_SIZE, wndPos.y + offset + barSize));
		}

		roundedBoxColor(
			_renderer,
			(Sint16)bar.point0().x, (Sint16)bar.point0().y,
			(Sint16)bar.point1().x, (Sint16)bar.point1().y,
			3, bar.isMouseDown() ? 0xff9e9e9e : 0xff686868
		);

		if (isMouseDown()) {
			const CodeEdit::Vec2 &pos = getMousePos();
			if (!bar.isMouseDown()) {
				bar.checkMouseDown(this, pos, offset);
				if (!bar.isMouseDown()) {
					bool force = false;
					if (comp == 0) {
						force = intersects(
							pos.x, pos.y,
							wndPos.x, wndPos.y + wndSize.y,
							wndPos.x + wndSize.x, wndPos.y + wndSize.y + SCROLL_BAR_SIZE
						);
					} else {
						force = intersects(
							pos.x, pos.y,
							wndPos.x + wndSize.x, wndPos.y,
							wndPos.x + wndSize.x + SCROLL_BAR_SIZE, wndPos.y + wndSize.y
						);
					}
					if (force) {
						const float p = (pos[comp] - wndPos[comp] - barSize / 2.0f) / wndSize[comp] * contentSize[comp];
						setScroll(p);
					}
				}
			} else {
				setWidgetHoverable(false);
				const CodeEdit::Vec2 &downPos =bar.getMouseDownPos();
				const float diff = pos[comp] - downPos[comp];
				const float newOffset = bar.getMouseDownOffset() + diff;
				const float newPercent = clamp(newOffset / slide, 0.0f, 1.0f);
				if (slide != 0.0f)
					setScroll(newPercent * (contentSize[comp] - wndSize[comp]));
			}
		} else {
			if (bar.isMouseDown()) {
				bar.releaseMouseDown(this);
				setWidgetHoverable(true);
				setWidgetFocused(true);
			}
		}
	}
};
#include <IOStream>
int main(int argc, char** argv)
{
	// Sets up debug helpers.
#if defined _DEBUG || defined DEBUG
	_CrtSetBreakAlloc(0);
	atexit(
		[] () {
			if (!!_CrtDumpMemoryLeaks()) {
				fprintf(stderr, "Memory leak!\n");

				_CrtDbgBreak();
			}
		}
	);

	openTerminal();
#endif /* Debug. */

	// Prepares.
	SDL_Window* wnd = SDL_CreateWindow(
		"SDL Code Edit",
		SDL_WINDOWPOS_CENTERED_DISPLAY(0), SDL_WINDOWPOS_CENTERED_DISPLAY(0),
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN
	);
	SDL_SetWindowMinimumSize(wnd, WINDOW_WIDTH, WINDOW_HEIGHT);
	SDL_Renderer* rnd = SDL_GetRenderer(wnd);
	if (!rnd)
		rnd = SDL_CreateRenderer(wnd, -1, 0);
	SDL_Cursor* cursorArrow = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	SDL_Cursor* cursorInput = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	SDL_ShowCursor(1);

	CodeEditAdapter* edit = new CodeEditAdapter();
	edit->initialize(rnd);
	edit->setMouseCursorChangedHandler(
		[&] (bool input) {
			// Toggles between input and arrow cursors.
			SDL_SetCursor(input ? cursorInput : cursorArrow);
		}
	);
	edit->text(
		"#include <stdio.h>\n"
		"\n"
		"int main() {\n"
		"	printf(\"hello world\\n\");\n"
		"\n"
		"	return 0;\n"
		"}\n",
		0
	);

	// Loops.
	Uint64 timestamp = 0;
	SDL_Event e;
	bool done = false;
	while (!done) {
		// Processes events.
		while (SDL_PollEvent(&e)) {
			switch (e.type) {
			case SDL_QUIT: {
					done = true;
				}
				break;
			default:
				break;
			}
			edit->onEvent(&e);
		}

		// Renders.
		SDL_SetRenderDrawColor(rnd, 0x2e, 0x32, 0x38, 0xff);
		SDL_RenderClear(rnd);

		boxColor(
			rnd,
			WIDGET_BORDER_X, WIDGET_BORDER_Y,
			// Reserves space for scroll bars.
			edit->width() - WIDGET_BORDER_X - 2 - SCROLL_BAR_SIZE, edit->height() - WIDGET_BORDER_Y - 2 - SCROLL_BAR_SIZE,
			0xff2c2c2c
		);
		edit->render(); // Renders the widget and processes events.
		stringColor(rnd, WIDGET_BORDER_X, (WIDGET_BORDER_Y - 8) / 2, "Syntax highlighting code edit widget", 0xffffffff);
		CodeEdit::Coordinates cp = edit->getCursorPosition();
		
		eastl::ostringstream ss;
		ss << "Ln " << cp.line + 1 << ", Col " << cp.column + 1;
		stringColor(rnd, WIDGET_BORDER_X, edit->height() - WIDGET_BORDER_Y + (WIDGET_BORDER_Y - 8) / 2, ss.str().c_str(), 0xffffffff);


		const Uint64 now = SDL_GetPerformanceCounter();
		if (timestamp == 0)
			timestamp = now;
		const Uint64 diff = now - timestamp;
		const double ddiff = (double)diff / SDL_GetPerformanceFrequency();
		const double rest = 1.0 / 60.0 - ddiff; // 60 FPS.
		timestamp = now;
		if (rest > 0)
			SDL_Delay((Uint32)(rest * 1000));

		SDL_RenderPresent(rnd);
	}

	// Finishes.
	delete edit;

	//primitivePurge();
	if (cursorInput)
		SDL_FreeCursor(cursorInput);
	if (cursorArrow)
		SDL_FreeCursor(cursorArrow);
	if (rnd)
		SDL_DestroyRenderer(rnd);
	if (wnd)
		SDL_DestroyWindow(wnd);

	return 0;
}
