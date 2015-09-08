
/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 *
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430FR59xx Demo - eUSCI_A0 UART echo at 9600 baud using BRCLK = 32768Hz
//
//  Description: This demo echoes back characters received via a PC serial port.
//  ACLK is used as a clock source and the device is put in LPM3
//  Note that level shifter hardware is needed to shift between RS232 and MSP
//  voltage levels.
//
//  The example code shows proper initialization of registers
//  and interrupts to receive and transmit data.
//  To test code in LPM3, disconnect the debugger and use an external supply
//  Otherwise use LPM0
//
//  ACLK = 32768Hz, MCLK =  SMCLK = default DCO
//
//                MSP430FR5969
//             -----------------
//            |                 |--LFXTIN (32768Hz reqd.)
//            |                 |--LFXTOUT
//            |                 |
//       RST -|     P2.0/UCA0TXD|----> PC (echo)
//            |                 |
//            |                 |
//            |     P2.1/UCA0RXD|<---- PC
//            |                 |
//
//   P. Thanigai
//   Texas Instruments Inc.
//   August 2012
//   Built with IAR Embedded Workbench V5.40 & Code Composer Studio V5.5
//******************************************************************************


#include "msp430.h"
#include "cw.h"
#include "LED1.h"
#include "TII2C.h"
#include "sd.h"
#include "bmpLib.h"
#include "uart.h"
#include "gps.h"
#include "accel.h"
#include "./FatFS/ff.h"
FIL logfile;
void initRTC(){
	RTCCTL0 = 0;
	RTCCTL1 = 0;
	RTCCTL2 = 0;
	RTCSEC = 0;
	RTCHOUR = 0;
	RTCMIN = 0;

}
void initAll(){
	LED2INIT();
	LED1INIT();
	PCUARTInit();
	initRTC();
	set_port(0);
	accel_init();
	BMP180GetCalVals(1);
	BMP180GetCalVals(0);
	SDOpenFile(&logfile,"LOG1_00.csv");
}
char MSP430Delay3(int cycles){
	TA1CCTL0 = 0;
	TA1R = 0;
	TA1CCR0 = cycles;
	TA1CTL |= TASSEL_1 | MC_1;	//Use ACLK as source for timer
	TA1CTL |= MC_1;	//Use UP mode timer
	TA1CCTL0 |= CCIE; // CCR0 interrupt enabled
	__bis_SR_register(LPM0_bits + GIE);                // Enter LPM0 w/ Interrupt
	TA1CCTL0 &= ~CCIE;
	return 0;
}

