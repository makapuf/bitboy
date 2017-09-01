#ifndef __LCD_H__
#define __LCD_H__

#include "defs.h"
#include <stdint.h>

struct vissprite
{
	uint16_t pix; // interleaved 2bits line to blit
	uint8_t *buf;
	int x;
	uint8_t pal, pri, pad[6];
};

struct scan
// scanline 
{
	int bg[64]; // background
	int wnd[64]; // window
	uint8_t buf[256]; // buffer to write to
	uint8_t pal1[128];
	uint16_t pal2[64];
	uint32_t pal4[64];
	uint8_t pri[256];
	int ns; //  nb of sprites
	struct vissprite vs[16];

	int l, x, y, s, t, u, v, wx, wy, wt, wv;
};

struct obj
{
	uint8_t y;
	uint8_t x;
	uint8_t pat;
	uint8_t flags;
};

struct lcd
{
	uint8_t vbank[2][8192];
	union
	{
		uint8_t mem[256];
		struct obj obj[40];
	} oam;
	uint8_t pal[128];
};

extern struct lcd lcd;
extern struct scan scan;


void lcd_begin();
void lcd_refreshline();
void pal_write(int i, uint8_t b);
void pal_write_dmg(int i, int mapnum, uint8_t d);
void vram_write(int a, uint8_t b);
void pal_dirty();
void vram_dirty();
void lcd_reset();
void bg_scan_color();
void updatepatpix();

/* lcdc.c */
void lcdc_trans();
void lcdc_change(uint8_t b);
void stat_write(uint8_t b);
void stat_trigger();


#endif
