/*
** Dingoo A320 main
** modelled on generic gnuboy main.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#ifdef DINGOO_NATIVE
#include <dingoo/audio.h>
#endif /* DINGOO_NATIVE */

#include "gnuboy.h"
#include "loader.h"
#include "input.h"
#include "rc.h"


#include "Version"


/* Dingoo SDL key mappings are the default, can over ride in gnuboy.rc */
static char *defaultconfig[] =
{
	"bind space quit", /* X button */
	"bind shift quit", /* Y button - LSHIFT*/
	"bind tab quit", /* Left shoulder */
	"bind backspace quit", /* Right shoulder */
	"bind up +up", /*  */
	"bind down +down", /*  */
	"bind left +left", /*  */
	"bind right +right", /*  */
	"bind ctrl +a", /* A button - LEFTCTRL */
	"bind alt +b", /* B button - LEFTALT */
	"bind enter +start", /* START button */
	"bind esc +select", /* SELECT button */
	/*
	"bind 1 \"set saveslot 1\"",
	"bind 2 \"set saveslot 2\"",
	"bind 3 \"set saveslot 3\"",
	"bind 4 \"set saveslot 4\"",
	"bind 5 \"set saveslot 5\"",
	"bind 6 \"set saveslot 6\"",
	"bind 7 \"set saveslot 7\"",
	"bind 8 \"set saveslot 8\"",
	"bind 9 \"set saveslot 9\"",
	"bind 0 \"set saveslot 0\"",
	"bind ins savestate",
	"bind del loadstate",
	*/
#ifdef DINGOO_NATIVE
	"source A:\\\\GAME\\\\gnuboy.rc", /* native OS path; Maybe try same directory as .sim location too? NOTE gnuboy.rc needs "\" to be escaped (as does C compiler for string literals) */
#endif /* DINGOO_NATIVE */
	"source gnuboy.rc", /* try same directory as rom */
	NULL
};


void doevents()
{
	event_t ev;
	int st;

	ev_poll();
	while (ev_getevent(&ev))
	{
		if (ev.type != EV_PRESS && ev.type != EV_RELEASE)
			continue;
		st = (ev.type != EV_RELEASE);
		rc_dokey(ev.code, st);
	}
}


static void shutdown()
{
	vid_close();
	pcm_close();
}

void die(char *fmt, ...)
{
	va_list ap;
#ifdef DINGOO_DIE_LOG_TO_FILE
	char tmp_buf[1024];
    int mesg_len=0;
    FILE *fptr=NULL;
#endif /* DINGOO_DIE_LOG_TO_FILE */

	va_start(ap, fmt);
#ifdef DINGOO_DIE_LOG_TO_FILE
    fptr = fopen("stderr.txt", "wb");
    if (fptr != NULL)
    {
        mesg_len = vsnprintf(tmp_buf, sizeof(tmp_buf)-1, fmt, ap);
        fwrite(tmp_buf, mesg_len, 1, fptr);
        fclose(fptr);
    }
    /* else nothing we can do except maybe print to screen */
#else
	fprintf(stderr, fmt, ap); /* NOTE with Oct 2010 Dingoo native SDK this goes to the serial port */
#endif /* DINGOO_DIE_LOG_TO_FILE */
	va_end(ap);
	exit(1);
}

static int bad_signals[] =
{
	/* These are all standard, so no need to #ifdef them... */
	SIGINT, SIGSEGV, SIGTERM, SIGFPE, SIGABRT, SIGILL,
#ifdef SIGQUIT
	SIGQUIT,
#endif
#ifdef SIGPIPE
	SIGPIPE,
#endif
	0
};

static void fatalsignal(int s)
{
	die("Signal %d\n", s);
}

static void catch_signals()
{
	int i;
	for (i = 0; bad_signals[i]; i++)
		signal(bad_signals[i], fatalsignal);
}

#ifdef GNUBOY_NO_PRINTF
void debug_printf_init()
{
	char tmp_buf[1024];
	int mesg_len=0;
	FILE *fptr=NULL;

	fptr = fopen("stdout.txt", "w");
	if (fptr != NULL)
	{
		mesg_len = sprintf(tmp_buf, "stdout for gnuboy\n");
		fwrite(tmp_buf, mesg_len, 1, fptr);
		fclose(fptr);
	}
	/* else nothing we can do except maybe print to screen */
}

void debug_printf(char *fmt, ...)
{
	va_list ap;
	char tmp_buf[1024];
	int mesg_len=0;
	FILE *fptr=NULL;

	va_start(ap, fmt);
	fptr = fopen("stdout.txt", "a");
	if (fptr != NULL)
	{
		mesg_len = vsnprintf(tmp_buf, sizeof(tmp_buf)-1, fmt, ap);
		fwrite(tmp_buf, mesg_len, 1, fptr);
		fclose(fptr);
	}
	/* else nothing we can do except maybe print to screen */
	va_end(ap);
}
#endif /* GNUBOY_HAVE_PRINTF */

