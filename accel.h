#ifndef ACCEL_H
#define ACCEL_H

#define ACCELADDR 0x3A
#define WRITEBIT 0x1
#define ACCELIDREG 0x00
#define ACCELXREG 0x32
#define ACCELYREG 0x34
#define ACCELZREG 0x36
#define ACCELBWRATEREG 0x2C
#define ACCELPWRCTLREG 0x2D
#define ACCELDATAFMTREG 0x31
#define ACCELFIFOCTLREG 0x38
#define WRITEBIT 0
#define READBIT 1

struct accelDataStruct {
	int16_t x;
	int16_t y;
	int16_t z;
};
struct accelDataStruct accelData;

int accel_init(void);
int accel_read(void);

#endif

