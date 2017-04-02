/*
 * Voltmeter.c
 *
 * Created: 2/28/2017 10:28:18 AM
 * Author : Tyler
 */ 

#define F_CPU 1000000UL

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

// LCD pins
#define rs PD0
#define rw PD1
#define en PD2
#define data PORTB

void lcd_init();						// clears LCD screen and sets positions
void lcd_cmd(char cmd_out);				// sends a command across the LCD command bits
void lcd_data(char data_out);			// sends data across the LCD data register
void lcd_str(char *str);				// prints a string to the LCD screen
void adc_init();						// sets the reference voltage and clock prescaler for the ADC
uint16_t read_adc(uint8_t channel);		// reads and returns the voltage input at an ADC channel

int main()
{
	float realV0;			// voltage at ADC channel 0
	int intV0;
	float diffV0;
	int decV0;
	char number[16];	// character array for converting doubles to string
	
	DDRB=0XFF;			// Port B: data register for LCD - set to output
	DDRD=0XFF;			// Port D: command bits for LCD - set to output
	
	adc_init();
	
	while(1)
	{
		lcd_init();
		realV0 = (float)(read_adc(7)*5.0/1023.0);	// read ADC channel 0 and calculate voltage
		intV0 = (int) realV0;
		diffV0 = realV0 - (float)intV0;
		decV0 = (int) (diffV0*1000);
		
		if(decV0 < 10){
			sprintf(number, "%u.00%u V", intV0, decV0);		// convert voltage to string
		}
		else if(decV0 < 100){
			sprintf(number, "%u.0%u V", intV0, decV0);		// convert voltage to string
		}
		else sprintf(number, "%u.%u V", intV0, decV0);		// convert voltage to string
		
		lcd_str(number);					// print voltage
		_delay_ms(500);
	}
}

void lcd_init()
{
	lcd_cmd(0X38);				// initialize 2 lines, 5x7 font for 16x2 LCD
	lcd_cmd(0X0C);				// display on, cursor off
	lcd_cmd(0X01);				// clear screen
	lcd_cmd(0X83);				// set position
	lcd_str("Voltmeter");
	lcd_cmd(0xC0);				// set position
}

void lcd_cmd(char cmd_out)
{
	data=cmd_out;					// set the command across LCD data register
	PORTD=(0<<rs)|(0<<rw)|(1<<en);	// enable LCD to receive command
	_delay_ms(10);
	PORTD=(0<<rs)|(0<<rw)|(0<<en);	// latch last command
	_delay_ms(10);
}

void lcd_data(char data_out)
{
	data=data_out;					// set the data across the LCD data register	
	PORTD=(1<<rs)|(0<<rw)|(1<<en);	// enable LCD to receive data
	_delay_ms(10);
	PORTD=(1<<rs)|(0<<rw)|(0<<en);	// latch last data
	_delay_ms(10);
}

void lcd_str(char *str)
{
	unsigned int i=0;
	while(str[i]!='\0')		// print each character in string to LCD
	{
		lcd_data(str[i]);
		i++;
	}
}

void adc_init()
{
	ADMUX = (1<<REFS0);	// Aref = Vcc
	
	/* the ADC needs a clock signal between 50kHz and 200kHz, with internal 8MHz clock,
	   a prescaler of 64 gives the ADC a clock of 125kHz */
	
	ADCSRA = (1<<ADPS2)|(1<<ADPS1)|(1<<ADEN);	// enable ADC, set prescaler to 64
}

uint16_t read_adc(uint8_t channel)
{
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);	// select ADC channel
	ADCSRA |= (1<<ADSC);						// select single conversion mode
	while( ADCSRA & (1<<ADSC) );				// wait until ADC conversion is complete
	return ADC;
}