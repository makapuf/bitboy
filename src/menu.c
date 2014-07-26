/* simple menu */

// character size
#define screen_w 160/8 // 20
#define screen_h 144/8 // 18

#define TILE_CURSOR 64
#define MENU_LEN 15

#ifdef NOTITLE 
#define INTRO_TIME 0*60
#else
#define INTRO_TIME 4*60
#endif 

#include <stdint.h>
#include <string.h> // memcpy
#include <bitbox.h>
#include "tilemaps.h"


uint8_t tilemap[screen_w*screen_h];
// 64 first : ascii 32 -> 96, 64 next  : highlighted

// null terminated
extern char *game_names[]; // 16 max 
extern const uint8_t *const game_roms[]; // 16 max now c
// decor

// start & stop line on screen
#define line1 (480-144*2)/2  
#define line2 (480+144*2)/2
#define col1 (640-320)/2

void menu_line()
{
	if ((vga_line+1)/2==line1/2 || (vga_line+1)/2 == line2/2) 
	{
		memset(draw_buffer+col1,0,160*2*2);
	}
	if ((vga_line+1)/2 <= line1/2 || (vga_line+1)/2 >= line2/2)  return;

	uint8_t *tmap = &tilemap[(vga_line-line1)/16*screen_w];
	uint8_t c;
	const uint16_t *src;
	uint32_t *dst;
	dst = (uint32_t* )(draw_buffer+col1);

	for (int i=0;i<20;i++)
	{
		c = *tmap++;
		src = &tile_data[c][(((vga_line-line1)/2)%8)*8]; 
		for (int i=0;i<8;i++) {
			uint16_t c = *src++;
			*dst++ = c * 0x10001; 
		}
	}
}

void txt(int x, int y, char *data)
{
	uint8_t *dst=tilemap+screen_w*y+x;
	while (*data) *dst++=(*data++ - ' ')&0x3f;
}

void draw_screen()
{
	memcpy(tilemap,tilemap_menu,screen_w*screen_h);
	// affiche choix
	for (int i=0;i<MENU_LEN;i++)
	{
		txt(3,i+2,game_roms[i]?game_names[i]:" --             ");
	}	

}

int menu_choice=0;
int menu_done=0;

void menu_setup()
{
	menu_choice=0;
	menu_done=0;
}

void menu_frame() 
{
	if (vga_frame<INTRO_TIME)
	{
		memcpy(tilemap,tilemap_intro, screen_w*screen_h);
	}
	else
	{
	// fond: in init.
	draw_screen();

	// choix courant : animated cursor.
	for (int i=0;i<MENU_LEN;i++)
		tilemap[1+screen_w*(2+i)] = (i==menu_choice)?(TILE_CURSOR+(vga_frame/8)%4):0;

	// check start pressed
	if (vga_frame%4 == 0)
	{
		if (GAMEPAD_PRESSED(0,start) || GAMEPAD_PRESSED(0,A)) menu_done=1;
		if (GAMEPAD_PRESSED(0,select)) {
			if (menu_choice<MENU_LEN && game_roms[menu_choice+1])
				menu_choice++;
			else 
				menu_choice=0;
		}

		if (GAMEPAD_PRESSED(0,up) && menu_choice>0) menu_choice--;
		if (GAMEPAD_PRESSED(0,down) && menu_choice<MENU_LEN && game_roms[menu_choice+1]) menu_choice++;
	}

	}
}
