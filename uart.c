#include "msp430.h"
#include "queue.h"
#include "uart.h"
#include "gps.h"
#define BUFFLEN 80
#define MAXGETRETRIES 100000
char UARTSending; // Used to not transmit until the string is there.
#define __dint() _BIC_SR(GIE)
#define __eint() _BIS_SR(GIE)

volatile uartStruct uartData;
int charsAvail(void){
	return uartData.rxQ->stored;
}
void PCUARTInit(void){
	// Configure GPIO
	WDTCTL = WDTPW | WDTHOLD;                 // Stop Watchdog
#define UART1RX 	BIT5
#define UART1TX	BIT6
	P2SEL1 |= UART1RX | UART1TX;                    // USCI_A0 UART operation
	P2SEL0 &= ~(UART1RX | UART1TX);
	PJSEL0 |= BIT5 | BIT4;                    // For XT1

	// Disable the GPIO power-on default high-impedance mode to activate
	// previously configured port settings
	PM5CTL0 &= ~LOCKLPM5;

	// XT1 Setup
	CSCTL0_H = CSKEY >> 8;                    // Unlock CS registers
	CSCTL2 = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
	CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;     // Set all dividers
	CSCTL4 &= ~LFXTOFF;
	do
	{
		CSCTL5 &= ~LFXTOFFG;                    // Clear XT1 fault flag
		SFRIFG1 &= ~OFIFG;
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
	CSCTL0_H = 0;                             // Lock CS registers

	// Configure USCI_A0 for UART mode
	UCA1CTLW0 = UCSWRST;                      // Put eUSCI in reset
	UCA1CTLW0 |= UCSSEL__ACLK;                // CLK = ACLK
	UCA1BR0 = 3;                              // 9600 baud
	UCA1MCTLW |= 0x5300;                      // 32768/9600 - INT(32768/9600)=0.41
	// UCBRSx value = 0x53 (See UG)
	UCA1BR1 = 0;
	UCA1CTL1 &= ~UCSWRST;                     // Initialize eUSCI
	UCA1IE |= UCRXIE | UCTXCPTIE;//| UCTXIE;
	__bis_SR_register(GIE);
	uartData.txQ = initQueue(BUFFLEN);
	//uartData.rxQ = initQueue(BUFFLEN);
	__eint();
	//putString("This is a test");

}

int getString(char* str, int len){
	len--;
	char temp;
	int i;
	i = 0;
	while (numEnqueued(uartData.rxQ) >0){
		__dint();
		temp = dequeue(uartData.rxQ);
		__eint();
		if (temp == '\r' | temp == '\n'){
			// Then weve reached the end of a line.
			temp = peek(uartData.rxQ);
			if (temp == '\r' | temp == '\n'){
				dequeue(uartData.rxQ);
				str[i] = 0;
				return i;
			} else {
				// Then we dont need to dequeue.
				str[i] = 0;
				return i;
			}
		} else {
			// Then were going good.
			str[i++] = temp;
		}
		if (i == len){
			// end of buf.
			str[i] = 0;
			return i;
		}
	}
	// Then out of queue.
	str[i] = 0;
	return i;
}
void putString(char* str){
	int i;
	char curr;
	i = 0;
	curr = str[i];
	while (curr){
		putChar(curr);
		curr = str[++i];
	}
	//UCA0IE |=  UCTXIE;
}
void putChar(char ch){
	if (uartData.txQ->stored == uartData.txQ->capacity) {
		UCA0IFG |= UCTXIFG;
	}
	while ((uartData.txQ->stored == uartData.txQ->capacity) & uartData.txQ->stored > 0) {
		UCA0IE |=  UCTXIE;
	}
	__dint();
	enqueue(uartData.txQ, ch);
	__eint();
	/*UARTTXBuf[UARTTXLen++] = ch;*/
	UCA0IE |=  UCTXIE;

}

char getChar(void){
	char ret;
	int retryCount;
	retryCount = 0;
	while (retryCount < MAXGETRETRIES){
		if (numEnqueued(uartData.rxQ)){
			__dint();
			ret = dequeue(uartData.rxQ);
			__eint();
			return ret;
		}
	}
	return 0x35;
}

void putNum(int num){
	char ch;
	ch = (num / 10000) + '0';
	putChar(ch);
	num %= 10000;
	ch = (num / 1000) + '0';
	putChar(ch);
	num %= 1000;
	ch = (num / 100) + '0';
	putChar(ch);
	num %= 100;
	ch = (num / 10) + '0';
	putChar(ch);
	num %= 10;
	ch = num + '0';
	putChar(ch);

}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
	char temp;
 	__dint();
	switch(__even_in_range(UCA1IV, USCI_UART_UCTXCPTIFG)){
	//if ( USCI_NONE){}
	//if ( UCA0IV & USCI_UART_UCRXIFG){
	/*if (UARTRXLen < MAXRXBUFF){*/
	/*UARTRXBuf[UARTRXLen++] = UCA0RXBUF;*/
	/*}*/
	case USCI_NONE: break;
	case USCI_UART_UCRXIFG:
		//enqueue(uartData.rxQ, (UCA0RXBUF));
		temp = UCA1RXBUF;
		//putChar(temp);
		ParseGPS(temp);
		__no_operation();
		break;
	case USCI_UART_UCTXIFG:
		//if( UCA0IV & USCI_UART_UCTXIFG){
		if (numEnqueued (uartData.txQ)){
			// Then we actually have something to send.
			UCA1TXBUF = dequeue(uartData.txQ);
		} else {
			UCA1IE &=  ~UCTXIE;
		}	
		/*if (UARTTXOutIndex < UARTTXLen){*/
		/*while(!(UCA0IFG & UCTXIFG));*/
		/*UCA0TXBUF = UARTTXBuf[UARTTXOutIndex++];*/
		/*} else {*/
		/*UARTSending = 0;*/
		/*UCA0IE &=  ~UCTXIE;*/
		/*UARTTXOutIndex =x;*/
		/*UARTTXLen = 0;*/
		/*}*/
		break;
	case USCI_UART_UCSTTIFG:
		if (numEnqueued (uartData.txQ)){
			// Then we actually have something to send.
			UCA1TXBUF = dequeue(uartData.txQ);
		} else {
			UCA1IE &=  ~UCTXIE;
		}
		break;
	case USCI_UART_UCTXCPTIFG: break;

	}
	//LED1_TOGGLE();
	__eint();
}
