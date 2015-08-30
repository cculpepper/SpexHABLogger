#ifndef UART_H
#define UART_H
#include "queue.h"
#define BUFFLEN 80

typedef struct {
	volatile QRecStruct *txQ;
	volatile QRecStruct *rxQ;
} uartStruct;

int charsAvail(void);
void PCUARTInit(void);
void putString(char* str);
void putChar(char ch);
char getChar(void);
void putNum(int num);
int getString(char*, int); // returns num of things in the string the int arg is the length of the buffer.
#endif
