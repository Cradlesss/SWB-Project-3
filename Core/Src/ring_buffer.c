#include "ring_buffer.h"

void RB_Init(RingBuffer *rb){
    rb->head = 0;
    rb->tail = 0;
}

int RB_Write(RingBuffer *rb, unsigned char c){
    int next = (rb->head +1) % 256;

    if(next == rb->tail){
        return -1; 
    }

    rb->data[rb->head] = c;
    rb->head = next;
    return 0;
}

int RB_Read(RingBuffer *rb, unsigned char *c){
	if (rb->head == rb->tail){
		return -1;
	}

	*c = rb->data[rb->tail];
	rb->tail = (rb->tail + 1) % 256;
	return 0;
}
