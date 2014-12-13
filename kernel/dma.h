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
 * $Revision: 1.10 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef GEEKOS_DMA_H
#define GEEKOS_DMA_H

#include "debug.h"
#include "process.h"
#include "machine.h"
#include "idt.h"

enum DMA_Direction {
    DMA_READ,
    DMA_WRITE
};

void Init_DMA(void);
bool Reserve_DMA(int chan);
void Setup_DMA(enum DMA_Direction direction, int chan, void *addr, unsigned long size);

void Mask_DMA(int chan);
void Unmask_DMA(int chan);

#endif  /* GEEKOS_DMA_H */
