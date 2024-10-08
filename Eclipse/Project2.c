#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define compare_value 15625 //Compare value for Prescaler=1024

unsigned char seconds_unit = 0;
unsigned char seconds_ten = 0;
unsigned char minutes_unit = 0;
unsigned char minutes_ten = 0;
unsigned char hours_unit = 0;
unsigned char hours_ten = 0;
unsigned char flag_hours_increment = 0;
unsigned char flag_minutes_increment = 0;
unsigned char flag_seconds_increment = 0;
unsigned char flag_hours_decrement = 0;
unsigned char flag_minutes_decrement = 0;
unsigned char flag_seconds_decrement = 0;
unsigned char flag_toggle = 0;
unsigned char toggle = 1;
unsigned char last_button_state = 0;

//Function Prototypes
void Display(void); //Function to display on seven-segment
void Timer1_CTC(void); //Function for timer1 compare mode
void Define_Port(void); //Function to define all port
void Interrupt0(void); //Function for interrupt0
void Interrupt1(void); //Function for interrupt1
void Interrupt2(void); //Function for interrupt2
void Hours_Increment(void); //Function to increase hours
void Minutes_Increment(void); //Function to increase minutes
void Seconds_Increment(void); //Function to increase seconds
void Hours_Decrement(void); //Function to decrease hours
void Minutes_Decrement(void); //Function to decrease minutes
void Seconds_Decrement(void); //Function to decrease seconds
void Count_up(void); //Function count up the seven-segment
void Count_down(void); //Function count down the seven-segment
void Toggle(void); //Function to toggle to change count mode
void Count_up_led(void); //Function to turn on led when count up is active
void Count_down_led(void); //Function to turn on led when count down is active
void Toggle_LED(void); //Function to toggle led when count mode change


//Interrupt Service Routine for Timer1 Compare Match A
ISR(TIMER1_COMPA_vect)
{
	// Check the state of the 'toggle' variable to determine which counting function to call
	if(toggle == 1)
	{
		Count_up();
	}
	else
	{
		Count_down();
	}
}


//Interrupt Service Routine for External Interrupt 0 (INT0)
ISR(INT0_vect)
{
	TCNT1=0; //Reset the Timer/Counter1 value to 0
	// Reset the units and tens place of seconds, minutes, and hours to 0
	seconds_unit=0;
	seconds_ten=0;
	minutes_unit=0;
	minutes_ten=0;
	hours_unit=0;
	hours_ten=0;
}


// Interrupt Service Routine for External Interrupt 1 (INT1)
ISR(INT1_vect)
{
	// Stop Timer/Counter1 by clearing the prescaler bits
	TCCR1B &=~ (1<<CS10);
	TCCR1B &=~ (1<<CS11);
	TCCR1B &=~ (1<<CS12);
}


// Interrupt Service Routine for External Interrupt 2 (INT2)
ISR(INT2_vect)
{
	TCCR1B |= (1<<CS12) | (1<<CS10); // Start or resume Timer/Counter1 by setting the prescaler bits
}


void Display(void)
{
	//Display Seconds_unit
	PORTA=0X20;
	PORTC =(PORTC & 0XF0) | (seconds_unit & 0X0F);
	_delay_ms(2);
	//Display Seconds_ten
	PORTA=0X10;
	PORTC =(PORTC & 0XF0) | (seconds_ten & 0X0F);
	_delay_ms(2);
	//Display Minutes_unit
	PORTA=0X08;
	PORTC =(PORTC & 0XF0) | (minutes_unit & 0X0F);
	_delay_ms(2);
	//Display Minutes_ten
	PORTA=0X04;
	PORTC =(PORTC & 0XF0) | (minutes_ten & 0X0F);
	_delay_ms(2);
	//Display Hours_unit
	PORTA=0X02;
	PORTC =(PORTC & 0XF0) | (hours_unit & 0X0F);
	_delay_ms(2);
	//Display Hours_ten
	PORTA=0X01;
	PORTC =(PORTC & 0XF0) | (hours_ten & 0X0F);
	_delay_ms(2);
}

