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

#ifndef TABLE_H
#define TABLE_H

#define TABLE_WIDTH         16
#define TABLE_HEIGHT        8
#define CELL_SIZE           25

#define PULSE_RADIUS        2
#define PULSE_CLIP_SCALE    1.5

struct pixel
{
    unsigned char r,g,b;
    int h,s,v;
}; 

struct pulse
{
    int x,y,radius;
    int r,g,b;
    int decay;
};

extern struct pixel table[TABLE_WIDTH][TABLE_HEIGHT];
extern struct pixel tmp_table[TABLE_WIDTH][TABLE_HEIGHT];

extern struct pulse pulses[NUM_LIGHTS];

extern unsigned char offset_circle;
extern unsigned char first_assigned;
extern unsigned char pulse_pulses;

void init_table(void);
void clear_table(void);
void clear_tmp_table(void);
void draw_pulse(int);
void increase_table_bg(float);
void draw_table_bg(void);
void assign_cells(void);

#endif
