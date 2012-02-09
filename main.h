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

#ifndef MAIN_H
#define MAIN_H

#include <fftw3.h>

#define TRUE            1
#define FALSE           0

#define SAMPLE_SIZE     1024
#define SAMPLE_RATE     44100
#define FFT_SIZE        (SAMPLE_SIZE / 2)
#define MAX_FREQ        8000
#define HIST_SIZE       120
#define FREQ_PER_BIN    (SAMPLE_RATE / SAMPLE_SIZE)
#define FFT_NUM_BINS    (MAX_FREQ/FREQ_PER_BIN)
#define FFT_BIN_WIDTH   10              // pixel width of bin

// magnitude clip
#define USE_CLIP        TRUE
#define USE_CLIP_DYN    TRUE            // 1 = use dynamic clip, 0 = use static
#define CLIP_STATIC_MAG 800             // static clip magnitude

#define MAG_SCALE       4000//10000

#define NUM_LIGHTS      (2*4)           // how many lights to display
#define LIGHT_DECAY     (HIST_SIZE*2)   // cycles until light is clear to trigger again
#define LIGHT_SIZE      50              // pixel size
#define LIGHT_SPACING   20              // pixels between groups

extern double clip_mag;
extern double clip_mag_decay;
extern char clipped;        

extern double MAG_TRIGGER; 
extern double VAR_TRIGGER; 

struct bin
{
    double mag;
    double last_mag;
    double diff;
    
    double hist[HIST_SIZE];
    double hist_avg;
    double hist_std;

    char triggered;
    char trigger_hist[HIST_SIZE];

    char is_pulse;
};

extern struct bin fft_bin[FFT_NUM_BINS];

extern double fft_global_mag_avg;
extern double fft_global_mag_max;
extern double fft_global_hist_mag_avg;  // average of all the bin history averages
extern double fft_global_hist_mag_max;  // max value of global history
extern double fft_global_hist_std_avg;  // avg of all the std deviations
extern double fft_global_hist_std_max;  // max of all the std deviations

extern char new_data; // flag to signal that we have new data

struct light
{
    unsigned char state;    // 1 = 0n
    int last_bin;           // keep track of which bin this light was last assigned to
    int decay;              // keep track of how long its been since this light was triggered
    char found_pulse;       // flag to see if we found a pulse during each loop
};

extern struct light lights[NUM_LIGHTS];                
extern int lights_last_bin[NUM_LIGHTS];    
extern int lights_time_decay[NUM_LIGHTS];

extern int i,j,k;


void hsv_to_rgb( int h, float s, float v, int *r, int *g, int *b);

#endif
