/*
    Copyright (C) 2014 <>< Charles Lohr


    Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

void delay_ms(uint32_t time) {
  uint32_t i;
  for (i = 0; i < time; i++) {
    _delay_ms(1);
  }
}

#define NOOP asm volatile("nop" ::)

void SendTick();
void SendAPacket();
void SendBPacket();
void SendCPacket();

static void setup_clock( void )
{
	/*Examine Page 33*/

	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;	/*No scalar*/

	//Maximum clock speeds!
	PLLCSR = _BV(PLLE) | _BV( PCKE );
	OSCCAL = 0xff;
}


int main( )
{
	int i;
	cli();

	setup_clock();

	DDRB &= _BV(2); //Sound input
	DDRB |= _BV(3) | _BV(4); //Speaker

	//Processor is operating at ~25 MHz, or a 100? MHz main clock???
	//Let's try using the timer as-is, 8 bit fast PWM.
	//120 MHz?? / 256 cycles = 500 kHz... That seems high enough.

	//CONNECT SPEAKER PIN PB3 to PB4
	//PB3 = ~OC1B~
	//PB4 =  OC1B

	//CONNECT Electret Mic between GND and PB2, with a resistor from PB2 to VCC.

	TCCR1 = _BV(CS10); //Use high speed PCK.
	GTCCR = _BV(PWM1B) | _BV(COM1B0);
	//Make sure PLLE is already selected.

	//Turn on the ADC.
	//ADC1 = PB2 (MUX = 1)
	//REFS = 0 to 5V ADC.
	//Set ADC to free run, with MSB 8 bits in ADCH. (ADLAR = 1)
	//Set ADC clock to system / 16.
	ADMUX = /*_BV(ADLAR) |*/1;
	ADCSRA = _BV(ADEN) | _BV(ADSC) | _BV(ADATE) | _BV(ADPS2) | _BV(ADPS1);
	while(1)
	{
		//This will cause OCR1B to make a triangle wave, going up over time,
		//then resetting to 0 every 256 cycles.
		//If you wait 100 us between cycles, that is 1/(.00001*256)
		//Or around 390 Hz.
		//Measured voltage = 2.05V / 4.85 = 0.422 * 256 = 108 is the center.

		int16_t i = ADC;
		i -= 104*4;

		i *= 5;

		if( i > 127 ) i = 127;
		if( i < -127 ) i = -127;

		OCR1B = i + 127;
	}
	
	return 0;
} 
