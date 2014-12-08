#include "sound.h"
#include "stdint.h"
#include "machine.h"
#include "debug.h"
#include "process.h"

int SoundCard::sb16_inb(int offset){
	return inb(BASE + offset);
}

void SoundCard::sb16_outb(int offset, int value){
	outb(BASE + offset, value);
}


void SoundCard::init(){
	reset();
	write(0xE1);
	int maj = sb16_inb(READ_DATA);
	int min = sb16_inb(READ_DATA);
	Process::trace("initialized sb16 v%d.%d", maj, min);
	write(SPEAKER_ON);
}

void SoundCard::reset(){
	sb16_outb(RESET, 1);
	uint32_t garbage;
	for(garbage = 0; garbage < TIMEOUT; garbage++);
	sb16_outb(RESET, 0);
	for(garbage = 0; garbage < TIMEOUT; garbage++);
	int x = sb16_inb(READ_DATA);
	if(x != 0xAA)
		Debug::printf("sb16: ERROR returned: 0x%x after reset, expected 0xAA\n", x);
}

int SoundCard::write(int value){
	Debug::printf("sb16: writing 0x%x\n", value);
	for (uint32_t i = 0; i < TIMEOUT; i++) {
		if((sb16_inb(WRITE_STATUS) & 0x80) == 0) {
			sb16_outb(WRITE_COMMAND, value);
			return 0;
		}
	}

	Debug::printf("sb16: SoundBlaster: DSP Command(%x) timeout\n", value);
	return -1;
}

void SoundCard::play(unsigned char byte){
	write(0x10);
	write(byte);
}