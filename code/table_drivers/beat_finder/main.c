// http://stackoverflow.com/questions/604453/analyze-audio-using-fast-fourier-transform
// http://www.gamedev.net/reference/programming/features/beatdetection/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h> 
#include <pthread.h>

#include "main.h"
#include "fft.h"
#include "draw.h"
#include "table.h"
#include "serial.h"

unsigned char use_gui = TRUE;
unsigned char use_serial = TRUE;

double clip_mag = 0;        // dynamic magnitude clip
double clip_mag_decay = 0;   // dynamio clip decreases at rate of some function, this indexes the function
char clipped = 0;            // 1 = we clipped a bin this loop

// Mag: 0.170000 Var: 0.220000
// default trigger levels for detecting beats
double MAG_TRIGGER=    .82; //.36
double VAR_TRIGGER=    .83; //.36

struct bin fft_bin[FFT_NUM_BINS];

double fft_global_mag_avg;
double fft_global_mag_max;
double fft_global_hist_mag_avg;     // average of all the bin history averages
double fft_global_hist_mag_max;     // max value of global history
double fft_global_hist_std_avg;     // avg of all the std deviations
double fft_global_hist_std_max;     // max of all the std deviations

char new_data = 0;

struct light lights[NUM_LIGHTS];

int i,j,k = 0;

void hsv_to_rgb( int h, float s, float v, int *r, int *g, int *b)
{
    float rr, gg, bb;

    float c = v * s;
    float hh = h / 60.0f;
    float x = c * (1 - fabs(fmod(hh,2.0f) - 1));

    if (hh >= 0 && hh < 1)      { rr = c; gg = x; bb = 0; }
    else if (hh >= 1 && hh < 2) { rr = x; gg = c; bb = 0; }
    else if (hh >= 2 && hh < 3) { rr = 0; gg = c; bb = x; }
    else if (hh >= 3 && hh < 4) { rr = 0; gg = x; bb = c; }
    else if (hh >= 4 && hh < 5) { rr = x; gg = 0; bb = c; }
    else if (hh >= 5 && hh < 6) { rr = c; gg = 0; bb = x; }
    else { rr = 0; gg = 0; bb = 0; }

    float m = v - c;

    rr += m;
    gg += m;
    bb += m;

    *r = (int)(rr * 255.0f);
    *g = (int)(gg * 255.0f);
    *b = (int)(bb * 255.0f);
    
}


void init_lights(void)
{
    for (i=0; i<NUM_LIGHTS; i++)
    {
        lights[i].state = 0;
        lights[i].decay = 0;
        lights[i].last_bin = -1;
    }
}

void detect_beats(void)
{
    for (i = 0; i < FFT_NUM_BINS; i++)
    {
        // shift trigger history down
        for (k=1; k < HIST_SIZE; k++)
        {
            fft_bin[i].trigger_hist[k-1] = fft_bin[i].trigger_hist[k];
        }

        // see if we detect a beat
        if (fft_bin[i].mag/fft_global_mag_max > MAG_TRIGGER && fft_bin[i].hist_std/fft_global_hist_std_max > VAR_TRIGGER)
            fft_bin[i].triggered = 1;
        else
            fft_bin[i].triggered = 0;

        // if this bin is decreasing from last time it is no longer a beat
        //if (fft_bin_diff[i] <= 0)
        //    fft_bin_triggered[i] = 0;

        // add current trigger state to hist buffer
        fft_bin[i].trigger_hist[HIST_SIZE-1] = fft_bin[i].triggered;
    }
}


