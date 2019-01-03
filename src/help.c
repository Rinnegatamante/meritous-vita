//
//   help.c
//
//   Copyright 2007, 2008 Lancer-X/ASCEAI
//
//   This file is part of Meritous.
//
//   Meritous is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Meritous is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Meritous.  If not, see <http://www.gnu.org/licenses/>.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <string.h>
#include <vitasdk.h>

#include "levelblit.h"

struct help_line {
	char *t;
};

struct help_section {
	int lines;
	char *identifier;
	struct help_line *l[256];
};

struct help_file {
	int sections;
	struct help_section *s[256];
};

struct help_file *hlp = NULL;
int my_line;
int my_sec;
int my_cursor;
int my_link;
void InitHelp()
{
	FILE *fp;
	struct help_section *current_sec = NULL;
	struct help_line *current_line = NULL;
	char linebuf[80];
	hlp = malloc(sizeof(struct help_file));
	hlp->sections = 0;
	
	fp = fopen("ux0:data/meritous/dat/d/helpfile.txt", "r");
	while (!feof(fp)) {
		fgets(linebuf, 79, fp);
		if (linebuf[strlen(linebuf)-1] == '\n')
			linebuf[strlen(linebuf)-1] = 0;
			
		if (linebuf[0] == '\'') {
			// comment
			continue;
		}
		if (linebuf[0] == ':') {
			// section
			hlp->s[hlp->sections] = malloc(sizeof(struct help_section));
			current_sec = hlp->s[hlp->sections];
			hlp->sections++;
			current_sec->identifier = malloc(strlen(linebuf));
			current_sec->lines = 0;
			strcpy(current_sec->identifier, linebuf+1);
			continue;
		}
		
		// different line
		if (current_sec != NULL) {
			current_sec->l[current_sec->lines] = malloc(sizeof(struct help_line));
			current_line = current_sec->l[current_sec->lines];
			current_sec->lines++;
			current_line->t = malloc(strlen(linebuf)+1);
			strcpy(current_line->t, linebuf);
		}
	}
	fclose(fp);
}

void DisplayHelp()
{
	static int tick = 0;
	int i;
	struct help_section *current_sec = NULL;
	char *ltext;
	char c_ident[20];
	int line_num;
	int follow_link = 0;
	char linkfollow[20] = "";
	
	DrawRect(23, 23, 594, 434, 0);
	DrawRect(24, 24, 592, 432, 200);
	DrawRect(25, 25, 590, 430, 255);
	DrawRect(26, 26, 588, 428, 200);
	DrawRect(27, 27, 586, 426, 100);
	DrawRect(30, 30, 580, 420, 20);
	DrawRect(35, 35, 570, 410, 60);
	
	// 70x40 display
	current_sec = hlp->s[my_sec];
	
	my_line = my_cursor - 19;
	if (my_line < 0) my_line = 0;
	if (my_line >= (current_sec->lines)) my_line = current_sec->lines - 1;
	for (i = 0; i < 2; i++) {
		draw_text(23+i, 40+(my_cursor - my_line)*10, "->", 255);
		draw_text(599+i, 40+(my_cursor - my_line)*10, "<-", 255);
	}
	
	for (i = 0; i < 40; i++) {
		line_num = my_line + i;
		if (line_num >= 0) {
			if (line_num < current_sec->lines) {
				ltext = current_sec->l[line_num]->t;
				
				switch (ltext[0]) {
					case '!':
					
						draw_text(40 + (560-strlen(ltext+1)*8)/2, 40+i*10, ltext+1, 255);
						break;
					case '?':
						strncpy(c_ident, ltext+1, strchr(ltext+1, '?')-ltext-1);
						c_ident[strchr(ltext+1, '?')-ltext-1] = 0;
						
						draw_text(40, 40+i*10, strchr(ltext+1, '?')+1, my_cursor == line_num ? 200+(tick%16)*3 : 150);
						if ((my_link == 1)&&(my_cursor == line_num)) {
							follow_link = 1;
							strcpy(linkfollow, c_ident);
						}
						break;
					default:
						draw_text(40, 40+i*10, ltext, 200);
						break;
				}
			}
		}
	}
	tick++;
	SDL_Flip(screen);
	
	if (follow_link) {
		for (i = 0; i < hlp->sections; i++) {
			if (strcmp(linkfollow, hlp->s[i]->identifier) == 0) {
				my_sec = i;
				my_cursor = 0;
				break;
			}
		}
		my_link = 0;
	}
}

int MoveCursor()
{
	SDL_Event ev;
	static int key_delay = 0;
	static int key_up = 0, key_down = 0;
	
	if (key_delay > 0) key_delay--;
	
	my_link = 0;
	
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);
	if (pad.buttons & SCE_CTRL_DOWN) {
		key_down = 1;
		key_delay = 10;
		if (my_cursor < hlp->s[my_sec]->lines-1) my_cursor++;
	} else {
		key_down = 0;
	}
	if (pad.buttons & SCE_CTRL_UP) {
		key_up = 1;
		key_delay = 10;
		if (my_cursor > 0) my_cursor--;
	} else {
		key_up = 0;
	}
	if (pad.buttons & SCE_CTRL_SELECT) {
		return 0;
	}
	if (pad.buttons & SCE_CTRL_CROSS) {
		my_link = 1;
	}
	
	while (SDL_PollEvent(&ev)) {	
		if (ev.type == SDL_QUIT) {
			return 0;
		}
	}
	
	if (key_delay == 0) {
		if (key_up == 1) {
			if (my_cursor > 0) my_cursor--;
		}
		if (key_down == 1) {
			if (my_cursor < hlp->s[my_sec]->lines-1) my_cursor++;
		}
	}
	
	return 1;
}

void ShowHelp()
{
	int in_help = 1;
	if (hlp == NULL) {
		InitHelp();
	}
	my_line = 0;
	my_sec = 0;
	my_cursor = 0;
	my_link = 0;
	
	while (in_help)
	{
		DisplayHelp();
		in_help = MoveCursor();
		SDL_Delay(30);
	}
}
