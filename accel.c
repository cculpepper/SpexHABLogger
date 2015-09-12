#include "SWI2C.h"
#include <msp430.h>
#include <stdint.h>
#include "accel.h"

int accel_write_reg(char addr, char data[], int numBytes){
	int i;

	i = 0;
	SWI2CStart();
	sendByteInput(ACCELADDR | WRITEBIT);
	receiveAck();
	sendByteInput(addr);
	receiveAck();
	while (numBytes-- > 0){
		sendByteInput(data[i++]);
		receiveAck();
	}
	SWI2CStop();
	return 0;
}
int accel_read_reg(char addr, char data[], int numBytes){
	int i;
	i = 0;
	SWI2CStart();
	sendByteInput(ACCELADDR | WRITEBIT);
	receiveAck();
	sendByteInput(addr);
	receiveAck();
	SWI2CStop();

	SWI2CStart();
	sendByteInput(ACCELADDR | READBIT);
	receiveAck();
	while (numBytes-- > 0){
		data[i++] = receiveByte();
		if (numBytes > 0){
			sendAck(ACK);
		} else {
			sendAck(NACK);
		}
	}
	SWI2CStop();
}

int accel_init(void){
	char id, data;
	accel_read_reg(ACCELIDREG, &id, 1);
	if (id != 0xE5){
		return -1;
	}
	// Need to disable the FIFO, bypassmode.
	// Need to set the rate code to 0x7 because well be pulling data at
	// 	less than 12.5Hz
	// TODO: Make datarate faster for way down
		
	data = 0x7; // Enable 12.5 Hz update rate
	accel_write_reg(ACCELBWRATEREG, &data, 1);
	data = 0x8; // Going to enable measurement. 
	accel_write_reg(ACCELPWRCTLREG, &data, 1);
	// If we want to change the g range from the default of 2g, 
	// 	use the data_fmt reg. 
	// FIFO is disabled on boot. Good. 
	return 0;
}
int accel_read(void){
	// This will initiate a read of the data. 
	accel_read_reg(ACCELXREG, &accelData, 6);
}




