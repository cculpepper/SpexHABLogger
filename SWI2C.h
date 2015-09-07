#ifndef SWI2C
#define SWI2C
#include <msp430.h>
#include <stdint.h>
#define ACK 0
#define NACK 1
// required
// required
void sendByte(void);
void sendByteInput(char);
char receiveByte(void);
void sendAck(char);
int receiveAck(void);
void SWI2CStart(void);
void SWI2CStop(void);
#endif