void Timer1_CTC(void)
{
	TCNT1 = 0; // Set timer1 initial number to zero
	OCR1A = compare_value; //Set Compare value for Prescaler=1024

	TCCR1A = (1<<FOC1A);
	/* Configure timer control register TCCR1A
	 * 1. COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * 3. CTC Mode WGM10=0 WGM11=0 (Mode Number 4) */

	TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);
	/* Configure timer control register TCCR1B
	* 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	* 2. Prescaler = F_CPU/1024 CS10=0 CS10=1 CS12=1 */

	TIMSK = (1<<OCIE1A); // Enable Timer1 Compare A Interrupt
}

void Define_Port(void)
{
//Port D
	DDRD |= (1<<PD5); // PD5 Output pin(Count down led)
	PORTD &=~ (1<<PD5); //Led of PD5 off at start

	DDRD |= (1<<PD4); // PD4 Output pin(Count up led)
	PORTD |= (1<<PD4); // Led of PD4 off at start

	DDRD &=~ (1<<PD3); // PD3 Input pin(Button for Pause)

	DDRD &=~ (1<<PD2); // PD2 Input pin(Button for Reset)
	PORTD |= (1<<PD2); // Enable pull-up resistor on PD2

	DDRD |= (1<<PD0); // PD0 Output pin(Alarm led)
	PORTD &=~ (1<<PD0); // Led of PD0 off at start

//Port C
	DDRC |= 0X0F; // Set first 4 bits in Port C as output
	PORTC &= 0XF0; // Clear first 4 bits in Port C

//Port B
	DDRB &=~ (1<<PB7); // PB7 Input pin(Button for Count mode)
	PORTB |= (1<<PB7); //Enable pull-up resistor on PB7

	DDRB &=~ (1<<PB6); // PB6 Input pin(Button for Seconds Increment)
	PORTB |= (1<<PB6); // Enable pull-up resistor on PB6

	DDRB &=~ (1<<PB5); // PB5 Input pin(Button for Seconds Decrement)
	PORTB |= (1<<PB5); // Enable pull-up resistor on PB5

	DDRB &=~ (1<<PB4); // PB4 Input pin(Button for Minutes Increment)
	PORTB |= (1<<PB4); // Enable pull-up resistor on PB4

	DDRB &=~ (1<<PB3); // PB3 Input pin(Button for Minutes Decrement)
	PORTB |= (1<<PB3); // Enable pull-up resistor on PB3

	DDRB &=~ (1<<PB2); // PB2 Input pin(Button for Resume)
	PORTB |= (1<<PB2); // Enable pull-up resistor on PB2

	DDRB &=~ (1<<PB1); // PB1 Input pin(Button for Hours Increment)
	PORTB |= (1<<PB1); // Enable pull-up resistor on PB1

	DDRB &=~ (1<<PB0); // PB0 Input pin(Button for Hours Decrement)
	PORTB |= (1<<PB0); // Enable pull-up resistor on PB0

//Port A
	DDRA |= 0X3F; // Set first 6 bits in Port A as output
	PORTA &= 0XC0; // Clear first 6 bits in Port A

}

void Interrupt0(void)
{
	MCUCR |= (1<<ISC01); //Set ISC01 bit to 1 (falling edge trigger)
	GICR |= (1<<INT0); //Set INT0 bit to 1 (enable INT0)
}

void Interrupt1(void)
{
	MCUCR |= (1<<ISC10) | (1<<ISC11); //Set ISC11 and ISC10 bits to 1 (rising edge trigger)
	GICR |= (1<<INT1); //Set INT1 bit to 1 (enable INT1)
}

void Interrupt2(void)
{
	GICR |= (1<<INT2); //Set INT2 bit to 1 to enable INT2
}

