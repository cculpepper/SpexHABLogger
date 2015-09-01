#include <msp430.h>
#include <stdint.h>
#include "LED1.h"

// required
#define SCL BIT5
#define SDA BIT6
#define SCLOUT P3OUT
#define SCLDIR P3DIR
#define SDAOUT P3OUT
#define SDADIR P3DIR
#define SDAIN  P3IN

#define READ 0xA1
#define WRITE 0xA0
#define FAILURE -1
#define SUCCESS 0
// required
void sendByte(void);
void sendByteInput(char);
char receiveByte(void);
void sendAck(char);
int receiveAck(void);
void SWI2CStart(void);
void SWI2CStop(void);
int initSwI2C(void);

// required
unsigned char txData = 0;
unsigned char rxData = 0;
unsigned char ackFlag = 0;
unsigned char bitCounter = 0;
unsigned int address = 0; // 12 bit address, upper 4 bits should be 0s.

char bmp180CheckComs(){
	SWI2CStart();
	txData = 0xEE;
	sendByte();
	receiveAck();
	if(!ackFlag)
		return 3;
	//txData = 0xBF;
	sendByteInput(0xD0);
	//_delay_cycles(100); // delay for testing
	//SWI2CStart();
	//sendByteInput(0xD0);
	receiveAck();
	if(!ackFlag)
		return 2;
	SWI2CStop();
	SWI2CStart();
	sendByteInput(0xEF);
	receiveAck();
	if(!ackFlag)
		return FAILURE;
	receiveByte();
	if (rxData == 0x55){
		sendAck(0);
		return 1;
	}
	return 0;
}



char bmp180ReadBytes(char addr, char reg, int len, char* dest){
	int index;
	index = 0;
	SWI2CStart();

	sendByteInput(addr);
	receiveAck();
	if(!ackFlag)
		return 3;
	//txData = 0xBF;
	sendByteInput(reg);
	//_delay_cycles(100); // delay for testing
	//SWI2CStart();
	//sendByteInput(0xD0);
	receiveAck();
	if(!ackFlag)
		return 2;
	SWI2CStop();
	_NOP();
	_NOP();
	SWI2CStart();
	sendByteInput(addr+1);
	receiveAck();
	if(!ackFlag)
		return FAILURE;
	while (len--){
		receiveByte();
		dest[index++] = rxData;
		sendAck(0);
	}
	SWI2CStop();
	_NOP();
	_NOP();
	return 0;
}



void sendByteInput(char input) {

	/*LED2_ON();*/
	//SDADIR |= SDA;
	bitCounter = 0;
	while(bitCounter < 8) {
		SCLOUT &= ~SCL;
		SCLDIR |= SCL;
		_NOP();
		_NOP();
		if (input & BIT7) {
			SDADIR &= ~SDA ;
		} else {
			 SDAOUT &= ~SDA;
			 SDADIR |= SDA;
		}
		// Strobe the clock up and down.
		SCLDIR &= ~SCL;
		input <<= 1;
		bitCounter++;
		_NOP();
		SCLDIR |= SCL;
		SCLOUT &= ~SCL;
		_NOP();
		_NOP();
	}
	/*LED2_OFF();*/

}
// required
// receive byte from slave
char  receiveByte(void) {
	/*LED2_ON();*/
	char res;
	bitCounter = 0;
	SDADIR &= ~ SDA;
	while(bitCounter < 8) {
		SCLDIR &= ~ SCL;
		_NOP();
		//MSP430Delay(50);
		res <<= 1;
		bitCounter++;
		if(SDAIN & SDA) {
			res |= BIT0;
		}
		_NOP();
		_NOP();
		SCLOUT &= ~SCL;
		SCLDIR |= SCL;

		//MSP430Delay(50);
	}
	/*LED2_OFF();*/
	return res;
}

void receiveBytePtr(char* ptr) {
	bitCounter = 0;
	while(bitCounter < 8) {
		SCLOUT |= SCL;
		*ptr <<= 1;
		bitCounter++;
		if(SDAIN & SDA) {
			*ptr |= BIT0;
		}
		SCLOUT &= ~SCL;
	}
}
// required
// send master's ACK
void sendAck(char nack) {
	// set nack to one to nack it.
	if (nack){
		SDADIR &= ~SDA;
	} else {
		SDAOUT &= ~SDA;
		SDADIR |= SDA;
	}
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	// To pulse the clock.
	SCLDIR &= ~SCL;
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	SCLOUT &= ~SCL;
	SCLDIR |= SCL;
	SDADIR &= ~SDA;
	// Releasing the SDA
}
// required
// receive slave's ACK
int receiveAck(void) {
	SDADIR &= ~SDA;
	_NOP();
	_NOP();
	SCLOUT |= SCL;
	_NOP();
	_NOP();
	if (SDAIN & SDA) {
		(ackFlag = 0) ;
	} else {
		(ackFlag = 1);
	}
	_NOP();
	_NOP();
	SCLOUT &= ~SCL;
	_NOP();
	_NOP();
	return ackFlag;
}
// required
// start condition
void SWI2CStart(void) {
	/*LED2_ON();*/

	SDADIR &= ~SDA;
	SCLDIR &= ~SCL;
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	_NOP();
	SDADIR |= SDA;
	SDAOUT &= ~SDA;
	_NOP();
	_NOP();
	//SCLDIR |= SCL;
	//SCLDIR &= ~SCL;
	_NOP();
	_NOP();
	/*LED2_OFF();*/

}
// required
// stop condition
void SWI2CStop(void) {
	/*LED2_ON();*/
	SDAOUT &= ~SDA;
	SDADIR |= SDA;
	_NOP();
	_NOP();
	_NOP();
	SCLDIR &= ~SCL;
	_NOP();
	_NOP();
	_NOP();
	SDADIR &= ~SDA;
	/*LED2_OFF();*/

}
