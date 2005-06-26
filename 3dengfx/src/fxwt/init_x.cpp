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

/* OpenGL through GLX (X Window System)
 *
 * Author: John Tsiombikas 2005
 */

#include "3dengfx_config.h"

#if GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_X11

#include <stdlib.h>
#include "init.hpp"
#include "gfx_library.h"
#include "3dengfx/3denginefx.hpp"
#include "common/err_msg.h"

Display *fxwt_x_dpy;
Window fxwt_x_win;
static GLXContext glx_ctx;

bool fxwt::init_graphics(GraphicsInitParameters *gparams) {
	Display *dpy;
	Window win;
	info("Initializing GLX");

	if(!(dpy = XOpenDisplay(0))) {
		error("Could not connect to the X server");
		return false;
	}

	int screen = DefaultScreen(dpy);
	Window root_win = RootWindow(dpy, screen);

	info("Trying to set video mode %dx%dx%d, d:%d s:%d %s", gparams->x, gparams->y, gparams->bpp, gparams->depth_bits, gparams->stencil_bits, gparams->fullscreen ? "fullscreen" : "windowed");
	
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
	
	int glx_attrib[] = {
		GLX_RGBA, GLX_DOUBLEBUFFER,
		GLX_RED_SIZE, color_bits,
		GLX_GREEN_SIZE, color_bits,
		GLX_BLUE_SIZE, color_bits,
		GLX_DEPTH_SIZE, zbits,
		GLX_STENCIL_SIZE, stencil_bits,
		None
	};

	XVisualInfo *vis_info;
	if(!(vis_info = glXChooseVisual(dpy, screen, glx_attrib))) {
		error("%s: Could not set requested video mode", __func__);
		XCloseDisplay(dpy);
		return false;
	}

	// check the video mode we got
	int arbits, agbits, abbits, azbits, astencilbits;
	glXGetConfig(dpy, vis_info, GLX_RED_SIZE, &arbits);
	glXGetConfig(dpy, vis_info, GLX_GREEN_SIZE, &agbits);
	glXGetConfig(dpy, vis_info, GLX_BLUE_SIZE, &abbits);
	glXGetConfig(dpy, vis_info, GLX_DEPTH_SIZE, &azbits);
	glXGetConfig(dpy, vis_info, GLX_STENCIL_SIZE, &astencilbits);

	info("Initialized video mode:");
	info("    bpp: %d (%d%d%d)", arbits + agbits + abbits, arbits, agbits, abbits);
	info("zbuffer: %d", azbits);
	info("stencil: %d", astencilbits);

	/* if the dont_care_flags does not contain DONT_CARE_BPP and our color bits
	 * does not match, we should return failure, however we test against
	 * the difference allowing a +/-1 difference in order to allow for 16bpp
	 * formats of either 565 or 555 and consider them equal.
	 */
	if(!(gparams->dont_care_flags & DONT_CARE_BPP) && abs(arbits - color_bits) > 1 && abs(agbits - color_bits) > 1 && abs(abbits - color_bits) > 1) {
		error("%s: Could not set requested exact bpp mode", __func__);
		XFree(vis_info);
		XCloseDisplay(dpy);
		return false;
	}

	// now if we don't have DONT_CARE_DEPTH in the dont_care_flags check for 
	// exact depth buffer format, however consider 24 and 32 bit the same
	if(!(gparams->dont_care_flags & DONT_CARE_DEPTH) && azbits != zbits) {
		if(!(zbits == 32 && azbits == 24 || zbits == 24 && azbits == 32)) {
			error("%s: Could not set requested exact zbuffer depth", __func__);
			XFree(vis_info);
			XCloseDisplay(dpy);
			return false;
		}
	}

	// if we don't have DONT_CARE_STENCIL make sure we have the stencil format
	// that was asked.
	if(!(gparams->dont_care_flags & DONT_CARE_STENCIL) && astencilbits != gparams->stencil_bits) {
		error("%s: Could not set exact stencil format", __func__);
		XFree(vis_info);
		XCloseDisplay(dpy);
		return false;
	}

	// everything is ok, create the context
	if(!(glx_ctx = glXCreateContext(dpy, vis_info, 0, True))) {
		error("%s: Failed to create GLX context", __func__);
		XFree(vis_info);
		XCloseDisplay(dpy);
		return false;
	}

	XSetWindowAttributes xattr;
	xattr.background_pixel = xattr.border_pixel = BlackPixel(dpy, screen);
	xattr.colormap = XCreateColormap(dpy, root_win, vis_info->visual, AllocNone);

	if(gparams->fullscreen) {
		/*
		XF86VidModeModeInfo **modes;
		XF86VidModeModeInfo *vid_mode = 0;
		int mode_count;
		XF86VidModeGetAllModeLines(dpy, screen, &mode_count, &modes);
		orig_mode = *modes[0];

		for(int i=0; i<mode_count; i++) {
			if(modes[i]->hdisplay == iparams->x && modes[i]->vdisplay == iparams->y) {
				vid_mode = modes[i];
			}
		}
		if(!vid_mode) {
			error("Could not set requested video mode");
			XFree(modes);
			XFree(vis_info);
			XCloseDisplay(dpy);
			return -1;
		}
		
		XF86VidModeSwitchToMode(dpy, screen, vid_mode);
		XF86VidModeSetViewPort(dpy, screen, 0, 0);
		XFree(modes);

		xattr.override_redirect = True;
		win = XCreateWindow(dpy, root_win, 0, 0, iparams->x, iparams->y, 0, vis_info->depth, InputOutput, vis_info->visual, CWColormap | CWOverrideRedirect, &xattr);

		XWarpPointer(dpy, None, win, 0, 0, 0, 0, 0, 0);
		XMapRaised(dpy, win);
        XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        XGrabPointer(dpy, win, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, win, None, CurrentTime);
		
		fullscreen = true;
		*/
	} else {
		win = XCreateWindow(dpy, root_win, 0, 0, gparams->x, gparams->y, 0, vis_info->depth,
				InputOutput, vis_info->visual, CWColormap | CWBackPixel | CWBorderPixel, &xattr);
	}
	long events = ExposureMask | StructureNotifyMask | KeyPressMask;	// expose and key events
	events |= ButtonPressMask | ButtonReleaseMask | PointerMotionMask;	// mouse events
	XSelectInput(dpy, win, events);

	// set WM cooperation settings
	Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(dpy, win, &wm_delete, 1);

	XTextProperty tp_wname;
	static char *win_title = "3dengfx/X";
	XStringListToTextProperty(&win_title, 1, &tp_wname);
	XSetWMName(dpy, win, &tp_wname);
	XFree(tp_wname.value);

	XClassHint class_hint;
	class_hint.res_name = "3dengfx";
	class_hint.res_class = "3dengfx_graphics";
	XSetClassHint(dpy, win, &class_hint);

	XFree(vis_info);

	if(glXMakeCurrent(dpy, win, glx_ctx) == False) {
		error("%s: Failed to make the GLX context current", __func__);
		glXDestroyContext(dpy, glx_ctx);
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
		return false;
	}

	if(!glXIsDirect(dpy, glx_ctx)) {
		warning("using indirect rendering, which might be slow...");
	}

	XMapWindow(dpy, win);
	XFlush(dpy);

	fxwt_x_dpy = dpy;
	fxwt_x_win = win;
	
	return true;
}

void fxwt::destroy_graphics() {
	info("Shutting down GLX");
	glXMakeCurrent(fxwt_x_dpy, None, 0);
	glXDestroyContext(fxwt_x_dpy, glx_ctx);
	XDestroyWindow(fxwt_x_dpy, fxwt_x_win);

	/*
	if(fullscreen) {
		XF86VidModeSwitchToMode(dpy, DefaultScreen(dpy), &orig_mode);
		XF86VidModeSetViewPort(dpy, DefaultScreen(dpy), 0, 0);
	}
	*/
	
	XCloseDisplay(fxwt_x_dpy);
}

#ifndef GLX_ARB_get_proc_address
void *glXGetProcAddress(const char *name) {
	char *err_str;
	void *sym;
	void *so = dlopen("libGL.so", RTLD_LAZY);
	if(!so) {
		perror("dlopen failed");
		return 0;
	}
	
	dlerror();
	sym = dlsym(so, name);
	if((err_str = dlerror())) {
		fprintf(stderr, "dlsym failed: %s\n", err_str);
		sym = 0;
	}
	
	dlclose(so);
	return sym;
}
#endif

#endif	// GFX_LIBRARY == NATIVE && NATIVE_LIB == NATIVE_X11
