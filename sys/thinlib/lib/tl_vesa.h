/*
** thinlib (c) 2001 Matthew Conte (matt@conte.com)
**
**
** tl_vesa.h
**
** VESA code header
**
** $Id: tl_vesa.h,v 1.1.1.1 2003/07/04 19:19:58 joshua Exp $
*/

#ifndef _TL_VESA_H_
#define _TL_VESA_H_

#include "tl_bmp.h"

extern int thin_vesa_init(int width, int height, int bpp, int param);
extern void thin_vesa_shutdown(void);

extern int thin_vesa_setmode(int width, int height, int bpp);

extern bitmap_t *thin_vesa_lockwrite(void);
extern void thin_vesa_freewrite(int num_dirties, rect_t *dirty_rects);

#endif /* !_TL_VESA_H_ */

/*
** $Log: tl_vesa.h,v $
** Revision 1.1.1.1  2003/07/04 19:19:58  joshua
** initial import
**
*/
