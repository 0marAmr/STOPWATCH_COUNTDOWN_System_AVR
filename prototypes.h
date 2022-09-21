/*
 *  Created on: Sep 21, 2022
 *  Author: Omar Amr Mahmoud Hafz
 */

#ifndef PROTOTYPES_H_
#define PROTOTYPES_H_

#define TICKS_IN_ONE_SEC 15625
#define HR_MIN_SEC 3
#define BUTTON_DELAY 10 				// delay time to avoid bouncing effect
#define LED_DELAY 10 					// delay time to display the output on the segments
//defining  input pins for the Navigation device
#define LD (PIND & (1<<PD0))
#define EN (PIND & (1<<PD1))
#define LEFT (PINC & (1<<PC4))
#define RIGHT (PINC & (1<<PC5))
#define INC (PINC & (1<<PC6))
#define DEC (PINC & (1<<PC7))
#define HOURS_SEGMENT 0x10
#define SECONDS_SEGMENT 0x01

typedef unsigned char uint8;
typedef enum {
	FALSE, TRUE
}bool;

enum{
	SECS, MINS, HRS
};
enum{
	COUNTDOWN_TIMER, STOPWATCH
};

void Timer1_CTC_INIT(void);
void display_INIT(void);
void load_time_INIT(void);
void RESET_Interrupt_INIT(void); 		//INT0
void PAUSE_Interrupt_INIT(void);		//INT1
void RESUME_Interrupt_INIT(void);		//INT2
void tick_inc_dec(uint8 * const time);
void user_inc_dec(uint8 * data, uint8 shifter, bool inc);
void display(const uint8 * const time_now);
void load_time(uint8 * const time_now);

#endif /* PROTOTYPES_H_ */