void Hours_Increment(void)
{
	if (!(PINB & (1 << PB1))) //Check if button is pressed (active low)
	{
		_delay_ms(30); //Debounce delay
		if (!(PINB & (1 << PB1))) //Check if button is still pressed
		{
			if(flag_hours_increment == 0) // Proceed to increment hours only if not already incrementing
			{
				if(hours_unit == 9) //Check hours unit to prevent overflow
				{
					hours_unit = 0; //Set hours unit to 0
					if(hours_ten == 9) //Check hours ten to prevent overflow
					{
						hours_ten = 9; //Set hours ten to 9
						hours_unit = 9; //Set hours unit to 9
					}
					else
					{
						hours_ten++; //Increment hours ten
					}
				}
				else
				{
					hours_unit++; //Increment hours unit
				}
				flag_hours_increment = 1; //Set the flag to indicate that hours have been incremented
			}
		}
	}
	else
	{
		flag_hours_increment = 0; //Reset the increment flag if the button is not pressed
	}
}

void Minutes_Increment(void)
{
	if (!(PINB & (1 << PB4))) // Check if button is pressed (active low)
	{
		_delay_ms(30); //Debounce delay
		if (!(PINB & (1 << PB4))) //Check if button is still pressed
		{
			if(flag_minutes_increment == 0) //Proceed to increment minutes only if not already incrementing
			{
				if(minutes_unit == 9) //Check minutes unit to prevent overflow
				{
					minutes_unit = 0; //Set minutes unit to 0
					if(minutes_ten == 5) //Check minutes ten to prevent overflow
					{
						minutes_ten = 5; //Set minutes ten to 5
						minutes_unit = 9; //Set minutes unit to 9
					}
					else
					{
						minutes_ten++; //Increment minutes ten
					}
				}
				else
				{
					minutes_unit++; //Increment minutes unit
				}
				flag_minutes_increment = 1; //Set the flag to indicate that minutes have been incremented
			}
		}
	}
	else
	{
		flag_minutes_increment = 0; //Reset the increment flag if the button is not pressed
	}
}

void Seconds_Increment(void)
{

	if (!(PINB & (1 << PB6))) //Check if button is pressed (active low)
	{
		_delay_ms(30); //Debounce delay
		if (!(PINB & (1 << PB6))) //Check if button is still pressed
		{
			if(flag_seconds_increment == 0) //Proceed to increment seconds only if not already incrementing
			{
				if(seconds_unit == 9) //Check seconds unit to prevent overflow
				{
					seconds_unit = 0; //Set seconds unit to 0
					if(seconds_ten == 5) //Check seconds ten to prevent overflow
					{
						seconds_ten = 5; //Set seconds ten to 5
						seconds_unit = 9; //Set seconds unit to 9
					}
					else
					{
						seconds_ten++; //Increment seconds ten
					}
				}
				else
				{
					seconds_unit++; //Increment seconds unit
				}
				flag_seconds_increment = 1; //Set the flag to indicate that seconds have been incremented
			}
		}
	}
	else
	{
		flag_seconds_increment = 0; //Reset the increment flag if the button is not pressed
	}

}

void Hours_Decrement(void)
{
	if (!(PINB & (1 << PB0))) //Check if button is pressed (active low)
	{
		_delay_ms(30); //Debounce delay
		if (!(PINB & (1 << PB0))) //Check if button is still pressed
		{
			if(flag_hours_decrement == 0) //Proceed to decrement hours only if not already decrementing
			{
				if(hours_unit == 0) //Check hours unit if equal zero
				{
					hours_unit = 9; //Set hours unit to 9
					if(hours_ten == 0) //Check hours ten if equal zero
					{
						hours_unit=0; //Set hours unit to 0
						hours_ten=0; //Set hours ten to 0
					}
					else
					{
						hours_ten--; //Decrement hours ten
					}
				}
				else
				{
					hours_unit--; //Decrement hours unit
				}
				flag_hours_decrement = 1; //Set the flag to indicate that hours have been decremented
			}
		}
	}
	else
	{
		flag_hours_decrement = 0; //Reset the decrement flag if the button is not pressed
	}
}

