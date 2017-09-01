/*
 * bitbox interfaces (c) makapuf@gmail.com
 */

// main (as game_frame) is in bitbox_emu.c

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h> // for memcpy

#include "gnuboy.h"
#include "fb.h"
#include "input.h"
#include "hw.h"

#include <bitbox.h>


#define BITBOX_VIDBUF_LEN (160*144)

extern uint16_t audio_write[BITBOX_SNDBUF_LEN];
uint8_t audio_buffer[BITBOX_SNDBUF_LEN];

// put to ccmram to save space on sram
uint8_t video_buffer1[BITBOX_VIDBUF_LEN] __attribute__ ((section (".ccm")));
uint8_t video_buffer2[BITBOX_VIDBUF_LEN] __attribute__ ((section (".ccm")));

static uint8_t *lcd_display_buffer;
uint8_t framebuffer_flip; // ask to flip framebuf

struct fb fb; // here the frame buffer contains emulated pixels, the translation to real world is done in lines

void vid_init()
// force to 160x144 8bits
// will be translated back to 640x480 16 bits in game_line
{
	fb.w = 160;
	fb.h = 144;
	fb.pitch = 160;
	// not used
	fb.pelsize = 1;
	fb.indexed = 0;
	
	// set for the line rendering 0BGR, 12 bits
	// R
	fb.cc[0].r = 3; // 8 bits->5
	fb.cc[0].l = 10;
	// G
	fb.cc[1].r = 3;
	fb.cc[1].l = 5;
	// B
	fb.cc[2].r = 3;
	fb.cc[2].l = 0;


	fb.ptr = (byte *)&video_buffer1[0]; // = draw buffer
	lcd_display_buffer = &video_buffer2[0];

	fb.dirty = 1;
	fb.enabled = 1;
}

/* hw.pad values 
#define PAD_RIGHT  0x01
#define PAD_LEFT   0x02
#define PAD_UP     0x04
#define PAD_DOWN   0x08
#define PAD_A      0x10
#define PAD_B      0x20
#define PAD_SELECT 0x40
#define PAD_START  0x80
*/

// mapping of bitbox to gnuboy hw.pad bits

const int joystick_pos[8] = {
	gamepad_right, 
	gamepad_left, 
	gamepad_up,
	gamepad_down, 
	gamepad_A,
	gamepad_B,
	gamepad_select,
	gamepad_start
};

void gamepad_poll()
// direct mapping of bitbox events to hardware hw.pad, bypassing all of events ev_* functions.
{
	static uint16_t prev_state; // to detect changes since last time.
	hw.pad=0;
	for (int i=0;i<8;i++) {
		if (gamepad_buttons[0] & joystick_pos[i]) 
				hw.pad |= 1<<i;
	}
}


void vid_preinit()
{
}

void vid_close()
{
}

void vid_settitle(char *title)
{
	//SDL_WM_SetCaption(title, title);
}


void vid_begin() // as in begin FRAME. We're synced to vsync here
{
	uint8_t *tmp;

	// flip draw buffer & display
	tmp=lcd_display_buffer; 
	lcd_display_buffer=fb.ptr;
	fb.ptr=tmp; // current drawing framebuffer
}

void vid_end() // as in end frame
// 
{
	// XXX flip framebuffer (wait vsync ?)
}


#include "pcm.h"

struct pcm pcm;

static volatile int audio_done;

void game_snd_buffer(uint16_t *buffer, int len) 
// this callback is called each time we need to fill the buffer
// should be done within one line (??)
{
	if (pcm.buf) {
		uint16_t *b=buffer;
		for (int i=0;i<len;i++)
			*b++=pcm.buf[i]*0x101;
	}
	audio_done = 1;
}

void pcm_init()
{
	pcm.hz = BITBOX_SAMPLERATE;
	pcm.stereo = 0; // mono sound 
	pcm.len = BITBOX_SNDBUF_LEN; // uint8_t samples per 60Hz frame 
	pcm.buf = audio_buffer; // dont make a new one
	pcm.pos = 0;
	memset(pcm.buf, 0, pcm.len);
	// now start bitbox audio
	// audio_init(); // already initialized ?
	// audio_on = 1;
}

