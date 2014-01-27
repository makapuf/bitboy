#pragma once
#include <stdint.h>
#include "gamepad.h"
#include "audio.h"

// 0000RRRRGGGGBBBB

#define MARGIN  32
#define MAX_SCREEN_WIDTH 1024-2*MARGIN // inpixels
#define TILEDATA_ALIGN 16

inline uint16_t RGB(uint8_t r, uint8_t g, uint8_t b) 
{
    return (((b)&0xf)<<8 | ((g)&0xf)<<4 | ((r)&0xf));
}

#ifdef BITBOX_MODE_320
#define LINE_LENGTH 320 // attn this is not the size of the buffer since margins are added (easier to handle deltas)
#define PIXELCLOCK 14 
#else 
#define LINE_LENGTH 640 
#define PIXELCLOCK 7 // from line_length ?
#endif 

void game_init(void);
void game_frame(void);
void game_line(void);

void game_sample(void); // DEPRECATED this callback is called each time we need a new audio sample  !
void game_snd_buffer(uint8_t *buffer, int len); // this callback is called each time we need to fill the buffer

void die(char *fmt, ...);

// more pixels to allow over blitting 

extern uint16_t *draw_buffer;
extern uint32_t vga_line;
extern volatile uint32_t vga_frame;

void vga640_setup();

// perfs counters
extern uint32_t max_line,max_line_time; // max_line is the maximum number of cycles used on last frame.

