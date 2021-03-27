/*
 * IRCCD.c
 *
 * Created: 4/29/2018 9:05:19 PM
 * Author : amanu
 * FUSE E4 D9
 */ 


// defining the Input peripherals
#define vsrpBtn (PINB & (1<<PB6))
#define loadBtn (PIND & (1<<PD6))

#define isLoad_1_on (PINB & (1<<PB7))
#define isLoad_2_on (PIND & (1<<PD5))
#define isLoad_vf_on (PIND & (1<<PD7))

// defining the Output peripherals

// Seven Segment pin-out output
#define sseg_b0_on (PORTB |= (1<<PB4))
#define sseg_b0_off (PORTB &= ~(1<<PB4)) 
#define sseg_b1_on (PORTB |= (1<<PB5))
#define sseg_b1_off (PORTB &= ~(1<<PB5))
#define sseg_b2_on (PORTC |= (1<<PC4))
#define sseg_b2_off (PORTC &= ~(1<<PC4))
#define sseg_b3_on (PORTC |= (1<<PC3))
#define sseg_b3_off (PORTC &= ~(1<<PC3))
#define sseg_b4_on (PORTC |= (1<<PC2))
#define sseg_b4_off (PORTC &= ~(1<<PC2))
#define sseg_b5_on (PORTC |= (1<<PC0))
#define sseg_b5_off (PORTC &= ~(1<<PC0))
#define sseg_b6_on (PORTC |= (1<<PC1))
#define sseg_b6_off (PORTC &= ~(1<<PC1))
#define sseg_b7_on (PORTC |= (1<<PC5))
#define sseg_b7_off (PORTC &= ~(1<<PC5))
//////////////////////////////////////////////////////////////////////////////////////////////////////

// variable voltage load output pulse
#define triacPulseOn (PORTB |= (1<<PB0))
#define triacPulseOff (PORTB &= ~(1<<PB0))

// Load 1 output
#define load_1_on (PORTB |= (1<<PB7))
#define load_1_off (PORTB &= ~(1<<PB7))

// Load 2 output
#define load_2_on (PORTD |= (1<<PD5))
#define load_2_off (PORTD &= ~(1<<PD5))

// Load 3 output (variable voltage)
#define load_vf_on (PORTD |= (1<<PD7))
#define load_vf_off (PORTD &= ~(1<<PD7))

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Load 1 Status LED
#define led_1_on (PORTB |= (1<<PB3))
#define led_1_off (PORTB &= ~(1<<PB3))

// Load 2 Status LED
#define led_2_on (PORTB |= (1<<PB2))
#define led_2_off (PORTB &= ~(1<<PB2))

// Load 3 Status LED
#define led_3_on (PORTB |= (1<<PB1))
#define led_3_off (PORTB &= ~(1<<PB1))



#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
// #include <avr/wdt.h>
#include <util/delay.h>
#include <stdio.h>
//#include "myusart.h"
#include "buzzer.h"

#define IrDataSize 80
#define irStoreSize 6

// variables to take the ir remote data
uint16_t irRemoteDataRaw[IrDataSize];
int irdIncr = 0;
int irdRtime = 0;

// eeprom values for remote data
uint32_t EEMEM rd_load_1, rd_load_2, rd_load_vf, rd_vf_up, rd_vf_dn, rd_display;
// eeprom values for last status
uint8_t EEMEM load_1_m, load_2_m, load_vf_m, vf_level_m, display_stat_m;


// variable for display
int displayTime = 0;

// External Interrupt 0 service routine
ISR(INT0_vect)
{
	irdRtime = 0;
	irRemoteDataRaw[irdIncr] = TCNT1;
	TCNT1 = 0;
	irdIncr++;
}

// External Interrupt 1 service routine
ISR(INT1_vect)
{
	TCNT2=0x00;
	triacPulseOff;
}

// Timer1 overflow interrupt service routine
ISR(TIMER1_COMPA_vect)
{
	TCNT1 = 0;
	irdIncr = 0;
	if (irdRtime < 65535)
	irdRtime ++;
}

// Timer2 output compare interrupt service routine
ISR(TIMER2_COMP_vect)
{
	if (isLoad_vf_on) 
	{ 
		triacPulseOn;
		TCCR0=(0<<CS02) | (1<<CS01) | (1<<CS00);
	}
}


// Timer 0 overflow interrupt service routine
ISR(TIMER0_OVF_vect)
{
	triacPulseOff;
	TCCR0=(0<<CS02) | (0<<CS01) | (0<<CS00);
	TCNT0=0x83;
}