int pcm_submit() // time sync also done here from the buffer which MUST be one frame long.
{
	if (pcm.pos < pcm.len) return 1;
    #ifdef EMULATED
    while (!audio_done) // attend pile 
		SDL_Delay(2);
	#endif 

	audio_done = 0;
	pcm.pos = 0;
	return 1;
}

void pcm_close()
{
}



void gb_die(char *fmt, ...)
{

#ifdef EMULATED
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
#endif
}



// -------------------------------------------------------------------------
// rom loaders
#include "mem.h"
#include "hw.h"
#include "rtc.h"
#include "rc.h"

extern const char * const game_names[];
extern const uint8_t * const game_roms[];
extern const uint32_t game_palettes[][3][4];
static int mbc_table[256] =
{
	0, 1, 1, 1, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 3,
	3, 3, 3, 3, 0, 0, 0, 0, 0, 5, 5, 5, MBC_RUMBLE, MBC_RUMBLE, MBC_RUMBLE, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, MBC_HUC3, MBC_HUC1
};

static int rtc_table[256] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0
};

static int batt_table[256] =
{
	0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0,
	1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0,
	0
};

static int romsize_table[256] =
{
	2, 4, 8, 16, 32, 64, 128, 256, 512,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 128, 128, 128
	/* 0, 0, 72, 80, 96  -- actual values but bad to use these! */
};

static int ramsize_table[256] =
{
	1, 1, 1, 4, 16,
	4 /* FIXME - what value should this be?! */
};

extern int dmg_pal[4][4];

#ifdef BITBOY_FIXEDRAM
uint8_t ram_mem[8192 * BITBOY_FIXEDRAM];
#endif 

int bitbox_rom_load(int rom_id)
// almost like original rom load but provides directly data
{
	uint8_t c;
	uint8_t *data, *header;
	// int len = 0, rlen;

	data = (uint8_t *)game_roms[rom_id];

	header = data;
	
	memcpy(rom.name, header+0x0134, 16);
	if (rom.name[14] & 0x80) rom.name[14] = 0;
	if (rom.name[15] & 0x80) rom.name[15] = 0;
	rom.name[16] = 0;

	c = header[0x0147];
	mbc.type = mbc_table[c];
	mbc.batt = batt_table[c];
	rtc.batt = rtc_table[c];

	mbc.romsize = romsize_table[header[0x0148]];
	mbc.ramsize = ramsize_table[header[0x0149]];

	if (!mbc.romsize) message("unknown ROM size %02X\n", header[0x0148]);
	if (!mbc.ramsize) message("unknown SRAM size %02X\n", header[0x0149]);

	// XXX extra size ? (ie pad memory when file is smaller than declared size)
	/*
	rlen = 16384 * mbc.romsize;
	rom.bank = realloc(data, rlen);
	if (rlen > len) memset(rom.bank[0]+len, 0xff, rlen - len);
	*/
	rom.bank= (byte (*)[16384]) data;

	// static
	#ifdef BITBOY_FIXEDRAM 
	ram.sbank = (void*)&ram_mem;
	#else
	ram.sbank = malloc(8192 * mbc.ramsize);
	#endif 
	// initmem(ram.sbank, 8192 * mbc.ramsize);
	// initmem(ram.ibank, 4096 * 8);

	mbc.rombank = 1;
	mbc.rambank = 0;

	// debug
	message("loaded ROM , mapper type : %X romsize %d ramsize %d rombank = %d rambank = %d\n", 
		mbc.type, 
		mbc.romsize, mbc.ramsize, 
		mbc.rombank, mbc.rambank);

	// set custom dmg palette
	// XXX what about 4th palette ?
	for (int i=0;i<3;i++)
		for (int j=0;j<4;j++)
			dmg_pal[i][j] = (int)game_palettes[rom_id][i][j];
	

	// XXX NOPE NO GBC
	c = header[0x0143];
	hw.cgb = 0; // ((c == 0x80) || (c == 0xc0)) ;
	hw.gba = 0; // (hw.cgb && gbamode);

	return 0;
}

