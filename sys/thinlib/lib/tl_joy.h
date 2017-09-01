/*
** thinlib (c) 2001 Matthew Conte (matt@conte.com)
**
**
** tl_joy.h
**
** DOS joystick reading defines / protos
**
** $Id: tl_joy.h,v 1.1.1.1 2003/07/04 19:19:58 joshua Exp $
*/

#ifndef _TL_JOY_H_
#define _TL_JOY_H_

#define  JOY_MAX_BUTTONS   4

typedef struct joy_s
{
   int left, right, up, down;
   int button[JOY_MAX_BUTTONS];
} joy_t;

extern void thin_joy_shutdown(void);
extern int thin_joy_init(void);
extern int thin_joy_read(joy_t *joy);

#endif /* !_TL_JOY_H_ */

/*
** $Log: tl_joy.h,v $
** Revision 1.1.1.1  2003/07/04 19:19:58  joshua
** initial import
**
*/
