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

#ifndef AUDIO_H
#define AUDIO_H

#define FIFO_FILE        "/tmp/mpd.fifo"

extern double fft_input_avg;

extern pthread_mutex_t sample_mutex;
extern char new_sample;
extern int missed_samples;

int init_mpd(void);
int init_alsa(void);
void list_cards(void);
void *get_samples();

#endif
