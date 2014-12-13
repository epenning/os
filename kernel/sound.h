#ifndef _SOUND_H_
#define _SOUND_H_

#include "stdint.h"

class SoundCard {
private:
	static const uint32_t TIMEOUT = 32000;

	static const uint32_t JUMPER = 2;
	static const uint32_t BASE = 0x200 | (0x10 * JUMPER);

	//port offsets
	static const uint32_t RESET = 0x6;
	static const uint32_t READ_DATA = 0xA;
	static const uint32_t WRITE_COMMAND = 0xC;
	static const uint32_t WRITE_DATA = 0xC;
	static const uint32_t READ_STATUS = 0xE;
	static const uint32_t WRITE_STATUS = 0xC;

	//commands
	static const uint32_t SPEAKER_ON = 0xD1;
	static const uint32_t SPEAKER_OFF = 0xD3;
	static const uint32_t AUTO_DMA8 = 0x1C;
public:
	static int sb16_inb(int offset);
	static void sb16_outb(int offset, int value);
	static void dma_setup(uint32_t address, int count, int mode);
	static void set_speed();
    static void init();
    static void reset();
    static int write(int value);
    static void play(unsigned char byte);
    static void sleep(uint32_t jiffies);
};

#endif
