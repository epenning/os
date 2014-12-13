/*************************************************************************/
/*
 * GeekOS master source distribution and/or project solution
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * This file is not distributed under the standard GeekOS license.
 * Publication or redistribution of this file without permission of
 * the author(s) is prohibited.
 */
/*************************************************************************/
/*
 * 8237A DMA Controller Support
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.14 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

/*
 * Information sources:
 * - Intel 8237A datasheet:
 *    http://www.nondot.org/sabre/os/files/MiscHW/8237A_DMAControllerDatasheet.pdf
 * - Linux include/asm-i386/dma.h
 * - http://docs.freebsd.org/doc/3.4-RELEASE/usr/share/doc/handbook/dma.html
 */

/*
 * NOTES:
 * - Currently, we only support DMA channels 0-3.
 *
 * - Address lines A0..A3 of the master 8237A are wired to the
 *   low bits of the port I/O address bus in the obvious way.
 *   For some reason, A0..A3 of the slave 8237A (channels 4-7)
 *   are wired to bits 1..4 of the I/O address bus, so for that
 *   controller the port addresses must be shifted left one bit.
 *   Here is how register ports might be parameterized by
 *   controller and register:
 *
 *	#define COMMAND_REG		0x8
 *	#define DMA_REG(ctrlr,reg)	(((ctrlr)==0)?(reg):(0xD0+((reg)<<1)))
 *	#define DMA_COMMAND_REG(ctrlr)	DMA_REG(ctrlr,COMMAND_REG)
 */

#include "dma.h"

/* ----------------------------------------------------------------------
 * Definitions
 * ---------------------------------------------------------------------- */

/*
 * DMA controller ports and definitions.
 * See Figures 5 and 6 of 8237A datasheet.
 */

/*
 * Only support channels 0..3 for now.
 */
#define VALID_CHANNEL(chan)	(((chan) >= 0) && ((chan) < 4))

/*
 * The DMA controller can only address the first 2^24
 * bytes of memory.
 */
#define DMA_MAX_ADDR		0x1000000UL
#define VALID_MEM(start,size)	((uint32_t)(start)+(size))<DMA_MAX_ADDR

/*
 * Master controller:
 */
#define DMA_BASE		0x00	 /*!< Base I/O address of controller */
#define DMA_COMMAND_REG		0x08	 /*!< Command register (output) */
#define DMA_STATUS_REG		0x08	 /*!< Status register (input) */
#define DMA_REQUEST_REG		0x09	 /*!< Request register (output) */
#define DMA_MASK_ONE_REG	0x0A	 /*!< Set/reset one bit of mask (output) */
#define DMA_MODE_REG		0x0B	 /*!< Mode register (output) */
#define DMA_CLEAR_FF_REG	0x0C	 /*!< Clear byte pointer flip-flop (output) */
#define DMA_MASTER_CLEAR_REG	0x0D	 /*!< Reset the controller (output) */
#define DMA_TEMP_REG		0x0D	 /*!< Storage for mem/mem xfers (input) */
#define DMA_CLEAR_MASK_REG	0x0E	 /*!< Clear all mask bits (output) */
#define DMA_MASK_ALL_REG	0x0F	 /*!< Set all bits of mask (output) */

/*
 * Access to base/current address andbase/current word count.
 * The port address depends on the channel.
 */
#define DMA_ADDR_REG(chan)	(((chan) & 0x3)<<1)	 /*!< Base and current address (input,output) */
#define DMA_COUNT_REG(chan)	((((chan) & 3)<<1)|1)  /*!< Base and current work count (input,output) */

/*
 * Mask register bits:
 */
#define DMA_MASK_ENABLE		(1<<2)	 /*!< Mask register bit to enable a channel */

/*
 * Mode register bits:
 */
#define DMA_MODE_SINGLE		0x40	 /*!< Set up for single transfer */
#define DMA_MODE_CASCADE	0xC0	 /*!< Set up for cascade transfer (from slave controller) */
#define DMA_MODE_READ		0x04	 /*!< Read transfer */
#define DMA_MODE_WRITE		0x08	 /*!< Write transfer */

/*
 * DMA page registers
 */
static const char s_dmaPageRegisterList[] = {
    (char)0x87,	 /* DMA Channel 0 */
	(char)0x83,	 /* DMA Channel 1 */
	(char)0x81,	 /* DMA Channel 2 */
	(char)0x82,	 /* DMA Channel 3 */
};
#define DMA_PAGE_REG(chan)	(s_dmaPageRegisterList[chan])  /*!< DMA page register for a channel */

#define IS_RESERVED(chan)	((s_allocated & (1 << (chan))) != 0)