#ifdef DINGOO_NATIVE
char *path_search(char *name, char *mode, char *path)
{
    (void) mode; /* avoid warning about unused parameter */
    (void) path; /* avoid warning about unused parameter */
	return name;
}
#endif /* DINGOO_NATIVE */

#ifdef GNUBOY_HARDWARE_VOLUME
/*
** Set hardware volumne control
** volume should be specified in percent from 0 to 100
*/
void pcm_volume(int volume)
{
    #define MAX_DINGO_VOLUME 30
    
    /*
    ** Do not bother checking input
    ** Convert input percent into hardware value
    */
    volume = volume * MAX_DINGO_VOLUME / 100;
    
    /* Do check what we send to the hardware */
    if (volume > MAX_DINGO_VOLUME)
        volume = MAX_DINGO_VOLUME;
    if (volume < 0)
        volume = 0;

    /* FIXME TODO this should be moved into sys/dingoo/native.c */
    waveout_set_volume((unsigned int) volume);
}
#endif /* GNBOY_HARDWARE_VOLUME */


#ifndef GNUBOY_DISABLE_MAIN
int main(int argc, char *argv[])
{
	int i;
	char *opt, *arg, *cmd, *s, *rom = NULL;
#ifdef BUILD_DINGOO_SIM
    #define ARGC_COUNT_START 0
#else
    #define ARGC_COUNT_START 1
#endif /* BUILD_DINGOO_SIM */

    /*
	debug_printf_init();
	debug_printf("argc=%d\n", argc);
	debug_printf("argv[0]=>%s<\n", argv[0]);
    */
    
	/* Avoid initializing video if we don't have to */
	/* NOTE under Native dingoo SIM, argv[0] is NOT the exe name but the first argument! */
	for (i = ARGC_COUNT_START; i < argc; i++)
	{
		if (!strcmp(argv[i], "--bind")) i += 2;
		else if (!strcmp(argv[i], "--source")) i++;
		else if (!strcmp(argv[i], "--showvars"))
		{
			show_exports();
			exit(0);
		}
		else if (argv[i][0] == '-' && argv[i][1] == '-');
		else if (argv[i][0] == '-' && argv[i][1]);
		else rom = argv[i];
	}
	
	/*
	** If running as a .app, default rom name parameter,
	** default to a hard coded rom name Adjustris by Dave VanEe
	** available from http://www.pdroms.de/files/910/
	** TODO - add a ROM loading menu item
	*/
	if (!rom) rom = "Adjustris.GB.gz";
	if (!rom) die("missing ROM filename");

	/* If we have special perms, drop them ASAP! */
	vid_preinit();

	init_exports();

	s = strdup(argv[0]);
	sys_sanitize(s);
	sys_initpath(s);

	for (i = 0; defaultconfig[i]; i++)
		rc_command(defaultconfig[i]);

	cmd = malloc(strlen(rom) + 11);
	sprintf(cmd, "source %s", rom);
	s = (char *) strchr(cmd, '.');
	if (s) *s = 0;
	strcat(cmd, ".rc");
	rc_command(cmd);

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--bind"))
		{
			if (i + 2 >= argc) die("missing arguments to bind\n");
			cmd = malloc(strlen(argv[i+1]) + strlen(argv[i+2]) + 9);
			sprintf(cmd, "bind %s \"%s\"", argv[i+1], argv[i+2]);
			rc_command(cmd);
			free(cmd);
			i += 2;
		}
		else if (!strcmp(argv[i], "--source"))
		{
			if (i + 1 >= argc) die("missing argument to source\n");
			cmd = malloc(strlen(argv[i+1]) + 6);
			sprintf(cmd, "source %s", argv[++i]);
			rc_command(cmd);
			free(cmd);
		}
		else if (!strncmp(argv[i], "--no-", 5))
		{
			opt = strdup(argv[i]+5);
			while ((s = (char *) strchr(opt, '-'))) *s = '_';
			cmd = malloc(strlen(opt) + 7);
			sprintf(cmd, "set %s 0", opt);
			rc_command(cmd);
			free(cmd);
			free(opt);
		}
		else if (argv[i][0] == '-' && argv[i][1] == '-')
		{
			opt = strdup(argv[i]+2);
			if ((s = (char *) strchr(opt, '=')))
			{
				*s = 0;
				arg = s+1;
			}
			else arg = "1";
			while ((s = (char *) strchr(opt, '-'))) *s = '_';
			while ((s = (char *) strchr(arg, ','))) *s = ' ';
			
			cmd = malloc(strlen(opt) + strlen(arg) + 6);
			sprintf(cmd, "set %s %s", opt, arg);
			
			rc_command(cmd);
			free(cmd);
			free(opt);
		}
	}

	/* FIXME - make interface modules responsible for atexit() */
	atexit(shutdown);
	catch_signals();
	vid_init();
	pcm_init();

	rom = strdup(rom);
	sys_sanitize(rom);
	
	loader_init(rom);
	
	emu_reset();
	emu_run();

	/* never reached */
	return 0;
}
#endif /* GNUBOY_DISABLE_MAIN */
