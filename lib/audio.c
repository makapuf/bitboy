// audio generatiuon through DAC 1
#include <stdint.h>
#include "stm32f4xx.h"
#include "audio.h"

// XXX use double buffering ? DMA ?
// explicit the framerate !
uint8_t audio_buffer[BITBOX_SNDBUF_LEN]; // one sample per line
uint8_t *audio_ptr; // current sample to play 
uint8_t audio_on;

void audio_init()
{
	audio_on = 0;

	// enable DAC clock on APB1
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;
	// enable GPIOA clock on APB2
	RCC->APB2ENR |= RCC_AHB1LPENR_GPIOALPEN;

	/* Configure PA.04 (DAC) as Analog */

	// Set GPIOA pin X as ANALOG
	GPIOA->MODER |= GPIO_MODER_MODER4_0 | GPIO_MODER_MODER4_1;

	// Useful ? not that fast.
	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR4_1; // 50Mhz = 10 set bit 1
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR4_0); // 50Mhz = 10 reset bit 0

	// enables DAC out. Automatically setup pin to DAC output
	DAC->CR = DAC_CR_EN1; 
	// clear TEN1 for immediate value change

	audio_ptr = audio_buffer; 

}


void audio_out8(uint8_t value)
{
	// outputs value to DAC, value 0-255
	DAC->DHR8R1 = (uint32_t) value;
}



void audio_frame()
{
	if (audio_on) {
		// XXX switch buffers
		audio_ptr = audio_buffer;
		game_snd_buffer(audio_buffer,BITBOX_SNDBUF_LEN); 
	}
}

// --------------    ultra simple 1-voice, non tuned, non looped sampler ~ 16khz samples
/// FIXME !

int sample_id=0; // pos. in sample
Sample *sample;
void audio_start_sample(Sample *s)
{
	sample=s;
	sample_id=0;
}

/*
// fill buffer with sample
void audio_play_sample( uint8_t *buffer, int len )
{
	if (!sample) return;
	// every other sample (ie reduce by two the sampling frequency)
	// FIXME !!

	for (int i=0;i<min(len, sample->length*2),i++)
	{
		buffer[i]=sample_id[i+sample_id]
	}
	for (int i=min(len, sample->length*2))
	{
		buffer[i]=0;
	}
}

void audio_tri1k() 
{
	audio_out8(8*(sample_id++)&31);
}

*/