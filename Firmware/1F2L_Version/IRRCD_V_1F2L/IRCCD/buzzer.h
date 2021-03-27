/*
 * buzzer.h
 *
 * Created: 5/1/2018 2:16:05 PM
 *  Author: amanu
 */ 

// defining buzzer output peripheral
#define buzzer_on (PORTC |= (1<<PC5))
#define buzzer_off (PORTC &= ~(1<<PC5))

#ifndef BUZZER_H_
#define BUZZER_H_


void buzzer_gotIrData() {
	buzzer_on;
	_delay_ms(30);
	buzzer_off;
}

void buzzer_success(){
	buzzer_on;
	_delay_ms(30);
	buzzer_off;
	_delay_ms(20);
	buzzer_on;
	_delay_ms(30);
	buzzer_off;
}

void buzzer_loadOn() {
	buzzer_on;
	_delay_ms(20);
	buzzer_off;
	_delay_ms(10);
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
}

void buzzer_loadOff() {
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
	_delay_ms(10);
	buzzer_on;
	_delay_ms(20);
	buzzer_off;
}

void buzzer_error() {
	buzzer_on;
	_delay_ms(100);
	buzzer_off;
	_delay_ms(50);
	buzzer_on;
	_delay_ms(100);
	buzzer_off;
	_delay_ms(50);
	buzzer_on;
	_delay_ms(100);
	buzzer_off;
}

void buzzer_startup() {
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
	_delay_ms(30);
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
	_delay_ms(30);
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
	_delay_ms(30);
	buzzer_on;
	_delay_ms(80);
	buzzer_off;
}

void buzzer_changingLoop() {
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
	_delay_ms(30);
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
	_delay_ms(30);
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
	_delay_ms(30);
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
	_delay_ms(30);
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
	_delay_ms(30);
	buzzer_on;
	_delay_ms(80);
	buzzer_off;
}

void buzzer_button_dn() {
	buzzer_on;
	_delay_ms(50);
	buzzer_off;
}

void buzzer_button_up() {
	buzzer_on;
	_delay_ms(30);
	buzzer_off;
}


#endif /* BUZZER_H_ */