// to control the virtual port for the seven segment display
void sevenSegmentVirtualPort(uint8_t data, uint8_t dot){
	if (data & (1<<0)) sseg_b0_on; else sseg_b0_off;
	if (data & (1<<1)) sseg_b1_on; else sseg_b1_off;
	if (data & (1<<2)) sseg_b2_on; else sseg_b2_off;
	if (data & (1<<3)) sseg_b3_on; else sseg_b3_off;
	if (data & (1<<4)) sseg_b4_on; else sseg_b4_off;
	if (data & (1<<5)) sseg_b5_on; else sseg_b5_off;
	if (data & (1<<6)) sseg_b6_on; else sseg_b6_off;
	if (dot) sseg_b7_on; else sseg_b7_off;
}

// function to control seven segment display
// data limit -2 to 15
// 0 to 15 is to show the hex data 0 to F
// -1 is for dp only
// -2 is for dark display
void writeToSevenSegment(int data){
	
	if (data > 15)
	{
		data = 15;
	} else if (data < -2)
	{
		data = 0;
	}
	
	uint8_t sevSeg[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x27, 0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71};
	
	if (data>-1)
	{
		sevenSegmentVirtualPort(sevSeg[data], 0);
	} else if (data == -1)
	{
		sevenSegmentVirtualPort(0x00, 1);
	} else {
		sevenSegmentVirtualPort(0x00, 0);
	}
}

// function to get the irRemote data decoded
uint32_t getIrRemoteData() {
	uint32_t irRemoteDataHash = 0;
	
	if (irdRtime>0)
	{
		// To detect Sony
		if (irRemoteDataRaw[1] > 280 && irRemoteDataRaw[1] < 340)
		{
			for (int i=2; i<25; i+=2) {
				if (irRemoteDataRaw[i+1]>110)
				{
					irRemoteDataHash = irRemoteDataHash << 1;
					irRemoteDataHash += 1;
				} else {
					irRemoteDataHash = irRemoteDataHash << 1;
				}
			}
		}
		
		// To detect Samsung
		else if (irRemoteDataRaw[1] > 530 && irRemoteDataRaw[1] < 600)
		{
			for (int i=4; i<70; i+=2) {
				if (irRemoteDataRaw[i]>110)
				{
					irRemoteDataHash = irRemoteDataHash << 1;
					irRemoteDataHash += 1;
					} else {
					irRemoteDataHash = irRemoteDataHash << 1;
				}
			}
		}
		
		// To detect TCL
		else if (irRemoteDataRaw[1] > 430 && irRemoteDataRaw[1] < 530)
		{
			for (int i=4; i<70; i+=2) {
				if (irRemoteDataRaw[i]>110)
				{
					irRemoteDataHash = irRemoteDataHash << 1;
					irRemoteDataHash += 1;
					} else {
					irRemoteDataHash = irRemoteDataHash << 1;
				}
			}
		}
		
		// to detect myOne
		else if (irRemoteDataRaw[1] > 1020 && irRemoteDataRaw[1] < 1190)
		{
			for (int i=4; i<70; i+=2) {
				if (irRemoteDataRaw[i]>110)
				{
					irRemoteDataHash = irRemoteDataHash << 1;
					irRemoteDataHash += 1;
					} else {
					irRemoteDataHash = irRemoteDataHash << 1;
				}
			}
		}
		
		// to cancel the previous ir data array
		for (int i=0; i<IrDataSize; i++)
		{
			irRemoteDataRaw[i] = 0;
		}
	}
	
	// returning the decoded value of ir data
	return irRemoteDataHash;
}

// Set Duty cycle for triac
void setDutyLevel(int duty)
{
	switch(duty){
		case 0:
			OCR2 = 55;
			break;
		case 1:
			OCR2 = 51;
			break;
		case 2:
			OCR2 = 47;
			break;
		case 3:
			OCR2 = 43;
			break;
		case 4:
			OCR2 = 39;
			break;
		case 5:
			OCR2 = 34;
			break;
		case 6:
			OCR2 = 29;
			break;
		case 7:
			OCR2 = 24;
			break;
		case 8:
			OCR2 = 19;
			break;
		case 9:
			OCR2 = 1;
			break;
		default:
			OCR2 = 40;
	}
}

void updateDisplay(){
	if (eeprom_read_byte(&display_stat_m) || displayTime>1)
	{
		writeToSevenSegment(eeprom_read_byte(&vf_level_m));
		if (eeprom_read_byte(&load_1_m))
		led_1_on;
		else
		led_1_off;
		if (eeprom_read_byte(&load_2_m))
		led_2_on;
		else
		led_2_off;
		if (eeprom_read_byte(&load_vf_m))
		led_3_on;
		else
		led_3_off;
	} else {
		writeToSevenSegment(-2);
		led_1_off;
		led_2_off;
		led_3_off;
	}
}