void Minutes_Decrement(void)
{
	if (!(PINB & (1 << PB3))) //Check if button is pressed (active low)
	{
		_delay_ms(30); //Debounce delay
		if (!(PINB & (1 << PB3))) //Check if button is still pressed
		{
			if(flag_minutes_decrement == 0) //Proceed to decrement minutes only if not already decrementing
			{
				if(minutes_unit == 0) //Check minutes unit if equal zero
				{
					minutes_unit = 9; //Set minutes unit to 9
					if(minutes_ten == 0) //Check minutes ten if equal zero
					{
						minutes_unit=0; //Set minutes unit to 0
						minutes_ten=0; //Set minutes ten to 0
					}
					else
					{
						minutes_ten--; //Decrement minutes ten
					}
				}
				else
				{
					minutes_unit--; //Decrement minutes unit
				}
				flag_minutes_decrement = 1; //Set the flag to indicate that minutes have been decremented
			}
		}

	}
	else
	{
		flag_minutes_decrement = 0; //Reset the decrement flag if the button is not pressed
	}
}

void Seconds_Decrement(void)
{
	if (!(PINB & (1 << PB5))) //Check if button is pressed (active low)
	{
		_delay_ms(30); //Debounce delay
		if (!(PINB & (1 << PB5))) //Check if button is still pressed
		{
			if(flag_seconds_decrement == 0) //Proceed to decrement seconds only if not already decrementing
			{
				if(seconds_unit == 0) //Check seconds unit if equal zero
				{
					seconds_unit = 9; //Set seconds unit to 9
					if(seconds_ten == 0) //Check seconds ten if equal zero
					{
						seconds_unit=0; //Set seconds unit to 0
						seconds_ten=0; //Set seconds ten to 0
					}
					else
					{
						seconds_ten--; //Decrement seconds ten
					}
				}
				else
				{
					seconds_unit--; //Decrement seconds unit
				}
				flag_seconds_decrement = 1; //Set the flag to indicate that seconds have been decremented
			}
		}

	}
	else
	{
		flag_seconds_decrement = 0; //Reset the decrement flag if the button is not pressed
	}
}

void Toggle(void)
{
	if (!(PINB & (1 << PB7))) //Check if button is pressed (active low)
	{
		_delay_ms(30); //Debounce delay
		if (!(PINB & (1 << PB7))) //Check if button is still pressed
		{
			if(flag_toggle == 0) //Proceed to toggle only if not already toggled
			{
				toggle = !toggle;    // Toggle the value of toggle (0 -> 1 or 1 -> 0)
				flag_toggle = 1; // Set the flag to indicate that toggle has been processed
			}
		}
	}
	else
	{
		flag_toggle = 0; //Reset the toggle flag if the button is not pressed
	}
}

void Count_up(void)
{
	Count_up_led(); //Call function to handle LED indication related to counting up
	if(seconds_unit == 9) //Check seconds unit to prevent overflow
	{
		seconds_unit = 0; //Reset seconds unit to 0
		if(seconds_ten == 5) //Check seconds ten to prevent overflow
		{
			seconds_ten = 0; //Reset seconds ten to 0
			if(minutes_unit == 9) //Check minutes unit to prevent overflow
			{
				minutes_unit = 0; //Reset minutes unit to 0
				if(minutes_ten == 5 ) //Check minutes ten to prevent overflow
				{
					minutes_ten = 0; //Reset minutes ten to 0
					if(hours_unit == 9) //Check hours unit to prevent overflow
					{
						hours_unit = 0; //Reset hours unit to 0
						if(hours_ten == 9) //Check hours ten to prevent overflow
						{
							hours_ten = 0; //Reset hours ten to 0
						}
						else
						{
							hours_ten++;  //Increment hours ten
						}
					}
					else
					{
						hours_unit++; //Increment hours unit
					}
				}
				else
				{
					minutes_ten++; //Increment minutes ten
				}
			}
			else
			{
				minutes_unit++; //Increment minutes unit
			}
		}
		else
		{
			seconds_ten++; //Increment seconds ten
		}
	}
	else
	{
		seconds_unit++; //Increment seconds unit
	}
}

