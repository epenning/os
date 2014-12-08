#include "sound.h"
#include "stdint.h"
#include "machine.h"
#include "process.h"
#include "debug.h"

void SoundCard::init(){
	Debug::printf("SB16 BASE: 0x%x\n", BASE);
	outb(BASE + 6, 1);
	Process::sleepFor(1);
	outb(BASE + 6, 0);
	Process::sleepFor(1);
	int x = inb(BASE + 0xA);
	Debug::printf("Read Data port reads: 0x%x\n", x);
}