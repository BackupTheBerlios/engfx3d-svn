/*
This file is part of the 3dengfx, realtime visualization system.

Copyright (C) 2005 John Tsiombikas <nuclear@siggraph.org>

3dengfx is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

3dengfx is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with 3dengfx; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* OpenGL through Win32
 *
 * Author: John Tsiombikas 2005
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_WIN32

#include <stdlib.h>
#include "init.hpp"
#include "gfx_library.h"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

HWND__ *fxwt_win32_win;
HDC__ *fxwt_win32_dc;
static HGLRC__ *wgl_ctx;

bool fxwt::init_graphics(GraphicsInitParameters *gparams) {
	HWND__ *win;
	HDC__ *dc;

	info("Initializing WGL");
	HINSTANCE pid = GetModuleHandle(0);

	WNDCLASSEX wc;
	memset(&wc, 0, sizeof wc);
	wc.cbSize = sizeof wc;
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hIcon = wc.hIconSm = LoadIcon(0, IDI_APPLICATION);
	wc.hInstance = pid;
	wc.lpfnWndProc = HandleEvents;
	wc.lpszClassName = "win32_sucks_big_time";
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	RegisterClassEx(&wc);

	unsigned long style = WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	win = CreateWindow("win32_sucks_big_time", "3dengfx/win32", style, 0, 0, iparams->x, iparams->y, 0, 0, pid, 0);
	dc = GetDC(win);

	// determine color bits
	int color_bits;
	if(gparams->dont_care_flags & DONT_CARE_BPP) {
		color_bits = 1;
	} else {
		switch(gparams->bpp) {
		case 32:
		case 24:
			color_bits = 8;
			break;

		case 16:
		case 15:
			color_bits = 5;
			break;

		case 12:
			color_bits = 4;
			break;

		default:
			error("%s: Tried to set unsupported pixel format: %d bpp", __func__, gparams->bpp);
		}
	}

	// determine stencil bits
	int stencil_bits = gparams->stencil_bits;
	if(gparams->dont_care_flags & DONT_CARE_STENCIL) {
		stencil_bits = 1;
	}

	// determine zbuffer bits
	int zbits = gparams->depth_bits == 32 ? 24 : gparams->depth_bits;

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof pfd);
	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = gparams->bpp;
	pfd.cDepthBits = depth_bits;
	pfd.cStencilBits = stencil_bits;
	pfd.iLayerType = PFD_MAIN_PLANE;

	info("Trying to set video mode %dx%dx%d, d:%d s:%d %s", gparams->x, gparams->y, gparams->bpp, gparams->depth_bits, gparams->stencil_bits, gparams->fullscreen ? "fullscreen" : "windowed");

	int pix_format = ChoosePixelFormat(dc, &pfd);
	if(!pix_format) {
		error("ChoosePixelFormat() failed");
		ReleaseDC(win, dc);
		DestroyWindow(win);
		return false;
	}

	//TODO: examine if the pixelformat is correct

	if(!SetPixelFormat(dc, pix_format, &pfd)) {
		error("SetPixelFormat() failed");
		ReleaseDC(win, dc);
		DestroyWindow(win);
		return false;
	}

	if(!(wgl_ctx = wglCreateContext(dc))) {
		error("wglCreateContext() failed");
		ReleaseDC(win, dc);
		DestroyWindow(win);
		return false;
	}

	if(wglMakeCurrent(dc, wgl_ctx) == FALSE) {
        error("wglMakeCurrent() failed");
		wglDeleteContext(wgl_ctx);
		ReleaseDC(win, dc);
		DestroyWindow(win);
		return false;
	}

	fxwt_win32_dc = dc;
	fxwt_win32_win = win;

	return true;
}

void fxwt::destroy_graphics() {
	info("Shutting down WGL");
	wglMakeCurrent(0, 0);
	glDeleteContext(wgl_ctx);
	ReleaseDC(win, dc);
	DestroyWindow(win);
}

#endif	// GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_WIN32
