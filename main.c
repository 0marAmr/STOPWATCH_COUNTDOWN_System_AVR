/*
 *  Created on: Sep 21, 2022
 *  Author: Omar Amr Mahmoud Hafz
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "prototypes.h"

bool g_tick_flag = FALSE, g_reset = FALSE;
uint8 device_status = STOPWATCH; // device starts as stopwatch by default

ISR(TIMER1_COMPA_vect)
{
	g_tick_flag = TRUE;
}

ISR(INT0_vect)
{ // reset timer 1
	g_reset = TRUE;
	device_status = STOPWATCH; // reseting the timer implies Stopwatch operation
}

ISR(INT1_vect)
{
	TCCR1B &= ~(1 << CS11) & ~(1 << CS10); // pause timer 1
}

ISR(INT2_vect)
{
	TCCR1B |= (1 << CS11) | (1 << CS10); // resume timer 1 (prescaler=64)
}

int main(void)
{
	uint8 time[HR_MIN_SEC] = {0}; // Initially time is zero by default
	// initialization code
	Timer1_CTC_INIT();
	display_INIT();
	load_time_INIT();
	RESET_Interrupt_INIT();
	PAUSE_Interrupt_INIT();
	RESUME_Interrupt_INIT();
	sei(); // set global interrupt bit.
	while (1)
	{
		if (g_reset)
		{
			g_reset = FALSE;
			time[2] = time[1] = time[0] = 0; // reset time
		}
		if (g_tick_flag)
			tick_inc_dec(time);
		display(time);
		if (EN && device_status == STOPWATCH) // requesting COUNTDOWN_TIMER operation
			load_time(time);
	}
}

/* F_CPU = 1 MHz , N = 64
 * T_tick = 64 us
 * Ticks in a second = 15625
 * */
void Timer1_CTC_INIT(void)
{
	TCNT1 = 0;										   // initially the timer is set to zero.
	TCCR1A = (1 << FOC1A);							   // CTC mode: FOC1A is set.
	TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10); // config. prescaler = 64.
	OCR1A = TICKS_IN_ONE_SEC;
	TIMSK = (1 << OCIE1A); // set output compare A interrupt enable bit.
}

/* initial pin configuration of Port A and Port C */
void display_INIT(void)
{
	DDRA |= 0x3F;  // config. PA5:0 as op pins.
	PORTA &= 0xC0; // initially all the segments are OFF.

	DDRC |= 0x0F;  // config. PC3:0 as op pins.
	PORTC &= 0xF0; // initially the segment displays zero output.
}

/* initialize PC7:4 and PD1:0 as ip pins to manipulate time  */
void load_time_INIT(void)
{
	DDRC &= 0x0F; // config. PC7:4 as ip pins.
	DDRD &= 0xFC; // config. PD1:0 as ip pins.
}

void RESET_Interrupt_INIT(void)
{
	MCUCR |= (1 << ISC01); // config. INT0 to generate ISR with falling edge
	GICR |= (1 << INT0);   // enable external interrupt req.
	DDRD &= ~(1 << PD2);   // config. PD2 as ip pin
	PORTD |= (1 << PD2);   // enable internal pull up config.
}

void PAUSE_Interrupt_INIT(void)
{
	MCUCR |= (1 << ISC11) | (1 << ISC10); // config. INT1 to generate ISR with falling edge
	GICR |= (1 << INT1);				  // enable external interrupt req.
	DDRD &= ~(1 << PD3);				  // config. PD3 as ip pin
}

void RESUME_Interrupt_INIT(void)
{
	MCUCSR |= (1 << ISC2);
	GICR |= (1 << INT2); // enable external interrupt req.
	DDRB &= ~(1 << PB2); // config. PD3 as ip pin
	PORTB |= (1 << PB2); // enable internal pull up config.
}