// function to execute the load button operation
void executeLoadBtnOperation(int loadBtnValue) {
	if (loadBtnValue == 1)
	{
		if (isLoad_1_on)
		{
			load_1_off;
			eeprom_update_byte(&load_1_m, 0x00);
			buzzer_loadOff();
		} else {
			load_1_on;
			eeprom_update_byte(&load_1_m, 0xFF);
			buzzer_loadOn();
		}
	} else if (loadBtnValue == 2)
	{
		if (isLoad_2_on)
		{
			load_2_off;
			eeprom_update_byte(&load_2_m, 0x00);
			buzzer_loadOff();
		} else {
			load_2_on;
			eeprom_update_byte(&load_2_m, 0xFF);
			buzzer_loadOn();
		}
	} else if (loadBtnValue == 3)
	{
		if (isLoad_vf_on)
		{
			load_vf_off;
			eeprom_update_byte(&load_vf_m, 0x00);
			buzzer_loadOff();
		} else {
			load_vf_on;
			eeprom_update_byte(&load_vf_m, 0xFF);
			buzzer_loadOn();
		}
	} else if (loadBtnValue == 4)
	{
		if (eeprom_read_byte(&display_stat_m))
		eeprom_update_byte(&display_stat_m, 0);
		else
		eeprom_update_byte(&display_stat_m, 255);
		updateDisplay();
		buzzer_success();
	}
	if (loadBtnValue == 4) displayTime = 0;
	else displayTime = 3000;
	updateDisplay();
}

// function to execute variable load button action
void executeVrspBtnOperation(uint8_t dir) {
	int lastVariableData = (int)eeprom_read_byte(&vf_level_m);
	if (dir == 0)
	{
		lastVariableData++;
		buzzer_loadOn();
	} else {
		lastVariableData--;
		buzzer_loadOff();
	}
	
	if (lastVariableData > 9)
	{
		lastVariableData = 0;
	} else if (lastVariableData < 0)
	{
		lastVariableData = 9;
	}
	eeprom_write_byte(&vf_level_m, lastVariableData);
	setDutyLevel(lastVariableData);
	displayTime = 3000;
	updateDisplay();
}

// function to execute the remote operation
void executeIrRemoteOperation (uint32_t irRemoteDataHash) {
	
	if (eeprom_read_dword(&rd_load_1) == irRemoteDataHash)
	{
		executeLoadBtnOperation(1);
	}
	else if (eeprom_read_dword(&rd_load_2) == irRemoteDataHash)
	{
		executeLoadBtnOperation(2);
	}
	else if (eeprom_read_dword(&rd_load_vf) == irRemoteDataHash)
	{
		executeLoadBtnOperation(3);
	}
	else if (eeprom_read_dword(&rd_vf_up) == irRemoteDataHash)
	{
		executeVrspBtnOperation(0);
	}
	else if (eeprom_read_dword(&rd_vf_dn) == irRemoteDataHash)
	{
		executeVrspBtnOperation(1);
	}
	else if (eeprom_read_dword(&rd_display) == irRemoteDataHash)
	{
		executeLoadBtnOperation(4);
	}
	else {
		buzzer_gotIrData();
	}
}

void executeIrRemoteCalibration(uint32_t irRemoteDatahash, uint8_t dataCount) {
	if (dataCount == 1)
	{
		eeprom_update_dword(&rd_load_1, irRemoteDatahash);
	}
	else if (dataCount == 2)
	{
		eeprom_update_dword(&rd_load_2, irRemoteDatahash);
	}
	else if (dataCount == 3)
	{
		eeprom_update_dword(&rd_load_vf, irRemoteDatahash);
	}
	else if (dataCount == 4)
	{
		eeprom_update_dword(&rd_vf_up, irRemoteDatahash);
	}
	else if (dataCount == 5)
	{
		eeprom_update_dword(&rd_vf_dn, irRemoteDatahash);
	}
	else if (dataCount == 6)
	{
		eeprom_update_dword(&rd_display, irRemoteDatahash);
	}
	buzzer_success();
}

void restoreLastStatus() {
	setDutyLevel(eeprom_read_byte(&vf_level_m));
	if (eeprom_read_byte(&load_1_m)) load_1_on;
	else load_1_off;
	if (eeprom_read_byte(&load_2_m)) load_2_on;
	else load_2_off;
	if (eeprom_read_byte(&load_vf_m)) load_vf_on;
	else load_vf_off;
	displayTime = 3000;
	updateDisplay();
}



