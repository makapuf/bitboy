/*
 * Dingoo A320 SDL for Native system file
 * based on Windows port
 * requires SDL
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gnuboy.h"
#include "rc.h"


void sys_sanitize(char *s)
{
    /*
    ** used to be used to convert Windows style path\file into
    ** unix style path/file as gnuboy code then assumed unix
    ** Now a NOOP
    */
    /*
	int i;
	for (i = 0; s[i]; i++)
		if (s[i] == '\\') s[i] = '/';
*/
}

void sys_initpath(char *exe)
{
	char *buf, *home, *p;

	home = strdup(exe);
	sys_sanitize(home);
	p = (char *) strrchr(home, DIRSEP_CHAR);
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


