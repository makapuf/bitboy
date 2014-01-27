/*
 * MinGW32 system file
 * based on nix.c and dos.c
 * req's SDL
 * -Dave Kiddell
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gnuboy.h"

#include "rc.h"

/*
** Timer functions should be picked up from sdl.c (via GNUBOY_USE_SDL_TIMERS)
**  void *sys_timer();
**  int sys_elapsed(Uint32 *cl);
**  void sys_sleep(int us);
*/

void sys_sanitize(char *s)
{
    /*
    ** used to be used to convert Windows style path\file into
    ** unix style path/file as gnuboy code then assumed unix
    ** Now a NOOP
    */
}

void sys_initpath(char *exe)
{
	char *buf, *home, *p;

	home = strdup(exe);
	sys_sanitize(home);
	p = strrchr(home, DIRSEP_CHAR);
	if (p) *p = 0;
	else
	{
		buf = ".";
		rc_setvar("rcpath", 1, &buf);
		rc_setvar("savedir", 1, &buf);
		return;
	}
	buf = malloc(strlen(home) + 8);
	sprintf(buf, ".;%s%s", home, DIRSEP);
	rc_setvar("rcpath", 1, &buf);
	sprintf(buf, ".", home);
	rc_setvar("savedir", 1, &buf);
	free(buf);
}

void sys_checkdir(char *path, int wr)
{
	(void) path; /* avoid warning about unused parameter */
	(void) wr; /* avoid warning about unused parameter */
}