int main(void)
{
	
	//usart_init(19200);
	
	
	// Input/Output Ports initialization
	// Port B initialization
	DDRB=(1<<DDB7) | (0<<DDB6) | (1<<DDB5) | (1<<DDB4) | (1<<DDB3) | (1<<DDB2) | (1<<DDB1) | (1<<DDB0);
	PORTB=(0<<PB7) | (1<<PB6) | (0<<PB5) | (0<<PB4) | (0<<PB3) | (0<<PB2) | (0<<PB1) | (0<<PB0);

	// Port C initialization
	DDRC=(0<<DDC6) | (1<<DDC5) | (1<<DDC4) | (1<<DDC3) | (1<<DDC2) | (1<<DDC1) | (1<<DDC0);
	PORTC=(0<<PC6) | (0<<PC5) | (0<<PC4) | (0<<PC3) | (0<<PC2) | (0<<PC1) | (0<<PC0);

	// Port D initialization
	DDRD=(1<<DDD7) | (0<<DDD6) | (1<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
	PORTD=(0<<PD7) | (1<<PD6) | (0<<PD5) | (0<<PD4) | (1<<PD3) | (1<<PD2) | (0<<PD1) | (0<<PD0);


	// Timer/Counter 0 initialization
	// Clock source: System Clock
	// Clock value: 125.000 kHz
	// TCCR0=(0<<CS02) | (1<<CS01) | (1<<CS00);
	TCCR0=(0<<CS02) | (0<<CS01) | (0<<CS00);
	TCNT0=0x83;
	
    
	// Timer/Counter 1 initialization
	// Clock source: System Clock
	// Clock value: 125.000 kHz
	// Mode: Normal top=0xFFFF
	// OC1A output: Disconnected
	// OC1B output: Disconnected
	// Noise Canceler: Off
	// Input Capture on Falling Edge
	// Timer Period: 0.52429 s
	// Timer1 Overflow Interrupt: On
	// Input Capture Interrupt: Off
	// Compare A Match Interrupt: Off
	// Compare B Match Interrupt: Off
	TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
	TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (1<<CS11) | (1<<CS10);
	TCNT1H=0x00;
	TCNT1L=0x00;
	ICR1H=0x00;
	ICR1L=0x00;
	OCR1AH=0x00;
	//OCR1AL=0x00;
	//OCR1BH=0x00;
	OCR1A = 1200;
	OCR1BL=0x00;

	// Timer/Counter 2 initialization
	// Clock source: System Clock
	// Clock value: 7.813 kHz
	// Mode: Normal top=0xFF
	// OC2 output: Disconnected
	// Timer Period: 32.768 ms
	ASSR=0<<AS2;
	TCCR2=(0<<WGM20) | (0<<COM21) | (0<<COM20) | (0<<WGM21) | (1<<CS22) | (1<<CS21) | (1<<CS20);
	TCNT2=0x00;
	OCR2=35;

	// Timer(s)/Counter(s) Interrupt(s) initialization
	TIMSK=(1<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (1<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (1<<TOIE0);

	// External Interrupt(s) initialization
	// INT0: On
	// INT0 Mode: Any change
	// INT1: On
	// INT1 Mode: Any change
	GICR|=(1<<INT1) | (1<<INT0);
	MCUCR=(0<<ISC11) | (1<<ISC10) | (0<<ISC01) | (1<<ISC00);
	GIFR=(1<<INTF1) | (1<<INTF0);

	// Global enable interrupts
	sei();
	
	// variables for ir remote action handling
	uint32_t irRemoteDataHash = 0;
	uint32_t irRemoteDataHashPrev = 0;
	uint16_t irRemoteDataExecFilterTime = 0;
	uint8_t irRemoteSecondPressSence = 0;
	
	// variables for button operation
	uint16_t loadBtnMultipressTime = 0;
	uint16_t loadBtnValue = 0;
	
	// variables for remote calibration loop
	uint8_t dataCount = 0;
	uint8_t isIrDataUnique = 0;
	uint16_t loopConunt = 0;
	uint8_t blinkDisplay = 0;
	uint32_t irDataVerifHolder[irStoreSize];

mainLoop:
	// to set the U sign after come to this loop
	sevenSegmentVirtualPort(0x63, 0);
	// Beep the buzzer
	buzzer_changingLoop();

	// stay wait here until button release
	_delay_ms(50);
	while(!vsrpBtn);
	_delay_ms(50);
	
	restoreLastStatus();

    while (1) 
    {
		// temporary display allow
		if (displayTime>0)
		{
			displayTime--;
			if (displayTime == 1)
			updateDisplay();
		}
		
		// Button operation execution /////////////////////////////////////////////////////////////////////
		// button multi press time control
		if (loadBtnMultipressTime > 0) loadBtnMultipressTime--;
		
		if (!loadBtn)
		{
			buzzer_button_dn();			
			loadBtnValue++;
			while(!loadBtn);
			_delay_ms(50);
			loadBtnMultipressTime = 300;
		}
		
		// to execute the button operation
		if (loadBtnMultipressTime == 0 && loadBtnValue > 0)
		{
			executeLoadBtnOperation(loadBtnValue);
			loadBtnValue = 0;
		}
		
		if (!vsrpBtn)
		{
			buzzer_button_dn();
			for (int i=0; !vsrpBtn ; i++)
			{
				_delay_ms(1);
				if (i>2000)
				{
					goto remoteCalibLoop;
				}
			}
			executeVrspBtnOperation(0);			
		}
		
		// ir remote operation execution filter /////////////////////////////////////////////////////////
		
		// to get the ir remote data in decoded format from the ir remote buffer memory
		irRemoteDataHash = getIrRemoteData();
		
		// operation execution time filter for continuous remote press for same button
		if (irRemoteDataExecFilterTime < 65535) irRemoteDataExecFilterTime++;
		// execution timer time filter for continuous separate press for same button
		if (irdRtime > 10) irRemoteSecondPressSence = 1;
		
		// checking the availability of decoded ir remote data 
		if (irRemoteDataHash > 0)
		{
			// to execute different button press
			if (irRemoteDataHashPrev != irRemoteDataHash) {
				irRemoteDataHashPrev = irRemoteDataHash;
				irRemoteDataExecFilterTime = 0;
				irRemoteSecondPressSence = 0;
				executeIrRemoteOperation(irRemoteDataHash);
			} 
			// to execute same button quick press
			else if (irRemoteSecondPressSence) {
				irRemoteDataExecFilterTime = 0;
				irRemoteSecondPressSence = 0;
				executeIrRemoteOperation(irRemoteDataHash);
			} 
			// to execute same button continuous press
			else if (irRemoteDataExecFilterTime>900) {
				irRemoteDataExecFilterTime = 0;
				executeIrRemoteOperation(irRemoteDataHash);
			}
		}
		
		// delay to control the loop timing
		_delay_ms(1);
    }
	
remoteCalibLoop:
	// to set the U sign after come to this loop
	sevenSegmentVirtualPort(0x5C, 0);
	
	buzzer_changingLoop();
	
	// stay wait here until button release
	_delay_ms(50);
	while(!vsrpBtn);
	_delay_ms(75);
	// variables for the remote calibration operation
	dataCount = 0;
	isIrDataUnique = 0;
	loopConunt = 501;
	for (int i=0; i<irStoreSize; i++)
	{
		irDataVerifHolder[i] = 0;
	}
	while(1){
		
		if (!vsrpBtn)
		{
			buzzer_button_dn();
			
			_delay_ms(50);
			//while(!vsrpBtn);
			goto mainLoop;
		}
		// to blink the seven segment display with showing the current data pointer number
		if (loopConunt>500)
		{
			loopConunt = 0;
			if (blinkDisplay)
			{
				blinkDisplay = 0;
				writeToSevenSegment(dataCount);
			} else {
				blinkDisplay = 1;
				writeToSevenSegment(-1);
			}

		} else loopConunt++;
		
		// ir remote operation execution filter /////////////////////////////////////////////////////////
		
		// to get the ir remote data in decoded format from the ir remote buffer memory
		irRemoteDataHash = getIrRemoteData();
		
		// checking the availability of decoded ir remote data
		if (irRemoteDataHash > 0)
		{
			// to execute different ir remote button press
			if (irRemoteDataHashPrev != irRemoteDataHash) {
				irRemoteDataHashPrev = irRemoteDataHash;
				
				isIrDataUnique = 1;
				for (int i=0; i<irStoreSize; i++)
				{
					if (irDataVerifHolder[i] == irRemoteDataHash)
					{
						isIrDataUnique = 0;
					}
				}
				
				if (isIrDataUnique)
				{
					irDataVerifHolder[dataCount] = irRemoteDataHash;
					executeIrRemoteCalibration(irRemoteDataHash, ++dataCount);
					if (dataCount >= irStoreSize)
					{
						while(irdRtime < 10) _delay_ms(1);
						for (int i=0; i<IrDataSize; i++)
						{
							irRemoteDataRaw[i] = 0;
						}
						goto mainLoop;
					}
				}
			}
		}
		
		// delay to control the loop timing
		_delay_ms(1);
	}
}

