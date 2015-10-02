#include "msp430.h"
#include "cw.h"
#include "LED1.h"
#include "TII2C.h"
#include "sd.h"
#include "bmpLib.h"
#include "uart.h"
#include "gps.h"
#include "accel.h"
#include "FatFS/ff.h"
FIL logfile;
void initRTC(){
	RTCCTL0 = 0;
	RTCCTL1 = 0;
	RTCCTL2 = 0;
	RTCSEC = 0;
	RTCHOUR = 0;
	RTCMIN = 0;

}
int initClock(){
	if (1){
		CSCTL0_H = CSKEY >> 8;
		CSCTL1 = 0x000C;
		CSCTL2 = (CSCTL2 & 0x00F8) | 0x0003;
		CSCTL0_H = 0;
	} else {
		return 1;
	}
}
void initS1(){
//	P1SEL  &=  (~BIT1);    //  Set P1.0    SEL for GPIO
	P1DIR  &=  ~BIT1;   //  Set P1.0    as  Output
	P1OUT  |=  BIT1;   //  Set P1.0    HIGH
	P1REN |= BIT1;
}
void initAll(){
	initS1();
	LED2INIT();
	LED1INIT();
	if (initClock()){
		/* Blink UART
		 * Wait for press.
		 */
		cwSend("CLK", 3);
		while (P1IN & BIT1);
	} else {

		cwSend("CLKGO", 5);
		while (P1IN & BIT1);
	}

	while (P1IN & BIT1);
	if (PCUARTInit()){
		/* Blink UART
		 * Wait for press.
		 */
		cwSend("UART", 4);
		while (P1IN & BIT1);
	} else {

		cwSend("UARTGO", 6);
		while (P1IN & BIT1);
	}
	initRTC();
	set_port(0);
	if (accel_init()){
		/* Blink UART
		 * Wait for press.
		 */
		cwSend("ACCL", 4);
		while (P1IN & BIT1);
	} else {

		cwSend("ACCLGO", 6);
		while (P1IN & BIT1);
	}
	if (BMP180GetCalVals(1)){
		/* Blink UART
		 * Wait for press.
		 */
		cwSend("BMP1", 4);
		while (P1IN & BIT1);
	} else {

		cwSend("BMP1GO", 6);
		while (P1IN & BIT1);
	}
	if (BMP180GetCalVals(0)){
		/* Blink UART
		 * Wait for press.
		 */
		cwSend("BMP0GO", 4);
		while (P1IN & BIT1);
	} else {

		cwSend("BMP1GO", 6);
		while (P1IN & BIT1);
	}
	if (SDOpenFile(&logfile,"LOG1_00.csv")){
		/* Blink UART
		 * Wait for press.
		 */
		cwSend("SD", 2);
		while (P1IN & BIT1);
	} else {

		cwSend("SDGO", 4);
		while (P1IN & BIT1);
	}
}
char MSP430Delay3(int cycles){
	TA1CCTL0 = 0;
	TA1R = 0;
	TA1CCR0 = cycles;
	TA1CTL |= TASSEL_1 | MC_1 | ID_3;	//Use ACLK as source for timer
	TA1EX0 = TAIDEX_3;
	TA1CTL |= MC_1;	//Use UP mode timer
	TA1CCTL0 |= CCIE; // CCR0 interrupt enabled
	__bis_SR_register(LPM0_bits + GIE);                // Enter LPM0 w/ Interrupt
	TA1CCTL0 &= ~CCIE;
	return 0;
}
enum FLIGHTSTATE{
	TEST,
	ASCENT,
	CUTDOWN,
	DESCENT
};
enum FLIGHTSTATE flightState;
/*The remove befor flight jumper is on 1.2.
 * Need to enable input, pull up. When is not 0, we are go for launch.
 */
int main(void)
{
	int sec,hour,min,lines;
	flightState = TEST;
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
	//f_printf(&logfile, "0:AC1:,%d,AC2:,%d,AC3:,%d,AC4:,%u,AC5,%u,AC6,%u,B1,%d,B2:,%d,MB:,%d,MC:,%d,MD:,%d\r\n", calInst->ac1,calInst->ac2,calInst->ac3,calInst->ac4,calInst->ac5,calInst->ac6,calInst->b1,calInst->b2,calInst->mb,calInst->mc,calInst->md);
	set_bmp180(1);
	//f_printf(&logfile, "1:AC1:,%d,AC2:,%d,AC3:,%d,AC4:,%u,AC5,%u,AC6,%u,B1,%d,B2:,%d,MB:,%d,MC:,%d,MD:,%d\r\n", calInst->ac1,calInst->ac2,calInst->ac3,calInst->ac4,calInst->ac5,calInst->ac6,calInst->b1,calInst->b2,calInst->mb,calInst->mc,calInst->md);
	f_printf(&logfile, "RTCTime,GPStime,Date,Lat,Lon,Alt,180Temp0,180Press0,180Temp1,180Press1,X,Y,Z,Press2,Temp2,x1,X2,B5,TEMP,B6,X1,X2,X3,B3,X1,X2,X3,b4,B7,P,X1,X1,x2,p\r\n");
	//f_printf(&logfile, "X1:,=(%d-%u)*%u/2**15,X2:,=%d*2**11/($B$2+%d),B5:,=$B$2+$D$2\r\n")
	//f_printf(&logfile, "B6:,=$B$6-4000,X1:,(%d*($B$2*$B$2/2**12))/2**11,X2:,=%d*$B$2/2**11,X3:$i")
	f_printf(&logfile, ",%s, %s, %s%c, %s%c,, %d, %u,,,,,,,,=(G3-$L$1)*$J$1/2^15,=$T$1*2^11/(P3+$V$1),=P3+Q3,=(R3+8)/2^4,=R3-4000,=($P$1*(T3*T3/2^12))/2^11,=$D$1*T3/2^11,=U3+V3,=((($B$1*4+W3)*2)+2)/4,=$F$1*T3/2^13,=($N$1*(T3*T3/2^12))/2^16,=(Y3+Z3+2)/4,=$H$1*(AA3+32768)/2^15,=(H3-X3)*(50000/2),\"=IF(AC3<HEX2DEC(80000000),AC3*2/AB3,AC3/AB3*2)\",=(AD3/2^8)*(AD3/2^8),=AE3*3038/2^16,=(-7357*AD3)/2^16,=AD3+(AF3+AG3+3971)/2^4\r\n", gpsData.time, gpsData.date, gpsData.lat, gpsData.latHemi, gpsData.lon, gpsData.lonHemi, BMP180GetRawTemp(), BMP180GetRawPressure(7500));
	f_sync(&logfile);
	lines += 4;
	while(1){
		//gpsTerm();
		uartEnableRx();
		//MSP430Delay(2000);// Get a fix.
		set_bmp180(0);
		accel_read();
		f_printf(&logfile, "%02u:%02u:%02u,%s,%s,%s%c,%s%c,,%d,%u,",hour,min,sec,gpsData.time, gpsData.date, gpsData.lat, gpsData.latHemi, gpsData.lon, gpsData.lonHemi, BMP180GetTemp(), BMP180GetPressure(7500));
		set_bmp180(1);
//		f_printf(&logfile, "%d,%u,%d,%d,%d\r\n",BMP180GetRawTemp(), 0,accelData.x,accelData.y,accelData.z);
		f_printf(&logfile, "%d,%u,%d,%d,%d\r\n",BMP180GetTemp(), BMP180GetPressure(7500),accelData.x,accelData.y,accelData.z);
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
