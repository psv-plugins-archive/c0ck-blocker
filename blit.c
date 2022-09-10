/*
	PSP VSH 24bpp text bliter
*/
#include <psp2/types.h>
#include <psp2/display.h>
#include <psp2/kernel/sysmem.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "blit.h"

#define ALPHA_BLEND 1
#undef ALPHA_BLEND

extern unsigned char msx[];

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int pwidth, pheight, bufferwidth, pixelformat;
static unsigned int* vram32;

static uint32_t fcolor = 0x00ffffff;
static uint32_t bcolor = 0xff000000;
SceUID displayblock;
int displayblockinit = 0;

#if ALPHA_BLEND
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static uint32_t adjust_alpha(uint32_t col)
{
	uint32_t alpha = col >> 24;
	uint8_t mul;
	uint32_t c1,c2;

	if (alpha == 0)
		return col;

	if (alpha == 0xFF)
		return col;

	c1 = col & 0x00FF00FF;
	c2 = col & 0x0000FF00;
	mul = (uint8_t)(255 - alpha);
	c1 = ((c1 * mul) >> 8) & 0x00FF00FF;
	c2 = ((c2 * mul) >> 8) & 0x0000FF00;

	return (alpha << 24) | c1 | c2;
}
#endif

static SceDisplayFrameBuf psvFrameBuf = {
		sizeof(SceDisplayFrameBuf), NULL, 960, 0, 960, 544
};

static SceDisplayFrameBuf framebuf;

int blit_setup(void)
{
	SceDisplayFrameBuf param;
	param.size = sizeof(SceDisplayFrameBuf);
	sceDisplayGetFrameBuf(&param, SCE_DISPLAY_SETBUF_IMMEDIATE);

	pwidth = param.width;
	pheight = param.height;
	vram32 = param.base;
	bufferwidth = param.pitch;
	pixelformat = param.pixelformat;

	if ((bufferwidth == 0) || (pixelformat !=0 ))
		return -1;

	fcolor = 0x00ffffff;
	bcolor = 0xff000000;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// blit text
/////////////////////////////////////////////////////////////////////////////
void blit_set_color(int fg_col,int bg_col)
{
	fcolor = fg_col;
	bcolor = bg_col;
}

/////////////////////////////////////////////////////////////////////////////
// blit text
/////////////////////////////////////////////////////////////////////////////
int blit_string(int sx,int sy,const char *msg)
{
	int x,y,p;
	int offset;
	char code;
	unsigned char font;
	uint32_t fg_col,bg_col;

#if ALPHA_BLEND
	uint32_t col,c1,c2;
	uint32_t alpha;

	fg_col = adjust_alpha(fcolor);
	bg_col = adjust_alpha(bcolor);
#else
	fg_col = fcolor;
	bg_col = bcolor;
#endif

//Kprintf("MODE %d WIDTH %d\n",pixelformat,bufferwidth);
	if( (bufferwidth==0) || (pixelformat!=0)) return -1;

	for(x=0;msg[x] && x<(pwidth/16);x++)
	{
		code = msg[x] & 0x7f; // 7bit ANK
		for(y=0;y<8;y++)
		{
			offset = (sy+(y*2))*bufferwidth + sx+x*16;
			font = y>=7 ? 0x00 : msx[ code*8 + y ];
			for(p=0;p<8;p++)
			{
#if ALPHA_BLEND
				col = (font & 0x80) ? fg_col : bg_col;
				alpha = col>>24;
				if(alpha==0)
				{
					vram32[offset] = col;
					vram32[offset + 1] = col;
					vram32[offset + bufferwidth] = col;
					vram32[offset + bufferwidth + 1] = col;
				}
				else if(alpha!=0xff)
				{
					c2 = vram32[offset];
					c1 = c2 & 0x00ff00ff;
					c2 = c2 & 0x0000ff00;
					c1 = ((c1*alpha)>>8)&0x00ff00ff;
					c2 = ((c2*alpha)>>8)&0x0000ff00;
					uint32_t color = (col&0xffffff) + c1 + c2;
					vram32[offset] = color;
					vram32[offset + 1] = color;
					vram32[offset + bufferwidth] = color;
					vram32[offset + bufferwidth + 1] = color;
				}
#else
				uint32_t color = (font & 0x80) ? fg_col : bg_col;
				vram32[offset] = color;
				vram32[offset + 1] = color;
				vram32[offset + bufferwidth] = color;
				vram32[offset + bufferwidth + 1] = color;
#endif
				font <<= 1;
				offset+=2;
			}
		}
	}
	return x;
}

int blit_string_ctr(int sy, const char *msg)
{
	int sx = 960 / 2 - strlen(msg) * (16 / 2);

	return blit_string(sx, sy, msg);
}

int blit_string_ctrf(int sy, const char *msg, ...)
{
	va_list list;
	char string[512];

	va_start(list, msg);
	vsprintf(string, msg, list);
	va_end(list);

	int sx = 960 / 2 - strlen(string) * (16 / 2);

	return blit_string(sx, sy, string);
}

int blit_stringf(int sx, int sy, const char *msg, ...)
{
	va_list list;
	char string[512];

	va_start(list, msg);
	vsprintf(string, msg, list);
	va_end(list);

	return blit_string(sx, sy, string);
}

int blit_string_colorf(int sx, int sy, int color, const char *msg, ...)
{
	va_list list;
	char string[512];

	va_start(list, msg);
	vsprintf(string, msg, list);
	va_end(list);

	int fg = fcolor;
	int bg = bcolor;

	blit_set_color(color, bcolor);

	int ret = blit_string(sx, sy, string);

	blit_set_color(fg, bg);

	return ret;
}

int blit_string_color_centerf(int sy, int color, const char *msg, ...)
{
	va_list list;
	char string[512];

	va_start(list, msg);
	vsprintf(string, msg, list);
	va_end(list);

	int fg = fcolor;
	int bg = bcolor;

	blit_set_color(color, bcolor);

	int sx = 960 / 2 - strlen(string) * (16 / 2);

	int ret = blit_string(sx, sy, string);

	blit_set_color(fg, bg);

	return ret;
}

void blit_display_init()
{
	if (!displayblockinit) {
		displayblockinit = 1;
		displayblock = sceKernelAllocMemBlock("display", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, 2 * 1024 * 1024, NULL);
	}

	sceKernelGetMemBlockBase(displayblock, (void**)&psvFrameBuf.base);

	framebuf.size = sizeof(framebuf);
	framebuf.base = psvFrameBuf.base;
	framebuf.pitch = 960;
	framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
	framebuf.width = 960;
	framebuf.height = 544;

	sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);
}

void blit_clear_screen()
{
	int i;

	// :|
	for (i = 0; i < 34; i++)
		blit_string(0, i * 16, "                                                            ");
}
