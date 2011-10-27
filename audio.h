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
