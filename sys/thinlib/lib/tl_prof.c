/*
** thinlib (c) 2001 Matthew Conte (matt@conte.com)
**
**
** tl_prof.c
**
** Screen border color profiler nastiness.
**
** $Id: tl_prof.c,v 1.1.1.1 2003/07/04 19:19:58 joshua Exp $
*/

#include <pc.h>
#include "tl_types.h"
#include "tl_prof.h"

void thin_prof_setborder(int pal_index)
{
   inportb(0x3DA);
   outportb(0x3C0, 0x31);
   outportb(0x3C0, (uint8) pal_index);
}

/*
** $Log: tl_prof.c,v $
** Revision 1.1.1.1  2003/07/04 19:19:58  joshua
** initial import
**
*/
