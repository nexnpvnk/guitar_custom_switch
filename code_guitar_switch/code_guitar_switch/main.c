/*
 * Indicator.c
 *
 * Created: 24.03.2024 21:39:06
 * Author : nexnpvnk
 */

#define F_CPU 16000000UL
#define _BV(bit) (1 << (bit))


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#ifdef _cplusplus
	extern "C" {
#endif


// choose an AVR timer as systick
#define TIMERx 0

#if TIMERx == 0
#define TCNTx TCNT0
#define start_count 0x06
#define SET_TIMERx_PRESCALER() TCCR0B |= _BV(CS01) | _BV(CS00);		// prescaler 64 if 16 MHz, prescaler 8 for modbus
#define ENABLE_TIMERx_INTERRUPT() TIMSK0 |= _BV(TOIE0);
#define TIMERx_OVF_vect TIMER0_OVF_vect

#elif TIMERx == 1
#define TCNTx TCNT1
#define start_count 49536
#define SET_TIMERx_PRESCALER() TCCR1B |= _BV(CS10);					// prescaler 1 if 16 MHz
#define ENABLE_TIMERx_INTERRUPT() TIMSK1 |= _BV(TOIE1);
#define TIMERx_OVF_vect TIMER1_OVF_vect

#elif TIMERx == 2
#define TCNTx TCNT2
#define start_count 0x06
#define SET_TIMERx_PRESCALER() TCCR2B |= _BV(CS22);					// prescaler 64 if 16 MHz, prescaler 32 if 8 MHz
#define ENABLE_TIMERx_INTERRUPT() TIMSK2 |= _BV(TOIE2);
#define TIMERx_OVF_vect TIMER2_OVF_vect

#endif


volatile uint32_t sys_ms = 0;

void systick_activate(void)
{
	TCNTx = 0;
	SET_TIMERx_PRESCALER();
	ENABLE_TIMERx_INTERRUPT();
}

ISR (TIMERx_OVF_vect)
{
	TCNTx = start_count;
	++sys_ms;
}


/*
	a	-	PD2, 2
	b	-	PD3, 3
	c	-	PD4, 4
	d	-	PD5, 5
	e	-	PD6, 6
	f	-	PD7, 7
	g	-	PB0, 8
	
	anode0 - PB1, 9
	anode1 - PB2, 10
*/


#define COMMON_CATHODE
//#define COMMON_ANODE


#define a _BV(PORTD2)
#define b _BV(PORTD3)
#define c _BV(PORTD4)
#define d _BV(PORTD5)
#define e _BV(PORTD6)
#define f _BV(PORTD7)
#define g _BV(PORTB0)

#define LOWEST_DIGIT _BV(PORTB2)
#define HIGHEST_DIGIT _BV(PORTB1)
#define COMMAND_BUTTON _BV(PORTB5)

#define BUTTON_CHECK() (PINB & (_BV(PINB5)))


#define out0 _BV(PORTC0)
#define out1 _BV(PORTC1)
#define out2 _BV(PORTC2)
#define out3 _BV(PORTC3)
#define out4 _BV(PORTC4)
#define out5 _BV(PORTC5)
#define out6 _BV(PORTC6)


void InitHardware(void)
{
	
	// Init the segments as outputs
	DDRD |= a | b | c | d | e | f;
	DDRB |= g | LOWEST_DIGIT | HIGHEST_DIGIT;
	
	
	// Init the Command Button with input pull up
	PORTB |= COMMAND_BUTTON;
	
	// Init the Preset outputs
	DDRC |= out0 | out1 | out2 | out3 | out4 | out5 | out6;
	PORTC |= out0 | out1 | out2 | out3 | out4 | out5 | out6;
}


void DigitGenerator(uint8_t digit)
{
#ifdef COMMON_CATHODE
	switch (digit)
	{
		case 0:
		// a b c d e f
		PORTD |= a | b | c | d | e | f;
		PORTB &= ~g;
		break;
		case 1:
		// b c
		PORTD |= b | c;
		PORTD &= ~(a | d | e | f);
		PORTB &= ~g;
		break;
		case 2:
		// a b d e g
		PORTD |= a | b | d | e;
		PORTD &= ~(c | f);
		PORTB |= g;
		break;
		case 3:
		// a b c d g
		PORTD |= a | b | c | d;
		PORTD &= ~(e | f);
		PORTB |= g;
		break;
		case 4:
		// b c f g
		PORTD |= b | c | f;
		PORTD &= ~(a | d | e);
		PORTB |= g;
		break;
		case 5:
		// a c d f g
		PORTD |= a | c | d | f;
		PORTD &= ~(b | e);
		PORTB |= g;
		break;
		case 6:
		// a c d e f g
		PORTD |= a | c | d | e | f;
		PORTD &= ~b;
		PORTB |= g;
		break;
		case 7:
		// a b c
		PORTD |= a | b | c;
		PORTD &= ~(d | e | f);
		PORTB &= ~g;
		break;
		case 8:
		// a b c d e f g
		PORTD |= a | b | c | d | e | f;
		PORTB |= g;
		break;
		case 9:
		// a b c d f g
		PORTD |= a | b | c | d | f;
		PORTD &= ~e;
		PORTB |= g;
		break;
	}
#elif defined(COMMON_ANODE)
	switch (digit)
	{
		case 0:
		// a b c d e f
		PORTD &= ~(a | b | c | d | e | f);
		PORTB |= g;
		break;
		case 1:
		// b c
		PORTD &= ~(b | c);
		PORTD |= a | d | e | f;
		PORTB |= g;
		break;
		case 2:
		// a b d e g
		PORTD &= ~(a | b | d | e);
		PORTD |= c | f;
		PORTB &= ~g;
		break;
		case 3:
		// a b c d g
		PORTD &= ~(a | b | c | d);
		PORTD |= e | f;
		PORTB &= ~g;
		break;
		case 4:
		// b c f g
		PORTD &= ~(b | c | f);
		PORTD |= a | d | e;
		PORTB &= ~g;
		break;
		case 5:
		// a c d f g
		PORTD &= ~(a | c | d | f);
		PORTD |= b | e;
		PORTB &= ~g;
		break;
		case 6:
		// a c d e f g
		PORTD &= ~(a | c | d | e | f);
		PORTD |= b;
		PORTB &= ~g;
		break;
		case 7:
		// a b c
		PORTD &= ~(a | b | c);
		PORTD |= d | e | f;
		PORTB |= g;
		break;
		case 8:
		// a b c d e f g
		PORTD &= ~(a | b | c | d | e | f);
		PORTB &= ~g;
		break;
		case 9:
		// a b c d f g
		PORTD &= ~(a | b | c | d | f);
		PORTD |= e;
		PORTB &= ~g;
		break;
	}
#endif
}

void NumberGenerator(uint8_t* modes)
{
	static uint8_t number = 0;
	static uint8_t digit = 0;
	static uint32_t update_timer = 0;
	const volatile uint32_t update_timer_timeout = 4;
	
	
	if (sys_ms - update_timer > update_timer_timeout)
	{
		update_timer = sys_ms;
		digit = !digit;
		
#ifdef COMMON_CATHODE
		PORTB |= HIGHEST_DIGIT | LOWEST_DIGIT;
		
		
		
		if (!digit)
		{
			PORTB &= ~LOWEST_DIGIT;
			number = modes[0];
		}
		else if (digit)
		{
			PORTB &= ~HIGHEST_DIGIT;
			number = modes[1];
		}
#elif defined(COMMON_ANODE)
		PORTB &= ~(HIGHEST_DIGIT | LOWEST_DIGIT);
		
		if (!digit)
		{
			PORTB |= LOWEST_DIGIT;
			number = modes[0];
		}
		else if (digit)
		{
			PORTB |= HIGHEST_DIGIT;
			number = modes[1];
		}
#endif
		
		DigitGenerator(number);
	}
	
}


typedef enum
{
	USER_ACTION_NONE = 0,
	USER_ACTION_CLICK,
	USER_ACTION_PRESS,
	} UserAction;

typedef enum
{
	BUTTON_STATE_RELEASED = 0,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_ACTUALLY_PRESSED,
	BUTTON_STATE_RELEASE,
	} ButtonState;


uint8_t ButtonHandler(void)
{
	static ButtonState BUTTON_STATE = BUTTON_STATE_RELEASED; 
	static uint32_t ButtonDebounceTimer = 0;
	static uint32_t ButtonPressedTimer = 0;
	const volatile uint32_t ButtonDebounceTimeout = 50;
	const volatile uint32_t ButtonPressedTimeout = 500;
	
	if (sys_ms - ButtonDebounceTimer > ButtonDebounceTimeout)
	{
		ButtonDebounceTimer = sys_ms;
	
		if (!BUTTON_CHECK() && (BUTTON_STATE == BUTTON_STATE_RELEASED))
		{
			BUTTON_STATE = BUTTON_STATE_PRESSED;
			ButtonPressedTimer = sys_ms;
		}
		else if (BUTTON_CHECK() && (BUTTON_STATE == BUTTON_STATE_PRESSED))
		{
			BUTTON_STATE = BUTTON_STATE_RELEASED;
			return USER_ACTION_CLICK;
		}
		else if (!BUTTON_CHECK() && (BUTTON_STATE == BUTTON_STATE_PRESSED) && (sys_ms - ButtonPressedTimer > ButtonPressedTimeout))
		{
			BUTTON_STATE = BUTTON_STATE_ACTUALLY_PRESSED;
			return USER_ACTION_PRESS;
		}
		else if (BUTTON_CHECK() && (BUTTON_STATE == BUTTON_STATE_ACTUALLY_PRESSED))
		{
			BUTTON_STATE = BUTTON_STATE_RELEASED;
		}
	}

	return USER_ACTION_NONE;
}


void ModesOuptutHandler(uint8_t* modes)
{
	switch(modes[0])
	{
		case 0:
		
		break;
		
		case 1:
		
		break;
		
		case 2:
		
		break;
		
		case 3:
		
		break;
		
		case 4:
		
		break;
		
		case 5:
		
		break;		
	}
	
	switch(modes[1])
	{
		case 1:
		
		break;
		
		case 2:
		
		break;
		
		case 3:
		
		break;
		
		case 4:
		
		break;
		
		case 5:
		
		break;
		
		case 6:
		
		break;
		
		case 7:
		
		break;
		
		case 8:
		
		break;
	}
}

int main(void)
{
    systick_activate();
	InitHardware();
	
	sei();
	

	uint8_t modes[2] = {0, 1};
    while (1)
    {
		switch (ButtonHandler()) {
			case USER_ACTION_NONE:
			break;
			case USER_ACTION_CLICK:
			if (++modes[1] > 8)
			{
				modes[1] = 1;
			}
			break;
			case USER_ACTION_PRESS:
			if (++modes[0] > 5) {
				modes[0] = 0;
			}
			break;
		}

		NumberGenerator(modes);
		ModesOuptutHandler(modes);
    }
}



#ifdef _cplusplus
}
#endif

