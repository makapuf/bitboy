#include "../lib/system.h"
#include "../lib/kernel.h"

#include "gamepad.h"


void init_led()
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable gpioA
    GPIOA->MODER |= (1 << 16) ; // set pin 8 to be general purpose output
}

void toggle_led()
{
	GPIOA->ODR ^= 1<<8; // led on/off
}


void die(char *msg, ...)
{
	// ultra minimalistic, still allows traces & debug of msg 
	// consider morse code ?
	while (1) 
	{
		toggle_led();
		for (int i=0;i<168000000;i++) {};
	}
}

void main()
{
	InitializeSystem();
	vga640_setup();
	init_led();

	#ifdef GAMEPAD
	gamepad_init();
	#endif

	#ifdef AUDIO
	audio_init();
	#endif
	
	uint32_t oframe;

	game_init();
	while (1)
	{

		game_frame();

		
		// wait next frame
		oframe=vga_frame;
		while (oframe==vga_frame);

		
		if (vga_frame%32 == 0) toggle_led(); // each second

	}; // all work done inside interrupts
}