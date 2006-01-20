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

long CALLBACK fxwt_win32_handle_event(HWND__ *win, unsigned int msg, unsigned int wparam, long lparam);
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
	SwapBuffers(fxwt_win32_dc);
	Sleep(0);
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

long CALLBACK fxwt_win32_handle_event(HWND__ *win, unsigned int msg, unsigned int wparam, long lparam) {
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

	case WM_CHAR:
		{
			list<void (*)(int)>::iterator iter = keyb_handlers.begin();
			while(iter != keyb_handlers.end()) {
				(*iter++)(wparam);
			}
		}
		break;

	case WM_MOUSEMOVE:
		{
			list<void (*)(int, int)>::iterator iter = motion_handlers.begin();
			while(iter != motion_handlers.end()) {
				(*iter++)(LOWORD(lparam), HIWORD(lparam));
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
		break;
	//	DefWindowProc(win, msg, wparam, lparam);
	}

	return DefWindowProc(win, msg, wparam, lparam);
}

static void button_event(int bn, bool state, int x, int y) {
	button_state[bn] = state;
	list<void (*)(int, int, int, int)>::iterator iter = button_handlers.begin();
	while(iter != button_handlers.end()) {
		(*iter++)(bn, state, x, y);
	}
}

static int vkey_to_keysym(unsigned int vkey) {
	//return 0xff00 | (vkey & 0xff);
	//return vkey;
	
	// just make sure we dont return any chars. WM_CHAR
	// will take care of that
	if ( (vkey>=0x30 && vkey<=0x39) || (vkey>=0x41 && vkey<=0x5A) )
		return 100000;
	
	/* Arrows + Home/End pad */
	if (vkey == 37) return KEY_LEFT;
	if (vkey == 38) return KEY_UP;
	if (vkey == 39) return KEY_RIGHT;
	if (vkey == 40) return KEY_DOWN;	
	if (vkey == 36) return KEY_HOME;
	if (vkey == 35) return KEY_END;
	if (vkey == 33) return KEY_PAGEUP;
	if (vkey == 34) return KEY_PAGEDOWN;
	
	/* Numeric keypad  - numbers*/
	if (vkey>=96 && vkey<=105)
		return vkey + 160;

	/* Numeric keypad - symbols */
	if (vkey == 111) return KEY_KP_DIVIDE;
	if (vkey == 106) return KEY_KP_MULTIPLY;
	if (vkey == 109) return KEY_KP_MINUS;
	if (vkey == 107) return KEY_KP_PLUS;
	if (vkey == 144) return KEY_NUMLOCK;
		
	/* Function keys */
	if (vkey>=112 && vkey<=126)
		return vkey + 170;
	
	/* Key state modifier keys */
	if (vkey == 20)	return KEY_CAPSLOCK;
	if (vkey == 145) return KEY_SCROLLOCK;
	if (vkey == 16) return KEY_LSHIFT;
	if (vkey == 17) return KEY_LCTRL;
	// if (vkey == ???) return KEY_LALT;
	if (vkey == 91) return KEY_LSUPER;
	if (vkey == 92) return KEY_RSUPER;

	if (vkey == 8) return KEY_BACKSPACE;
	if (vkey == 9) return KEY_TAB;
	if (vkey == 12) return KEY_CLEAR;
	if (vkey == 13) return KEY_RETURN;
	if (vkey == 19) return KEY_PAUSE;
	if (vkey == 27) return KEY_ESCAPE;
	if (vkey == 128) return KEY_DELETE;
	
	// return meaningless key
	return 100000;
}

#endif	// GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_WIN32
