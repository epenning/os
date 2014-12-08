#ifndef _SOUND_H_
#define _SOUND_H_

#include "stdint.h"

class SoundCard {
public:
	static const uint32_t JUMPER = 2;
	static const uint32_t BASE = 0x200 | (0x10 * JUMPER);
	
    static void init();
};

#endif