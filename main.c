
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
#include "./FatFS/ff.h"
FIL logfile;

void initAll(){
	LED2INIT();
	LED1INIT();
	BMP180GetCalVals();
	BMP180GetCalVals();
	PCUARTInit();
	SDOpenFile(&logfile);
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
	WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog
	PM5CTL0 &= ~LOCKLPM5;
	//PCUartInit();
	initAll();
	testParse();
	initGps();
	// Enable USCI_A0 RX interrupt

	//__bis_SR_register(LPM3_bits | GIE);       // Enter LPM3, interrupts enabled

	__no_operation(); // For debugger
	//I2CInit()
	//cwSend("AB1TJ", 5);
	f_printf(&logfile, "GPS Time, GPSDate, GPS Latitude, GPS Longitude, BMP180 Temperature, BMP180 Pressure\r\n");
	f_sync(&logfile);
	while(1){
		//gpsTerm();
		f_printf(&logfile, "%s, %s, %s%c, %s%c, %d, %d\r\n", gpsData.time, gpsData.date, gpsData.lat, gpsData.latHemi, gpsData.lon, gpsData.lonHemi, BMP180GetTemp(), BMP180GetPressure(7500));
		f_sync(&logfile);
		MSP430Delay(10000000);
		LED1_TOGGLE();


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