void assign_lights(void)
{
    int pulse_count = 0;
    int center_of_pulse = 0;

    // finds how many groups of pulses there are
    // marks the center of them
    for (i=1; i<FFT_NUM_BINS; i++)
    {
        // not a pulse until proved otherwise        
        fft_bin[i].is_pulse = 0;

        // if this one is triggered and the previous one isn't we found start of group
        if (fft_bin[i].triggered && !fft_bin[i-1].triggered)
        {
            pulse_count++;
            center_of_pulse = i;

        }
        // if it is not triggered but the last one is we found end of group
        else if (!fft_bin[i].triggered && fft_bin[i-1].triggered)
        {
            center_of_pulse = (i-center_of_pulse) / 2  + center_of_pulse;
            fft_bin[center_of_pulse].is_pulse = 1;
        }

        // skip grouping logic, just count every one
        //fft_bin_pulse[i] = fft_bin_triggered[i];
    }

    //printf("pulse_count: %d\n", pulse_count);


    // go through groups of pulses and map them to lights
    // a light can only trigger if either:
    //     1. we find a pulse that is same place as last time for this light
    //     2. the light decay is zero, meaning it has not
    //        had a pulse in a while so we should pulse it asap

    // assume were not going to find any pulses
    for (i=0; i<NUM_LIGHTS; i++) { lights[i].found_pulse = 0; }

    // first find pulses that aleady have a light assigned
    for (i=0; i<NUM_LIGHTS; i++)
    {
        for (j=0; j<FFT_NUM_BINS; j++)
        {
            // check if we found a pulse that is in the same spot
            if (fft_bin[j].is_pulse && lights[i].last_bin ==  j)
            {
                // we found a pulse for this light
                lights[i].found_pulse = 1;

                // reset the decay
                pulses[i].decay = LIGHT_DECAY;
                lights[i].decay = LIGHT_DECAY;

                // turn light on
                lights[i].state = 1;
               
                // clear this pulse since we just handled it
                fft_bin[j].is_pulse = 0;

                // stop looking for pulses, go to next light
                break;
            }
        }
    }


    // take left over pulses and assign them to empty lights
    for (i=0; i<NUM_LIGHTS; i++)
    {
        // loop through all the pulses
        for (j=0; j<FFT_NUM_BINS; j++)
        {
            // check if we found an empty light 
            if (fft_bin[j].is_pulse && lights[i].decay == 0) 
            {
                // we found a pulse for this light
                lights[i].found_pulse = 1;

                // calculate new position and color
                pulses[i].x = (int)(((float)rand() * (float)(TABLE_WIDTH-2) / (RAND_MAX - 1.0)) + 1.0)+1;
                pulses[i].y = (int)(((float)rand() * (float)(TABLE_HEIGHT-2) / (RAND_MAX - 1.0)) + 1.0)+1;
                int color = (int)(((float)rand() * 360.0 / (RAND_MAX - 1.0)) + 1.0);
                hsv_to_rgb(color, 1,1, &pulses[i].r, &pulses[i].g, &pulses[i].b);

                // reset the decay
                pulses[i].decay = LIGHT_DECAY;
                lights[i].decay = LIGHT_DECAY;

                // turn light on
                lights[i].state = 1;

                // save which bin this was
                lights[i].last_bin = j;

                // clear this pulse since we just handled it
                fft_bin[j].is_pulse = 0;

                // stop looking for pulses, go to next light
                break;
            }
        }
    }

    // find all the lights that did not get assigned a pulse
    for (i=0; i<NUM_LIGHTS; i++)
    {
        // we this light has a pulse go to next one
        if (lights[i].found_pulse) continue;

        // turn the light off
        lights[i].state = 0;

        // decrement the decay for this light
        if (lights[i].decay == 0)
            // once the decay dies this light no long belongs to a bin
            lights[i].last_bin = -1;
        else
            lights[i].decay -= 1;

        // decrement the decay for this pulse
        pulses[i].decay -= 1;
        if (pulses[i].decay < 0) pulses[i].decay = 0;

        // when there is a heavy bass line we want to turn on as many lights as possible
        // also when there is a heavy bass line we will probably be clipping it.
        // we didn't find a pulse for this light
        // we clipped a bin
        // this light is almost free to trigger
        if (clipped && lights[i].decay < LIGHT_DECAY / 2)
            lights[i].state = 1;
    }
}



int main( int argc, char **argv )
{
    if ( use_gui )
    {
        printf("init_sdl()\n");
        if ( init_sdl() ) return 1;

        printf("init_gl()\n");
        init_gl();
    }

    printf("init_fft()\n");
    if ( init_fft() ) return 1;

    printf("init_alsa()\n");
    if ( init_alsa() ) return 1;

    if ( use_serial )
    {
        printf("init_serial()\n");
        if ( init_serial() ) use_serial = FALSE;
    }

    init_lights();
    init_table();

    pthread_t sample_thread;

    pthread_create(&sample_thread, NULL, &get_samples, NULL);

    while ( !done )
    {

        // check to see if we have a new sample
        if (new_sample)
        {
            // we are going to process this sample, it is no longer new
            pthread_mutex_lock(&sample_mutex);
            new_sample--;
            if (new_sample > 0) printf("# Unprocessed samples: %d\n", new_sample);
            pthread_mutex_unlock(&sample_mutex);

            do_fft();

            detect_beats();

            assign_lights();

            assign_cells();

            if ( use_gui )
            {
                if (handle_sdl_events()) return 1;
                draw_all();
            }

            if ( use_serial ) send_serial();
        }

        usleep(5000);
    }

    return 0;
}