int main(void)
{
	int sec,hour,min,lines;
	lines = 0;
	WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog
	PM5CTL0 &= ~LOCKLPM5;
	//PCUartInit();
	testParse(); // Get some data into the GPS struct. 
	initAll();


	__no_operation(); // For debugger
	//I2CInit()
	//cwSend("AB1TJ", 5);
	// Temp: X1 = (UT - AC6) * AC5 / 2^15
	// X2 = MC * 2^11/(X1+MD)
	// B5 = X1 + X2
	// T = (B5 + 8)/2^4
	// T =
	set_bmp180(0);
	f_printf(&logfile, "0:AC1:,%d,AC2:,%d,AC3:,%d,AC4:,%u,AC5,%u,AC6,%u,B1,%d,B2:,%d,MB:,%d,MC:,%d,MD:,%d\r\n", calInst->ac1,calInst->ac2,calInst->ac3,calInst->ac4,calInst->ac5,calInst->ac6,calInst->b1,calInst->b2,calInst->mb,calInst->mc,calInst->md);
	set_bmp180(1);
	f_printf(&logfile, "1:AC1:,%d,AC2:,%d,AC3:,%d,AC4:,%u,AC5,%u,AC6,%u,B1,%d,B2:,%d,MB:,%d,MC:,%d,MD:,%d\r\n", calInst->ac1,calInst->ac2,calInst->ac3,calInst->ac4,calInst->ac5,calInst->ac6,calInst->b1,calInst->b2,calInst->mb,calInst->mc,calInst->md);
	f_printf(&logfile, "RTCTime,GPStime,Date,Lat,Lon,Alt,180Temp0,180Press0,180Temp1,180Press1,X,Y,Z,Press2,Temp2,x1,X2,B5,TEMP,B6,X1,X2,X3,B3,X1,X2,X3,b4,B7,P,X1,X1,x2,p\r\n");
	//f_printf(&logfile, "X1:,=(%d-%u)*%u/2**15,X2:,=%d*2**11/($B$2+%d),B5:,=$B$2+$D$2\r\n")
	//f_printf(&logfile, "B6:,=$B$6-4000,X1:,(%d*($B$2*$B$2/2**12))/2**11,X2:,=%d*$B$2/2**11,X3:$i")
	f_printf(&logfile, ",%s, %s, %s%c, %s%c,, %d, %u,,,,,,,,=(G3-$L$1)*$J$1/2^15,=$T$1*2^11/(P3+$V$1),=P3+Q3,=(R3+8)/2^4,=R3-4000,=($P$1*(T3*T3/2^12))/2^11,=$D$1*T3/2^11,=U3+V3,=((($B$1*4+W3)*2)+2)/4,=$F$1*T3/2^13,=($N$1*(T3*T3/2^12))/2^16,=(Y3+Z3+2)/4,=$H$1*(AA3+32768)/2^15,=(H3-X3)*(50000/2),\"=IF(AC3<HEX2DEC(80000000),AC3*2/AB3,AC3/AB3*2)\",=(AD3/2^8)*(AD3/2^8),=AE3*3038/2^16,=(-7357*AD3)/2^16,=AD3+(AF3+AG3+3971)/2^4\r\n", gpsData.time, gpsData.date, gpsData.lat, gpsData.latHemi, gpsData.lon, gpsData.lonHemi, BMP180GetRawTemp(), BMP180GetRawPressure(7500));
	f_sync(&logfile);
	lines += 4;
	while(0){
		//gpsTerm();
		uartEnableRx();
		//MSP430Delay(2000);// Get a fix.
		LED1_ON();
		set_bmp180(0);
		accel_read();
		f_printf(&logfile, ",%s,%s,%s%c,%s%c,,%d,%u,",gpsData.time, gpsData.date, gpsData.lat, gpsData.latHemi, gpsData.lon, gpsData.lonHemi, BMP180GetRawTemp(), BMP180GetRawPressure(7500));
		set_bmp180(1);
		f_printf(&logfile, "%d,%u,%d,%d,%d\r\n",BMP180GetRawTemp(), BMP180GetRawPressure(7500),accelData.x,accelData.y,accelData.z);
		f_sync(&logfile);
		lines += 1;
		LED2_TOGGLE();
		LED1_TOGGLE();
		//uartDisableRx();
		//MSP430Delay(10000000);
		if (lines > 20000){
			f_close(&logfile);
			SDOpenFile(&logfile, "CONTU00.csv");
			lines = 0;
		}

	}
	// This is the going down code.
	f_close(&logfile);
	//SDCloseFile(&logfile);
	SDOpenFile(&logfile, "DWN1_00.csv");
	f_printf(&logfile, "RTCTime,Alt,internalTemp,X,Y,Z\r\n");
	f_sync(&logfile);
	set_port(0);
	while(1){
		accel_read();
		while ((RTCCTL1 & BIT4) == 0);
		hour =RTCHOUR;
		min = RTCMIN;
		sec = RTCSEC;
		f_printf(&logfile,"%02u:%02u:%02u,%s,,%d,%d,%d\r\n",hour,min,sec,gpsData.lat,accelData.x,accelData.y,accelData.z);
		f_sync(&logfile);
		lines += 1;
		LED1_TOGGLE();
		if (lines > 20000){
			f_close(&logfile);
			SDOpenFile(&logfile, "CONTD00.csv");
			lines = 0;
		}
	}
}

// Timer A0 interrupt service routine

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer_A_Delay (void){

	LPM0_EXIT;
	TA1CCR0 = 0;//Stop the counter.


}
#pragma vector= TRAPINT_VECTOR
__interrupt void TRAPINT_ISR(void)

{
	__no_operation();
}
