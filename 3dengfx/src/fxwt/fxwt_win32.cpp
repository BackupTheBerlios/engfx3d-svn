/*
This file is part of fxwt, the window system toolkit of 3dengfx.

Copyright (c) 2004, 2005 John Tsiombikas <nuclear@siggraph.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* main fxwt event handling and system abstraction.
 *
 * Author: John Tsiombikas 2004
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_WIN32

#include "gfx_library.h"
#include "fxwt.hpp"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

using std::list;
using namespace fxwt;

static long CALLBACK handle_event(HWND__ *win, unsigned int msg, unsigned int wparam, long lparam);
static void button_event(int bn, bool state, int x, int y);
static int vkey_to_keysym(unsigned int vkey);

extern HWND__ *fxwt_win32_win;
extern HDC__ *fxwt_win32_dc;

Vector2 fxwt::get_mouse_pos_normalized() {
	return Vector2(0, 0);
}

void fxwt::set_window_title(const char *title) {
	SetWindowText(fxwt_win32_win, title);
}

void fxwt::swap_buffers() {
	SwapBuffers(fxwt_win32_hdc);
}

int fxwt::main_loop() {
	set_verbosity(3);

	while(1) {
		if(!idle_handlers.empty()) {
			MSG event;
			while(PeekMessage(&event, 0, 0, 0, PM_REMOVE)) {
				TranslateMessage(&event);
				DispatchMessage(&event);
			}

			list<void (*)()>::iterator iter = idle_handlers.begin();
			while(iter != idle_handlers.end()) {
				(*iter++)();
			}
		} else {
			MSG event;
			if(!GetMessage(&event, 0, 0, 0)) break;
			TranslateMessage(&event);
			DispatchMessage(&event);
		}
	}

	return 0;
}

static long CALLBACK handle_event(HWND__ *win, unsigned int msg, unsigned int wparam, long lparam) {
	static int window_mapped;

	switch(msg) {
	case WM_SHOWWINDOW:
		if(wparam) {
			window_mapped = 1;
		} else {
            window_mapped = 0;
		}
		break;

	case WM_PAINT:
		if(window_mapped) {
			list<void (*)()>::iterator iter = disp_handlers.begin();
			while(iter != disp_handlers.end()) {
				(*iter++)();
			}
		}
		break;

	case WM_CLOSE:
		exit(0);

	case WM_KEYDOWN:
		{
			list<void (*)(int)>::iterator iter = keyb_handlers.begin();
			while(iter != keyb_handlers.end()) {
				(*iter++)(vkey_to_keysym(wparam));
			}
		}
		break;

	case WM_MOUSEMOVE:
		{
			list<void (*)(int, int)>::iterator iter = motion_handlers.begin();
			while(iter != motion_handlers.end()) {
				(*iter++)(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
			}
		}
		break;

	case WM_MOUSEWHEEL:
		{
			int bn = HIWORD(wparam) > 0 ? BN_WHEELUP : BN_WHEELDOWN;
			button_event(bn, true, LOWORD(lparam), HIWORD(lparam));
		}
		break;

	case WM_LBUTTONDOWN:
		button_event(BN_LEFT, true, LOWORD(lparam), HIWORD(lparam));
		break;
		
	case WM_MBUTTONDOWN:
		button_event(BN_MIDDLE, true, LOWORD(lparam), HIWORD(lparam));
		break;
		
	case WM_RBUTTONDOWN:
		button_event(BN_RIGHT, true, LOWORD(lparam), HIWORD(lparam));
		break;

	case WM_LBUTTONUP:
		button_event(BN_LEFT, false, LOWORD(lparam), HIWORD(lparam));
		break;
		
	case WM_MBUTTONUP:
		button_event(BN_MIDDLE, false, LOWORD(lparam), HIWORD(lparam));
		break;
		
	case WM_RBUTTONUP:
		button_event(BN_RIGHT, false, LOWORD(lparam), HIWORD(lparam));
		break;


	default:
		DefWindowProc(win, msg, wparam, lparam);
	}

	return 0;
}

static void button_event(int bn, bool state, int x, int y) {
	button_state[bn] = state;
	list<void (*)(int, int, int, int)>::iterator iter = button_handlers.begin();
	while(iter != button_handlers.end()) {
		(*iter++)(bn, state, x, y);
	}
}

static int vkey_to_keysym(unsigned int vkey) {
	return 0xff << 8 | (vkey & 0xff);
}

#endif	// GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_WIN32
