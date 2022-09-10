/*
	Copyright (C) 2017, qwikrazor87

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/power.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "blit.h"

uint32_t cock_blocker_version_major = 1, cock_blocker_version_minor = 1;

#define GREEN 0x00007F00
#define BLUE 0x007F3F1F
#define PURPLE 0x007F1F7F
#define ORANGE 0xFF007FFF

extern SceDisplayFrameBuf framebuf;

char filePassword[128], passwordStrBuf[128], *passwordStr = (char *)passwordStrBuf;
int ispassword = 0, passwordSize = 0;
const char *passwordPath = "ux0:tai/c0ck_bl0cker.bin";

SceCtrlData pad;
int lastbutton = 0;
int cancel = 0, enter = 0;

void getpassword()
{
	while (1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);

		if (lastbutton != pad.buttons) {
			passwordSize = (passwordStr - (char *)passwordStrBuf);

			if (pad.buttons & SCE_CTRL_SELECT) {
				cancel = 1;

				passwordStr = (char *)passwordStrBuf;
				memset(passwordStr, 0, 128);

				break;
			} else if ((pad.buttons & SCE_CTRL_START) && (passwordSize > 0)) {
				enter = 1;

				break;
			} else if (pad.buttons & SCE_CTRL_LTRIGGER) {
				if (passwordSize > 0) {
					*--passwordStr = 0;
					blit_clear_screen();
				}
			} else if (pad.buttons & SCE_CTRL_UP)
				*passwordStr++ = 'U';
			else if (pad.buttons & SCE_CTRL_LEFT)
				*passwordStr++ = 'L';
			else if (pad.buttons & SCE_CTRL_RIGHT)
				*passwordStr++ = 'R';
			else if (pad.buttons & SCE_CTRL_DOWN)
				*passwordStr++ = 'D';
			else if (pad.buttons & SCE_CTRL_TRIANGLE)
				*passwordStr++ = 'T';
			else if (pad.buttons & SCE_CTRL_SQUARE)
				*passwordStr++ = 'S';
			else if (pad.buttons & SCE_CTRL_CIRCLE)
				*passwordStr++ = 'O';
			else if (pad.buttons & SCE_CTRL_CROSS)
				*passwordStr++ = 'X';
		}

		blit_setup();
		blit_string_color_centerf(32, 0xFF007FFF, "C0ck Bl0cker v%d.%d", cock_blocker_version_major, cock_blocker_version_minor);
		blit_string_colorf(0, 64, 0xFFFFFFFF, "Allowed buttons for password: U L D R /\\ [] X O");
		blit_string_colorf(0, 80, 0xFFFFFFFF, "L trigger - delete last password entry");
		blit_string_colorf(0, 96, 0xFFFFFFFF, "Start - Accept password");
		blit_string_colorf(0, 112, 0xFFFFFFFF, "Select - Exit");
		blit_string_colorf(0, 128, 0xFF00FF00, "%s password: %s", ispassword ? "Enter current" : "Enter new", passwordStrBuf);

		sceDisplayWaitVblankStart();
		sceKernelDelayThread(0);

		lastbutton = pad.buttons;
	}

	while (pad.buttons & (SCE_CTRL_SELECT | SCE_CTRL_START))
		sceCtrlPeekBufferPositive(0, &pad, 1);
}

int _start(SceSize args, void *argp)
{
	int i, passwordSizeFixed = 0;

	SceUID fd = sceIoOpen(passwordPath, SCE_O_RDONLY, 0777);

	if (fd >= 0) {
		passwordSize = sceIoRead(fd, filePassword, 128);
		sceIoClose(fd);

		passwordSizeFixed = passwordSize;

		for (i = 0; i < passwordSize; i++)
			filePassword[i] ^= 0x87;

		ispassword = 1;
	}

	//let's stretch those little arms
	SceUInt64 SceLongArmOfTheVita = scePowerGetArmClockFrequency();
	scePowerSetArmClockFrequency(444);

	memset(&pad, 0, sizeof(SceCtrlData));

	blit_display_init();
	blit_set_color(0xFFFFFFFF, 0x00000000);

	memset(passwordStr, 0, 128);

	while (1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);

		if ((lastbutton != pad.buttons) || enter) {
			passwordSize = (passwordStr - (char *)passwordStrBuf);

			if (pad.buttons & SCE_CTRL_SELECT)
				sceKernelExitProcess(0);
			else if (((pad.buttons & SCE_CTRL_START) || enter) && (passwordSize > 0)) {
				enter = 0;

				if (ispassword) {
					if (!memcmp(filePassword, passwordStrBuf, passwordSizeFixed)) {
						blit_setup();
						blit_string_colorf(0, 144, 0xFF00FF00, "Password matched");
						sceDisplayWaitVblankStart();
						sceKernelDelayThread(1000000 * 3);

						break;
					} else {
						blit_setup();
						blit_string_color_centerf(144, 0xFF0000FF, "You've been c0ck bl0cked!");
						sceDisplayWaitVblankStart();
						sceKernelDelayThread(1000000 * 3);

						sceKernelExitProcess(0);
					}
				} else {
					blit_setup();
					blit_string_colorf(0, 144, 0xFF00FF00, "new password is: %s", passwordStrBuf);
					sceDisplayWaitVblankStart();
					sceKernelDelayThread(1000000 * 5);

					for (i = 0; i < passwordSize; i++)
						passwordStrBuf[i] ^= 0x87;

					fd = sceIoOpen(passwordPath, SCE_O_CREAT | SCE_O_WRONLY | SCE_O_TRUNC, 0777);
					sceIoWrite(fd, passwordStrBuf, passwordSize);
					sceIoClose(fd);

					break;
				}
			} else if (pad.buttons & SCE_CTRL_RTRIGGER) {
				blit_clear_screen();
				getpassword();
				blit_clear_screen();
			}
		}

		blit_setup();
		blit_string_color_centerf(32, 0xFF007FFF, "C0ck Bl0cker v%d.%d", cock_blocker_version_major, cock_blocker_version_minor);

		if (!enter)
			blit_string_colorf(0, 64, 0xFFFFFFFF, "Press R trigger to enter password");

		sceDisplayWaitVblankStart();
		sceKernelDelayThread(0);

		lastbutton = pad.buttons;
	}

	scePowerSetArmClockFrequency(SceLongArmOfTheVita);

	return 0;
}