/*#define DEBUG_DMA */
#ifdef DEBUG_DMA
#  define Debug(args...) Print(args)
#else
#  define Debug(args...)
#endif

/* ----------------------------------------------------------------------
 * Variables
 * ---------------------------------------------------------------------- */

static char s_allocated;	 /*!< Which channels have been allocated. */

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

#define KASSERT(cond) 					\
do {							\
    if (!(cond)) {					\
	Debug::printf("Failed assertion in %s: %s at %s, line %d, RA=%lx\n",\
		__func__, #cond, __FILE__, __LINE__,	\
		(unsigned long) __builtin_return_address(0));			\
	while (1)					\
	   ; 						\
    }							\
} while (0)

/**
 * Initialize the DMA controllers.
 */
void Init_DMA(void)
{
    Debug("Initializing DMA Controller...\n");

    /* Reset the controller */
    outb(DMA_MASTER_CLEAR_REG, 0);
}

/**
 * Reserve given DMA channel.
 * @param chan the channel to reserve
 * @return true if successful, false if not
 */
bool Reserve_DMA(int chan)
{
    Process::disable();
    bool result = false;

    //KASSERT(VALID_CHANNEL(chan));

    if (!IS_RESERVED(chan)) {
	/* Channel is available; unmask it. */
	outb(DMA_MASK_ONE_REG, chan & 3);

	/* Mask channel as allocated */
	s_allocated |= (1 << chan);
	result = true;
    }

    Process::enable();

    return result;
}

/**
 * Set up a DMA transfer.
 * @param direction the direction of the transfer (DMA_READ or DMA_WRITE)
 * @param chan the channel
 * @param addr the address of the buffer
 * @param size number of bytes to transfer
 */
void Setup_DMA(enum DMA_Direction direction, int chan, void *addr_, unsigned long size)
{
    char mode = 0;
    long addr = (unsigned long) addr_;

    /* Make sure parameters are sensible */
    KASSERT(direction == DMA_READ || direction == DMA_WRITE);
    KASSERT(VALID_CHANNEL(chan));
    KASSERT(IS_RESERVED(chan));
    KASSERT(VALID_MEM(addr, size));
    KASSERT(size > 0);
    KASSERT(size <= (0xffff - (addr & 0xffff)));  /* can't cross 64K boundary */

    /* Set up transfer mode */
    mode |= DMA_MODE_SINGLE;
    mode |= (direction == DMA_READ) ? DMA_MODE_READ : DMA_MODE_WRITE;
    mode |= (chan & 3);

    Debug::printf("Setup_DMA(%s,%d,%x,%d)\n", direction == DMA_READ ? "DMA_READ" : "DMA_WRITE",
	chan, addr, size);
    Debug("Setup_DMA: mode=%02x\n", mode);
    Debug("DMA_ADDR_REG for channel is %02x\n", DMA_ADDR_REG(chan));
    Debug("DMA_PAGE_REG for channel is %02x\n", DMA_PAGE_REG(chan));
    Debug("DMA_COUNT_REG for channel is %02x\n", DMA_COUNT_REG(chan));

    /* Temporarily mask the DMA channel */
    Mask_DMA(chan);

    /* Write the transfer mode */
    outb(DMA_MODE_REG, mode);

    /* Clear the byte pointer flip-flop */
    outb(DMA_CLEAR_FF_REG, 0);  /* doesn't matter what value is written here */

    /* Write the transfer address (LSB, then MSB) */
    outb(DMA_ADDR_REG(chan), addr & 0xFF);
    outb(DMA_ADDR_REG(chan), (addr >> 8) & 0xFF);

    /* Write the page register */
    outb(DMA_PAGE_REG(chan), (addr >> 16) & 0xFF);

    /*
     * Write the count (LSB, then MSB)
     * Note that the count is one less that the number of bytes transferred
     */
    --size;
    outb(DMA_COUNT_REG(chan), size & 0xFF);
    outb(DMA_COUNT_REG(chan), (size >> 8) & 0xFF);

    /* Now we can unmask the channel again */
    Unmask_DMA(chan);
}

/**
 * Mask given DMA channel.
 * The channel must have already been reserved.
 * @param chan the channel
 */
void Mask_DMA(int chan)
{
    KASSERT(VALID_CHANNEL(chan));
    KASSERT(IS_RESERVED(chan));

    outb(DMA_MASK_ONE_REG, (1 << 2) | (chan & 3));
}

/**
 * Unmask given DMA channel.
 * The channel must have already been reserved.
 * @param chan the channel
 */
void Unmask_DMA(int chan)
{
    KASSERT(VALID_CHANNEL(chan));
    KASSERT(IS_RESERVED(chan));

    outb(DMA_MASK_ONE_REG, chan & 3);
}
