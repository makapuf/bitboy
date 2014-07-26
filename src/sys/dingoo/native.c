/*
** Native Dingoo (uCOS II ) timer routines.
**
** NOTE ensure GNUBOY_USE_SDL_TIMERS is not defined
**
*/

#include <stdlib.h>
#include <time.h>

#include "gnuboy.h"


#define US(n) ( ((long)(n)) * 1000000 / CLOCKS_PER_SEC )


/*
**  Return timer suitable for use with sys_elapsed()
**  Timer is initialized with the current time.
*/
void *sys_timer()
{
    /* Currently using posix clock api rather than ucos OSTimeGet() */
	clock_t *cl;

	cl = malloc(sizeof cl);
	*cl = clock(); /* look at using OSTimeGet() instead .... */
	return (void *) cl;
}

/*
**  Return number of microseconds since input timer was last updated.
**  Also update the input timer with the current time.
*/
int sys_elapsed(void *in_ptr)
{
	clock_t *cl;
	clock_t now;
	int usecs;

	cl = (clock_t *) in_ptr;
	now = clock();
	usecs = (int) US(now - *cl);
	*cl = now;
	return usecs;
}

void sys_sleep(int us)
{
	/*
	**  "us" is microseconds, SDL timers are all milliseconds
	**  1 second [s]  == 1000 millisecond [ms] == 1000000 microsecond [us]
	**  1 millisecond [ms] == 1000 microsecond [us]
	**  1000 microsecond [us] == 0.001 millisecond [ms]
	*/

	/*
	** if zero or negative,
	** do NOT perform math for microsecond to millisecond conversion
	*/

	/* Busy wait, not efficient but seems to be more accurate than OS native sleep */
	clock_t start;
	if (us <= 0)
		return;
	start = clock();
	while ((int) US(clock()-start) < us);

}
