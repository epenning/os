#include "sound.h"
#include "stdint.h"
#include "machine.h"
#include "debug.h"
#include "process.h"
#include "dma.h"
#include "heap.h"

int SoundCard::sb16_inb(int offset){
	return inb(BASE + offset);
}

void SoundCard::sb16_outb(int offset, int value){
	outb(BASE + offset, value);
}

void SoundCard::dma_setup(uint32_t address, int count, int mode) {
	Debug::printf("Setting up \"DMA\"\n");

	count--;
	outb(0xA, 1 | 0x04);			// disable DMA channel
	outb(0xC, 0x00);				// clear flip flop
	outb(0xB, 0x58 + 1);				// set DMA mode
	outb(0x1 << 1, address >> 0);	// low byte of address
	outb(0x1 << 1, address >> 8);	// high byte of address
	outb(0x83, address >> 16);		// 64k page number
	outb((0x1 << 1) + 1, count >> 0);	// low byte of count
	outb((0x1 << 1) + 1, count >> 8);	// high byte of count
	outb(0xA, 1);					// disable DMA channel
}

void SoundCard::set_speed() {
	write(0x41);		// output rate
	write(22050 >> 8);	// high byte of speed
	write(22050);		// low byte of speed
}

void SoundCard::init(){
	reset();
	/*
	if (!Reserve_DMA(1))
		Debug::printf("reserve DMA failed - should never happen!\n");
	void* address = (void*)PhysMem::alloc();
	Setup_DMA(DMA_WRITE, 1, address, 0x1000);
	//dma_setup((uint32_t)address, 0, 0x00);
	//outb(0xB, 0x58 + 1);				// set DMA mode
	set_speed();
	*/
	write(0xE1);		// get version information
	int maj = sb16_inb(READ_DATA);
	int min = sb16_inb(READ_DATA);
	Process::trace("initialized sb16 v%d.%d", maj, min);
	/* set SB to have IRQ 5 and DMA 1 */
	/*
	sb16_outb(0x4, 0x80);	// IRQ register
	sb16_outb(0x5, (1 << (5/2 - 1)));
	sb16_outb(0x4, 0x81);	// DMA register
	sb16_outb(0x5, (1 << 1 | 1 << 5));
	write(SPEAKER_ON);
	write(0xC6);		// "program dsp with dma mode 8 bit auto out"
	//write(0xB6);		// "program dsp with dma mode 16 bit auto out"
	write (0x00);		// "program dsp with transfer mode mono
	write ((0x1000-1) >> 0);	// give length of single fragment to dsp
	write ((0x1000-1) >> 8);
	*/
	//write(0x1C);
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
	for (uint32_t i = 0; i < 1000; i++) {
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

void SoundCard::sound_interrupt_handler() {
	Debug::printf("sound interrupt handler went\n");
}
