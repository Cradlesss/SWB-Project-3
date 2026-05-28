/*
 * ring_buffer.h
 *
 *  Created on: 19 mar 2026
 *      Author: macmac
 */

#ifndef RING_BUFFER_H
#define RING_BUFFER_H

typedef struct{
	unsigned char data[256];
	int head;
	int tail;
} RingBuffer;

void RB_Init(RingBuffer *rb);
int RB_Write(RingBuffer *rb, unsigned char c);
int RB_Read(RingBuffer *rb, unsigned char *c);

#endif