void Count_down(void)
{
	Count_down_led(); //Call function to handle LED indication related to counting down
	if(seconds_unit == 0) //Check seconds unit
	{
		seconds_unit = 9; //Set seconds unit to 9
		if(seconds_ten == 0) //Check seconds ten
		{
			seconds_ten = 5; //Set seconds ten to 5
			if(minutes_unit == 0) //Check minutes unit
			{
				minutes_unit = 9; //Set minutes unit to 9
				if(minutes_ten == 0 ) //Check minutes ten
				{
					minutes_ten = 5; //Set minutes ten to 5
					if(hours_unit == 0) //Check hours unit
					{
						hours_unit = 9; //Set hours unit to 9
						if(hours_ten == 0) //Check hours ten
						{
							hours_ten = 0; //Reset hours ten to 0
							hours_unit = 0; //Reset hours unit to 0
							minutes_ten = 0; //Reset minutes ten to 0
							minutes_unit = 0; //Reset minutes unit to 0
							seconds_ten = 0; //Reset seconds ten to 0
							seconds_unit = 0; //Reset seconds unit to 0
							PORTD |= (1<<PD0); //Set the PD0 bit to turn on the LED and Buzzer
						}
						else
						{
							hours_ten--; //Decrement hours ten
						}
					}
					else
					{
						hours_unit--; //Decrement hours unit
					}
				}
				else
				{
					minutes_ten--; //Decrement minutes ten
				}
			}
			else
			{
				minutes_unit--; //Decrement minutes unit
			}
		}
		else
		{
			seconds_ten--; //Decrement seconds ten
		}
	}
	else
	{
		seconds_unit--; //Decrement seconds unit
	}
}

void Count_up_led(void)
{
	PORTD &=~ (1<<PD5); //Clear the PD5 bit to turn off the LED
	PORTD |= (1<<PD4); //Set the PD4 bit to turn on the LED
	PORTD &=~ (1<<PD0); //Clear the PD0 bit to turn off the LED
}

void Count_down_led(void)
{
	PORTD |= (1<<PD5); //Set the PD5 bit to turn off the LED
	PORTD &=~ (1<<PD4); //Clear the PD4 bit to turn on the LED
	PORTD &=~ (1<<PD0); //Clear the PD0 bit to turn off the LED
}

void Toggle_LED(void)
{
	if (!(PINB & (1 << PB7))) //Check if button is pressed (active low)
	{
		_delay_ms(10);  //Debounce delay
		if (last_button_state == 0) //Check if button was previously released
		{
			// Toggle LEDs connected to PD4 and PD5
			PORTD ^= (1 << PD4) | (1 << PD5);
			last_button_state = 1; //Update the last button state
		}
	}
	else
	{
		last_button_state = 0; //Update the button state when it's released
	}

}

int main(void)
{
	SREG |= (1<<7); //Enable global interrupt
	/*Calling Function*/
	Define_Port();
	Timer1_CTC();
	Interrupt0();
	Interrupt1();
	Interrupt2();
	while(1)
	{
		/*Calling Function*/
		Display();
		Toggle();
		Toggle_LED();
		Hours_Increment();
		Hours_Decrement();
		Minutes_Increment();
		Minutes_Decrement();
		Seconds_Increment();
		Seconds_Decrement();
	}
}
