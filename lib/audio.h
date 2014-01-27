#pragma once

#include <stdint.h>

#define BITBOX_SAMPLERATE 31469 
#define BITBOX_SNDBUF_LEN (BITBOX_SAMPLERATE/60) // 524

typedef struct Sample
{
    unsigned int length; // size in number of samples
    unsigned int sample_rate; // sampling freq
    uint8_t *data;
} Sample;

extern uint8_t audio_on; 

void audio_init();
void audio_out8(uint8_t value); // deprecated


extern uint8_t *audio_ptr; // only used through this inline
inline void audio_line()
{	
	if (audio_on) audio_out8(*audio_ptr++);
}

void audio_frame(); // will call audio callback

void game_snd_buffer(uint8_t *buffer, int len); // user provided

// ultra simple mono sample player
void audio_start_sample(Sample *);
void audio_play_sample();

/* sample values */
void audio_tri1k();
