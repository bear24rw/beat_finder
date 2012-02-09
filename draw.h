/***************************************************************************
 *   Copyright (C) 2010-2012 by Max Thrun                                  *
 *   bear24rw@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#ifndef DRAW_H
#define DRAW_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#define SCREEN_BPP 16

extern unsigned char done;

void init_gl(void);
int init_sdl(void);
void resize_window(int, int);
void service_keys(void);
void draw_table(int, int);
void draw_real_img_plot(float, float);
void draw_mag(int, float, float, float);
void draw_mag_hist_avg(int, float, float);
void draw_mag_hist_var(int, float, float);
void draw_mag_hist(int, float, float);
void draw_lines_to_lights(void);
void draw_lights(void);
int draw_all(void);
int handle_sdl_events(void);

#endif
