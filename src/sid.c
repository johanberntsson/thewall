/* Based on sid.c from Frank Buss,
 * http://www.frank-buss.de/c64/sid-test.html
 */

#include <stdio.h>
#include <stdint.h>
 
uint16_t g_freq[8] = {
	0x22cd,
	0x2710,
	0x2bd8,
	0x2e74,
	0x3424,
	0x3a87,
	0x41b2,
	0x459a
};
 
uint8_t* g_sidBase = (uint8_t*) 0xd400;
 
void setVolume(uint8_t volume)
{
	g_sidBase[24] = volume | (1 << 4);
}
 
void initSid()
{
	uint8_t i = 0;
	for (i = 0; i < 24; i++) g_sidBase[i] = 0;
	setVolume(15);
}
 
void setFrequency(uint8_t voice, uint16_t freqIndex)
{
	uint16_t freq = g_freq[freqIndex];
	g_sidBase[7 * voice] = freq & 0xff;
	g_sidBase[7 * voice + 1] = freq >> 8;
}
 
void setAdsr(uint8_t voice, uint8_t attack, uint8_t decay, uint8_t sustain, uint8_t release)
{
	g_sidBase[7 * voice + 5] = (attack << 4) | decay;
	g_sidBase[7 * voice + 6] = (sustain << 4) | release;
}
 
void startTriangle(uint8_t voice)
{
	g_sidBase[7 * voice + 4] = (1 << 4) | 1;
}
 
void stopTriangle(uint8_t voice)
{
	g_sidBase[7 * voice + 4] = 1 << 4;
}
 
void delay()
{
	long i;
	for (i = 0; i < 500; i++);
}
 
void startTone(uint8_t voice, uint8_t freqIndex)
{
	setFrequency(voice, freqIndex);
	setAdsr(voice, 2, 1, 15, 1);
	startTriangle(voice);
}
 
void stopTone(uint8_t voice)
{
	stopTriangle(voice);
}
 
void playOneTone(uint8_t freqIndex)
{
	startTone(0, freqIndex);
	delay();
	stopTone(0);
}
 
void playThreeTones(uint8_t freqIndex0, uint8_t freqIndex1, uint8_t freqIndex2)
{
	startTone(0, freqIndex0);
	startTone(1, freqIndex1);
	startTone(2, freqIndex2);
	delay();
	stopTone(0);
	stopTone(1);
	stopTone(2);
}
 
int play_melody(unsigned char __fastcall__ (*callback)(void))
{
	g_sidBase[23] = 7;
	g_sidBase[22] = 10;
	while (1) {
		uint8_t i;
		for (i = 0; i < 8; i++) {
			playOneTone(i);
			if(callback()) return 0;
		}
		delay();
		
		playThreeTones(0, 2, 5);
        if(callback()) return 0;
		playThreeTones(2, 5, 7);
        if(callback()) return 0;
		playThreeTones(1, 2, 5);
        if(callback()) return 0;
		playThreeTones(0, 2, 5);
        if(callback()) return 0;
		playThreeTones(0, 2, 4);
        if(callback()) return 0;
		playThreeTones(0, 2, 5);
        if(callback()) return 0;
		delay();
	}
	return 0;
}
