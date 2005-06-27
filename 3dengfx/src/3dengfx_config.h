#ifndef _3DENGFX_CONFIG_H_
#define _3DENGFX_CONFIG_H_

/* -- CAUTION --
 * this header is included in both C (89) and C++ source files
 * so if you want to comment-out some option, use C comments
 * not C++ // comments
 */

/* DON'T remove this definition */
#define USING_3DENGFX

#define VER_STR		"0.5"

/* installation path prefix */
#define PREFIX	"/usr/local"

/* underlying graphics support library selection. */
#define SDL					1
#define GLUT				2
#define GTK					3
#define GTKMM				4
#define NATIVE				5
#define NATIVE_X11			10
#define NATIVE_WIN32		11

/* set this to any of the above, NATIVE selects X11/WIN32 automatically */
#define GFX_LIBRARY		NATIVE

/* define this to use single precision floating point scalars
 * throughout the 3dengine code.
 * WARING: the *double* precision mode doesn't get too much testing,
 * it should work, but it might not, so better keep it defined.
 */
#define SINGLE_PRECISION_MATH

/* define this to diable png load/save support */
/* #define IMGLIB_NO_PNG */

/* define this to disable jpeg load/save support */
/* #define IMGLIB_NO_JPEG */


#if GFX_LIBRARY == NATIVE

#if defined(__unix__)
#define NATIVE_LIB		NATIVE_X11

#elif defined(WIN32)
#define NATIVE_LIB		NATIVE_WIN32

#endif	/* unix/win32 */

#endif	/* GFX_LIBRARY == NATIVE */

#endif	/* _3DENGFX_CONFIG_H_ */
