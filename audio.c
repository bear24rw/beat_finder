#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

#include "main.h"
#include "fft.h"
#include "audio.h"

FILE *fifo_file;

snd_pcm_t *handle;
snd_pcm_hw_params_t *params;
snd_pcm_uframes_t frames = 1024;

int rc;
int size;
unsigned int rate = 44100;
int dir = 0;
unsigned char *buffer; // 2 bytes / sample, 2 channels

uint16_t tmp_buffer[940];

pthread_mutex_t sample_mutex = PTHREAD_MUTEX_INITIALIZER;
char new_sample = 0;
int missed_samples = 0;

int init_mpd(void)
{
    fifo_file = fopen(FIFO_FILE, "rb");

    if (!fifo_file)
    {
        printf("Could not open mpd fifo file\n");
        return 1;
    }

    return 0;
}


void list_cards(void)
{
    snd_ctl_card_info_t *p_info = NULL;
    snd_ctl_card_info_alloca(&p_info);

    snd_pcm_info_t *p_pcminfo = NULL;
    snd_pcm_info_alloca(&p_pcminfo);

    printf("Availible alsa capture devices:\n");

    int i_card = -1;
    while (!snd_card_next(&i_card) && i_card >= 0)
    {
        char devname[10];
        snprintf( devname, 10, "hw:%d", i_card );

        snd_ctl_t *p_ctl = NULL;
        if ( snd_ctl_open( &p_ctl, devname, 0 ) < 0) continue;

        snd_ctl_card_info( p_ctl, p_info);
        printf("\t%s (%s)\n", snd_ctl_card_info_get_id(p_info), snd_ctl_card_info_get_name(p_info));

        int i_dev = -1;
        while (!snd_ctl_pcm_next_device(p_ctl, &i_dev) && i_dev >= 0)
        {
            snd_pcm_info_set_device(p_pcminfo, i_dev);
            snd_pcm_info_set_subdevice(p_pcminfo, 0);
            snd_pcm_info_set_stream(p_pcminfo, SND_PCM_STREAM_CAPTURE);

            if (snd_ctl_pcm_info(p_ctl, p_pcminfo) < 0) continue;

            printf("\t\thw:%d,%d : %s (%s)\n", i_card, i_dev, 
                    snd_pcm_info_get_id(p_pcminfo), 
                    snd_pcm_info_get_name(p_pcminfo));
        }

        snd_ctl_close(p_ctl);
    }

}

int init_alsa(void)
{
    list_cards();

    // open PCM device for recording
    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);

    if (rc < 0)
    {
        printf("Unable to open pcm device: %s\n", snd_strerror(rc));
        return 1;
    }
    else
        printf("Using %s\n", snd_pcm_name(handle)); 

    // allocate a hardware parameter object
    snd_pcm_hw_params_alloca(&params);

    if (rc < 0)
    {
        printf("Unable to configure this PCM device\n");
        return 1;
    }

    // fill it with default values
    snd_pcm_hw_params_any(handle, params);

    //
    // set the desired hardware parameters
    //

    // interleaved mode
    rc = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (rc < 0)
    {
        printf("Unable to set interleaved mode\n");
        return 1;
    }

    // signed 16-bit little-endian format
    rc = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U16_BE);
    if (rc < 0)
    {
        printf("Unable to set format\n");
        return 1;
    }

    // two channel stereo
    rc = snd_pcm_hw_params_set_channels(handle, params, 2);
    if (rc < 0)
    {
        printf("Unable to set to two channel stereo\n");
        return 1;
    }

    // 44100 bits/second sample rate
    unsigned int exact_rate = rate;
    rc = snd_pcm_hw_params_set_rate_near(handle, params, &exact_rate, 0);
    if (rc < 0)
    {
        printf("Error setting rate\n");
        return 1;
    }

    if (rate != exact_rate)
    {
        printf("%d Hz not supported. Using %d Hz instead", rate, exact_rate);
    }

    // set period size to 32 frames
    rc = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    if (rc < 0)
    {
        printf("Error setting period size\n");
        return 1;
    }

    // write the parameters to the driver
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0)
    {
        printf("Unable to set hw parameters: %s\n", snd_strerror(rc));
        return 1;
    }

    size = frames * 4; // 2 bytes / sample, 2 channels
    buffer = (unsigned char *) malloc(size);

    printf("Frames: %d\n", (int)frames);
    printf("Allocating buffer of size: %d\n", size);

    snd_pcm_hw_params_get_period_time(params, &rate, &dir);

    return 0;

}

void get_alsa(void)
{
    rc = snd_pcm_readi(handle, buffer, frames);

    if (rc == -EPIPE)
    {
        printf("Overrun occurred\n");
        snd_pcm_prepare(handle);
    }
    else if (rc < 0)
    {
        printf("Error from read: %s\n", snd_strerror(rc));
    }
    else if (rc != (int)frames)
    {
        printf("Short read. Only read %d frames, expected %d\n", rc, (int)frames);
    }
    else if (rc == (int)frames)
    {
        pthread_mutex_lock(&sample_mutex);

        // shift old buffer over to make room for new stuff
        for (i = 0; i < SAMPLE_SIZE - rc; i++)
            fft_input[i] = fft_input[i+rc];

        int frame = 0;
        for (frame=0; frame<rc; frame++)
        {
            int left = (buffer[frame*4+0] << 8) | buffer[frame*4+1];
            int right = (buffer[frame*4+2] << 8) | buffer[frame*4+3];

            fft_input[frame+(SAMPLE_SIZE-rc)] = (double)(left + right) / 1;
        }
       
        // if new_sample is still set it probably means we haven't processed it
        if (new_sample) missed_samples++;
        else if (new_sample == 0) missed_samples = 0;

        // we have a new sample
        new_sample = 1;

        pthread_mutex_unlock(&sample_mutex);
    }
}

void get_mpd(void)
{
    static int16_t buf[SAMPLE_SIZE];

    for (i = 0; i < SAMPLE_SIZE; i++) buf[i] = 0;

    int data = fread(buf, sizeof(int16_t), SAMPLE_SIZE, fifo_file);

    if (data != SAMPLE_SIZE)
    {
        printf("WRONG SAMPLE SIZE: %d!\n", data);
    }
    else
    {
        // cast the int16 array into a double array
        for (i = 0; i < SAMPLE_SIZE; i++) fft_input[i] = (double)buf[i];

        pthread_mutex_lock(&sample_mutex);
        new_sample++;
        pthread_mutex_unlock(&sample_mutex);
    }
}

void *get_samples(void)
{
    while(1)
    {
        get_alsa();

        //get_mpd();

        // find average of the input signal
        fft_input_avg = 0;
        for (int i = 0; i < SAMPLE_SIZE; i++)
            fft_input_avg += fft_input[i];
        fft_input_avg /= SAMPLE_SIZE;


    }
}
