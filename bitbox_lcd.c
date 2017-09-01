#include <string.h>
#include <stdint.h>

/* TODO
- color !
- faster blits
- error sprite retourne ligne du bas
- erreur bord G si sprite demi visible
- priority bit ! (& bave verticalement si pas prio ?)
- decalage horizontal (mario)
- palette qui clignote (megaman level1)
*/

#include <bitbox.h>


#include "gnuboy.h"
#include "defs.h"
#include "regs.h"
#include "hw.h"
#include "mem.h"
#include "lcd.h"
#include "rc.h"
#include "fb.h"


// also, look at RBIT on ARM cortex m3 or merge with next table
const uint8_t reversed_bits[256] = {
	0x0, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x8, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x4, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0xc, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x2, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0xa, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea, 0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x6, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0xe, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x1, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x9, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x5, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0xd, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x3, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0xb, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x7, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0xf, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

const uint16_t morton8[256]={ // interleave bit with zeros 0b11111111 -> 0b1010101010101010
	0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015, 0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
	0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115, 0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
	0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415, 0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
	0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515, 0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
	0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015, 0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
	0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115, 0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
	0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415, 0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
	0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515, 0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
	0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015, 0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
	0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115, 0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
	0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415, 0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
	0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515, 0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
	0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015, 0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
	0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115, 0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
	0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415, 0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
	0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515, 0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555, 
};

// instead do a reversed morton ?

struct lcd lcd;

struct scan scan;

// content of vram 
#define BG (scan.bg)
#define WND (scan.wnd)

//#define BUF (scan.buf) 
#define BUF (vdest) // GO DIRECTLY TO VDEST. XXX This will be an issue for color games if they change the palette mid frame.
/*BUF = output buffer for the line. The structure
of this array is simple: it is composed of 6 bpp gameboy color
numbers, where the bits 0-1 are the color number from the tile, bits
2-4 are the (cgb or dmg) palette index, and bit 5 is 0 for background
or window, 1 for sprite.*/


#define PRI (scan.pri)

#define PAL1 (scan.pal1)
#define PAL2 (scan.pal2)
#define PAL4 (scan.pal4)

#define VS (scan.vs) /* vissprites */
#define NS (scan.ns) // nb sprites ?

#define L (scan.l) /* current line being blit */
#define X (scan.x) /* screen position, from X = R_SCX; */
#define Y (scan.y) // (R_SCY + L) & 0xff;
#define S (scan.s) // tilemap position, from X (bg only) 
#define T (scan.t) // from y 
#define U (scan.u) /* position within tile from x,y */
#define V (scan.v)
#define WX (scan.wx)
#define WY (scan.wy)
#define WT (scan.wt)
#define WV (scan.wv)

static int scale = 1;
static int density = 0;


// static int sprsort = 1;
static int sprdebug;

#define DEF_PAL { 0x98d0e0, 0x68a0b0, 0x60707C, 0x2C3C3C }
/* FIXME See : games colorization !!
 http://tcrf.net/CGB_Bootstrap_ROM
*/

// Palettes : 
int dmg_pal[4][4] = { DEF_PAL, DEF_PAL, DEF_PAL, DEF_PAL };

static int usefilter, filterdmg;
static int filter[3][4] = {
	{ 195,  25,   0,  35 },
	{  25, 170,  25,  35 },
	{  25,  60, 125,  40 }
};

/*
rcvar_t lcd_exports[] =
{
	RCV_INT("scale", &scale),
	RCV_INT("density", &density),
	RCV_VECTOR("dmg_bgp", dmg_pal[0], 4),
	RCV_VECTOR("dmg_wndp", dmg_pal[1], 4),
	RCV_VECTOR("dmg_obp0", dmg_pal[2], 4),
	RCV_VECTOR("dmg_obp1", dmg_pal[3], 4),
	RCV_BOOL("sprsort", &sprsort),
	RCV_BOOL("sprdebug", &sprdebug),
	RCV_BOOL("colorfilter", &usefilter),
	RCV_BOOL("filterdmg", &filterdmg),
	RCV_VECTOR("red", filter[0], 4),
	RCV_VECTOR("green", filter[1], 4),
	RCV_VECTOR("blue", filter[2], 4),
	RCV_END
};
*/

static byte *vdest;

#ifdef ALLOW_UNALIGNED_IO /* long long is ok since this is i386-only anyway? */
#define MEMCPY8(d, s) ((*(long long *)(d)) = (*(long long *)(s)))
#else
#define MEMCPY8(d, s) memcpy((d), (s), 8)
#endif



inline uint16_t interleave(uint8_t a, uint8_t b)
{
	return morton8[a] | morton8[b]<<1;
}



inline int get_bgtileofs(int tile_h, int tile_v)
{
	// get absolute tile_id in vram
	// from horizontal tile ID on line
	int tile_id = ((X+7)/8+tile_h)%32+tile_v*32;
	// index of the tile in tileset
	// BG Tile Map Display Select (0=9800-9BFF, 1=9C00-9FFF)
	uint8_t *tilemap = lcd.vbank[0] + ((R_LCDC&0x08)?0x1C00:0x1800);

	// get absolute tile id as an offset from tile number.
	// tilemap points to  (R_LCDC & 0x10) : BG & Window Tile Data Select   (0=8800-97FF, 1=8000-8FFF)
	// tilemap indicates either uint8_t at 0x8000 or int8_t at 8800 (i.e. pattern #0 lies at address $9000 = $8800 + 16*128)
	if (R_LCDC & 0x10) 
		return tilemap[tile_id];
	else
		// 1st tile at 9000 = 256th tile
		return (256 + ((int8_t)(tilemap[tile_id]))); 
}

inline void blit8(uint8_t *dest,uint8_t a, uint8_t b )
// blit 8 bits to buf as interlaced 
// a,b : lsb, msb to blit

		/* Improvements : unroll, blit 4 by 4, don't care of over blitting,
		 we're going to display window on it anyway or not show it.
		 use a 256-table for 4+4 bits interlace & next
		 */
{
	for (int i=7;i>=0;--i) 
	{
		*(dest++) = ((b & (1<<i))>>i)<<1 | ((a & (1<<i))>>i); 
	}
}

void bg_scan()
{
	// remplit scanline buffer for background.
	// background buffer is a u8 array 

	int tile_v = scan.y/8;


	// S,T : bg tilemap position, starts direct at the right tile.

	byte *vram = lcd.vbank[0]; // tileset is always there
	byte a,b;
	int tile_ofs;

	// display background over data
	// background is displayed from 0 to window if we reached window vertically (screen coordinates)
	// current line (screen coordinates) is scan.l, which is 

	byte *dest = BUF; 


	// src = patpix[*(tile++)][V] + U; // U,V : position within tile.
	// assumes unroll + loop opt (if should be done outside) by the compiler.

	// before first full tile
	tile_ofs = get_bgtileofs(-1, tile_v); // tile before
	a = vram[tile_ofs*16+scan.v*2]; // LSB
	b = vram[tile_ofs*16+scan.v*2+1]; // MSB
	for (int i=(8-scan.x)&7;i>=0;--i)
	{
		*(dest++) = ((b & (1<<i))>>i)<<1 | ((a & (1<<i))>>i); 
	}

	// Roll all tiles
	for (int tile_h= 0 ; tile_h<(WX+7)/8 ; tile_h++) // tile : 0 -> window start, screen coord.
	{
		tile_ofs = get_bgtileofs(tile_h, tile_v);
		a = vram[tile_ofs*16+scan.v*2]; // LSB
		b = vram[tile_ofs*16+scan.v*2+1]; // MSB
		blit8(dest, a,b);
		dest +=8;
	}
}



void wnd_scan()
{

	int tile_v = (L-WY)/8;
	// already done if end of line (no window, hidden, not on this line, ...)
	if (WX >= 160) return; 
	uint8_t * dest = BUF + WX;
	int tile_ofs;
	uint8_t a,b;
	uint8_t *vram = lcd.vbank[0]; // tileset is always there


	uint8_t *tilemap = lcd.vbank[0]+((R_LCDC&0x40)?0x1C00:0x1800);
	// depasse un peu si ne finit pas pile. pas grave on a prévu la place
	for (int tile_h = 0; tile_h<(160 - WX)/8 ; tile_h++) 
	{

		int tile_id = tile_h+tile_v*32;

		if (R_LCDC & 0x10) 
			tile_ofs = tilemap[tile_id];
		else
			// 1st tile at 9000 = 256th tile
			tile_ofs = (256 + ((int8_t)(tilemap[tile_id]))); 


		a = vram[tile_ofs*16+WV*2]; // LSB
		b = vram[tile_ofs*16+WV*2+1]; // MSB
		blit8(dest, a,b);
		dest +=8;
	}

}



static void blendcpy(byte *dest, byte *src, byte b, int cnt)
{
	while (cnt--) *(dest++) = *(src++) | b;
}

static int priused(void *attr)
{
	uint32_t *a = (uint32_t *)attr;
	return (int)((a[0]|a[1]|a[2]|a[3]|a[4]|a[5]|a[6]|a[7])&0x80808080);
}


static void recolor(byte *buf, byte fill, int cnt)
{
	while (cnt--) *(buf++) |= fill;
}

void spr_count()
{
	int i;
	struct obj *o;
	
	NS = 0;
	if (!(R_LCDC & 0x02)) return;
	
	o = lcd.oam.obj;
	
	for (i = 40; i; i--, o++)
	{
		if (L >= o->y || L + 16 < o->y)
			continue;
		if (L + 8 >= o->y && !(R_LCDC & 0x04))
			continue;
		if (++NS == 10) break;
	}
}

void spr_enum()
// put at most 10 sprites data to the VS array as a<<8 | b, with pal, x, y 
// b&w only. 
{
	int i,v;
	struct obj *o;
	uint8_t a,b;

	if (hw.cgb) return; // not now.
	if (!(R_LCDC & 0x02)) return; // display pas les sprites ?

	NS = 0; // 0 sprites maintenant.
	o = lcd.oam.obj;
	
	for (i = 40; i; i--, o++) // scanne toutes les sprites. 
	{
		// skip passed sprites
		if (scan.l >= o->y || (scan.l) + 16 < o->y)
			continue;
		// small vertical sprites ?
		if (scan.l + 8 >= o->y && !(R_LCDC & 0x04))
			continue;

		// cree vissprite a la fin 
		scan.vs[scan.ns].x = (int)o->x - 8;

		v = scan.l +16 - (int)o->y; // position ds le sprite 

		if (o->flags & 0x40) { // reverse verti
			if ((R_LCDC & 0x04)) // big sprites
				v = 16-v;
			else 
				v =  8-v;
		}
		/* bits 0-1 are the color number from the tile, 
		   bits 2-4 are the (cgb or dmg) palette index, 
		   and bit 5 is 0 for background or window, 1 for sprite.*/

		scan.vs[scan.ns].pal = 1<<5 | ((o->flags & 0x10) >> 2); // palette : 32 + 4 (bit2) si flag palette 
		scan.vs[scan.ns].pri = (o->flags & 0x80) >> 7; // priority obj to bg

		//pat = o->pat | (((int)o->flags & 0x60) << 5); // pattern : en fn du flag : x/y flips
		if ((R_LCDC & 0x04)) // sprite size=16
		{
			o->pat  &= ~1; // ignore low bit
			if (v >= 8) 
			{
				v -= 8;
				o->pat ++;
			}
		};

		// take a and b from vram
		a = lcd.vbank[0][o->pat*16+2*v];
		b = lcd.vbank[0][o->pat*16+2*v+1];

		if (o->flags & 0x20) // reverse hori
			// flip x FIXME : make a interleave reverse table
			VS[NS].pix = interleave(reversed_bits[a],reversed_bits[b]); 
			//VS[NS].pix = interleave(reversed_bits[a],reversed_bits[b]); 
		else // non flipped
			VS[NS].pix = interleave(a,b); 

		if (++NS == 10) break; // arret a 10 sprites.
	}
	// pas de tri
}

void spr_scan()
{
	int nb, start, x;
	byte b, ns = NS;
	//byte *src, *dest, *bg, *pri;
	uint8_t *dest;
	struct vissprite *vs;
	//static byte bgdup[256];

	if (!ns) return;

	//memcpy(bgdup, BUF, 256); // copie du BG pr priority
	
	for (vs = &VS[ns-1]; ns; ns--, vs--) // parcourt les sprites ds l'ordre inverse
	{
		x = vs->x;
		// skip if not visible
		if (x >= 160) continue; 
		if (x <= -8) continue;

		// verifie nb a copier, start (depart ds sprite), et position de depart. ne peut pas etre a la fois au debut& a la fin
		if (x < 0) {
			nb = 8 + x;
			start = -x;
			dest = vdest;
		} else {
			start=0;
			dest = vdest + x;
			if (x > 152) nb = 160 - x;
			else nb = 8;
		}

		// simple solution : FIXME handle priority bit ! (here, sprites will always be in front of window)
		for (int i=0;i<nb;i++) 
		{
			b = (vs->pix>>2*(7-i-start)) & 3;
			if (b) dest[i] = vs->pal|b; // sprite palette 
		}

	}
}

void lcd_begin()
{
/*	vdest = fb.ptr + ((fb.w*fb.pelsize)>>1)
		- (80*fb.pelsize) * scale
		+ ((fb.h>>1) - 72*scale) * fb.pitch;
*/
	vdest = fb.ptr; // duh
	WY = R_WY;
}

void lcd_refreshline()
/* writes to vdest directly ! palette translation will be done on screen output. could be an issue*/
{
	
	if (!fb.enabled) return;
	if (!(R_LCDC & 0x80)) // disbled ?
		return; /* should not happen... */

	L = R_LY;
	X = R_SCX;
	Y = (R_SCY + L) & 0xff; // offset+ current line, wrapped (map coordinates)
	S = X >> 3;
	T = Y >> 3;
	U = X & 7;
	V = Y & 7;
	
	WX = R_WX - 7;
	if (WY>L || WY<0 || WY>143 || WX<-7 || WX>159 || !(R_LCDC&0x20))
		WX = 160;
	WT = (L - WY) >> 3;
	WV = (L - WY) & 7;

	spr_enum();

	// tilebuf();
	bg_scan();
	wnd_scan();

		// OR 4 on for window ? ( bit 2 in buf  =  palette index - for BG/W)
	recolor(BUF+WX, 0x04, 160-WX);  

	spr_scan(); // apply sprites data 

	// refresh_2((un16*)vdest, BUF, PAL2, 160); // always vdest (no scalebuf), etc.etc
	// will call  : for (int i=0;i<160;i++) { *(dest++) = palette[*(src++)] };
	vdest += 160; // pitch=160, always write to frame buffer. 
}

// start & stop on screen
#define line1 (480-144*2)/2  
#define line2 (480+144*2)/2
#define col1 (640-320)/2

void emu_line()
{
	if (vga_line==0) memset(draw_buffer,0,320*2); // 320 pixels wide = fullscreen

	if ((vga_line+1)/2==line1/2 || (vga_line+1)/2 == line2/2) 
	{
		memset(&draw_buffer[col1],0,160*2*2);
	}
	if ((vga_line+1)/2 <= line1/2 || (vga_line+1)/2 >= line2/2)  return;

	// XXX do it for u32 and use PAL4
	uint32_t *dest= (uint32_t*) &draw_buffer[col1];
	uint8_t *src = &fb.ptr[160*((vga_line-line1)/2)]; 
	for (int i=0;i<160;i++) 
	// 2 pixels at a time
		*(dest++) = PAL4[*(src++)];
	
}



// here we allow frame skipping and write a whole frame, while the LCD can switch frames 
// the video kernel will be a bit different as it is not a line buffer but an image buffer.
// palette translation will be done by output line however.




// palettes --------------------------------------------------------------------------
// FIXME only dmg

static void updatepalette(int i)
// we're only using one unique mode (pixel size =2, no sw scaling)
// only use PAL2
{
	int c, r, g, b;

	c = (lcd.pal[i<<1] | ((int)lcd.pal[(i<<1)|1] << 8)) & 0x7FFF;
	r = (c & 0x001F) << 3;
	g = (c & 0x03E0) >> 2;
	b = (c & 0x7C00) >> 7;
	r |= (r >> 5);
	g |= (g >> 5);
	b |= (b >> 5);


	r = (r >> fb.cc[0].r) << fb.cc[0].l;
	g = (g >> fb.cc[1].r) << fb.cc[1].l;
	b = (b >> fb.cc[2].r) << fb.cc[2].l;
	c = r|g|b;
	
	PAL2[i] = c;
	PAL4[i] = c<<16|c;

}

void pal_write(int i, byte b)
{
	if (lcd.pal[i] == b) return;
	lcd.pal[i] = b;
	updatepalette(i>>1);
}

void pal_write_dmg(int i, int mapnum, byte d)
{
	int j;
	int *cmap = dmg_pal[mapnum];
	int c, r, g, b;

	if (hw.cgb) return;

	/* if (mapnum >= 2) d = 0xe4; */
	for (j = 0; j < 8; j += 2)
	{
		c = cmap[(d >> j) & 3];
		r = (c & 0xf8) >> 3;
		g = (c & 0xf800) >> 6;
		b = (c & 0xf80000) >> 9;
		c = r|g|b;
		/* FIXME - handle directly without faking cgb */
		pal_write(i+j, c & 0xff);
		pal_write(i+j+1, c >> 8);
	}
}

void vram_write(int a, byte b)
{
	lcd.vbank[R_VBK&1][a] = b;
}

void vram_dirty()
{
}


void pal_dirty()
{
	int i;
	if (!hw.cgb)
	{
		pal_write_dmg(0, 0, R_BGP);
		pal_write_dmg(8, 1, R_BGP);
		pal_write_dmg(64, 2, R_OBP0);
		pal_write_dmg(72, 3, R_OBP1);
	}
	for (i = 0; i < 64; i++)
		updatepalette(i);
}

void lcd_reset()
{
	memset(&lcd, 0, sizeof lcd);
	lcd_begin();
	//vram_dirty();
	pal_dirty();
}