/* increment/decrement time via timer tick event */
void tick_inc_dec(uint8 *const time)
{
	if (device_status == STOPWATCH)
	{
		if (++time[SECS] == 60)
		{ // inc. secs
			time[SECS] = 0;
			time[MINS]++; // inc. mins
		}
		if (time[MINS] == 60)
		{
			time[MINS] = 0;
			time[HRS]++; // inc hrs
		}
		if (time[HRS] == 99 && time[MINS] == 59 && time[SECS] == 59)
			time[HRS] = time[MINS] = time[SECS] = 0; // timer overflows ( you'll need to simulate
	}												 // for 4 days to make sure, good luck with that)

	else if (device_status == COUNTDOWN_TIMER)
	{
		if (time[SECS]-- == 0)
		{ // dec. secs
			time[SECS] = 59;
			time[MINS]--; // dec. mins
		}
		if (time[MINS] > 60) // unsigned num overflows
		{
			time[MINS] = 0;
			time[HRS]--; // dec. hrs
		}
		if (time[HRS] == 0 && time[MINS] == 0 && time[SECS] == 0)
		{
			time[HRS] = 99; //  underflow (if that makes any sence)
			time[MINS] = time[SECS] = 59;
		}
	}
	g_tick_flag = FALSE;
}

/*displays time on all the segments using multiplexed tech.*/
void display(const uint8 *const time_now)
{
	uint8 shifter = 1;
	for (uint8 i = 0; i < 3; i++)
	{
		// units
		PORTA = (PORTA & 0xC0) | (shifter);
		PORTC = (PORTC & 0xF0) | ((time_now[i] % 10) & 0x0F);
		_delay_us(LED_DELAY);
		shifter <<= 1;

		// tenths
		PORTA = (PORTA & 0xC0) | (shifter);
		PORTC = (PORTC & 0xF0) | ((time_now[i] / 10) & 0x0F);
		_delay_us(LED_DELAY);
		shifter <<= 1;
	}
}

/* Functions defined below control the NAVIGATION DEVICE (extra freature added by me)*/
/*increments/decrements the current segment the user chooses*/
void user_inc_dec(uint8 *data, uint8 shifter, bool inc)
{
	if (inc)
	{
		if (shifter == HOURS_SEGMENT && *data < 99)
			(*data)++;
		else if (*data < 59)
			(*data)++;
	}
	else
	{
		if (*data > 0)
			(*data)--;
	}
}

/*displays time on two segments only.*/
void dual_segment_display(const uint8 data, uint8 shifter)
{
	PORTA = (PORTA & 0xC0) | (shifter);
	PORTC = (PORTC & 0xF0) | ((data % 10) & 0x0F);
	_delay_us(LED_DELAY);
	shifter <<= 1;

	PORTA = (PORTA & 0xC0) | (shifter);
	PORTC = (PORTC & 0xF0) | ((data / 10) & 0x0F);
	_delay_us(LED_DELAY);
}

/*input the initial countdown time*/
void load_time(uint8 *const time_now)
{
	TCCR1B &= ~(1 << CS11) & ~(1 << CS10); // pause timer 1
	PORTC &= 0xF0;						   // all segments are reset to zero to choose time
	uint8 shifter = 1;
	uint8 temp[HR_MIN_SEC] = {0}, i = 0;
	while (EN)
	{
		dual_segment_display(temp[i], shifter);
		if (INC)
		{
			_delay_ms(BUTTON_DELAY);
			if (INC)
			{
				user_inc_dec(&temp[i], shifter, TRUE);
				while (INC)
					; // do nothing (capture rising edge only)
			}
		}
		else if (DEC)
		{
			_delay_ms(BUTTON_DELAY);
			if (DEC)
			{
				user_inc_dec(&temp[i], shifter, FALSE);
				while (DEC)
					; // do nothing (capture rising edge only)
			}
		}
		else if (LEFT)
		{
			_delay_ms(BUTTON_DELAY);
			if (LEFT)
			{
				if (shifter != HOURS_SEGMENT)
				{ // the leftmost displayed segments are Hrs
					shifter <<= 2;
					i++;
				}
				while (LEFT)
					; // do nothing (capture rising edge only)
			}
		}
		else if (RIGHT)
		{
			_delay_ms(BUTTON_DELAY);
			if (RIGHT)
			{
				if (shifter != 0x1)
				{ // the rightmost displayed segments are Secs.
					shifter >>= 2;
					i--;
				}
				while (RIGHT)
					; // do nothing (capture rising edge only)
			}
		}

		if (LD)
		{
			_delay_ms(BUTTON_DELAY);
			if (LD)
			{
				device_status = COUNTDOWN_TIMER;
				for (uint8 i = 0; i < 3; i++)
					time_now[i] = temp[i]; // copy the entered time
				while (LD)
					;  // do nothing (capture rising edge only)
				break; // exit the loop (input has been taken)
			}
		}
	}
	TCCR1B |= (1 << CS11) | (1 << CS10); // resume timer 1 (prescaler=64)
}
