
// TearDrop
// Pump Controller
// Version 99.0 
// ATmega328P
// Created by Nadav Matalon
// 24 August 2014

// (TDCR_IC_PUMP_99)


// PUMP CONTROLLER (COM5)

// PIN_D2  (PIN 4)   -  ONE_WIRE
// PIN_D3  (PIN 5)   -  FREQ_VCC (TIMER2)
// PIN_D4  (PIN 6)   -  ALARM_OUT
// PIN_D5  (PIN 11)  -  SYSTEM_MODE (VCC/PWM)
// PIN_D6  (PIN 12)  -  SYSTEM_CONTROL_1 (OVD_OFF/OVD_ON)
// PIN_D7  (PIN 13)  -  SYSTEM_CONTROL_2 (MAN/AUTO)
// PIN_D8  (PIN 14)  -  SYSTEM_CONTROL_LED
// PIN_D9  (PIN 15)  -  PUMP_RPM_1
// PIN_D10 (PIN 16)  -  FREQ_PWM (TIMER1)
// PIN_D11 (PIN 17)  -  PUMP_RPM_2
// PIN_D12 (PIN 18)  -  PUMP_FLOW
// PIN_D13 (PIN 19)  -  PSU_RPM
// PIN_A0  (PIN 23)  -  TEMP_AN1
// PIN_A1  (PIN 24)  -  TEMP_AN2
// PIN_A2  (PIN 25)  -  PUMP_CONTROL (RAW_POT)
// PIN_A3  (PIN 26)  -  PUMP_VCC
// PIN_A4  (PIN 27)  -  I2C_SDA
// PIN_A5  (PIN 28)  -  I2C_SCL

// http://playground.arduino.cc/Main/PinChangeInt
// http://code.google.com/p/arduino-pinchangeint/
#include <PinChangeInt.h>

// http://playground.arduino.cc/Learning/OneWire
// https://github.com/ntruchsess/arduino-OneWire/tree/master
#include <OneWire.h>

// https://github.com/steamfire/WSWireLib
#include <WSWire.h>



const boolean DEBUG_MODE = false;		// DEBUG_MODE
// const boolean DEBUG_MODE = true;		// DEBUG_MODE

// const boolean PRINT_FIRST_ROW = false;	// DEBUG_MODE
const boolean PRINT_FIRST_ROW = true;		// DEBUG_MODE

// const boolean PRINT_SECOND_ROW = false;	// DEBUG_MODE
const boolean PRINT_SECOND_ROW = true;		// DEBUG_MODE

const boolean PSU_ALARM = false;		// PSU_ALARM_OFF
// const boolean PSU_ALARM = true;		// PSU_ALARM_ON


const byte PIN_D2 = 2;				// ONE_WIRE
const byte PIN_D3 = 3;				// FREQ_VCC (TIMER2)
const byte PIN_D4 = 4;				// ALARM_OUT
const byte PIN_D5 = 5;				// SYSTEM_MODE (VCC/PWM)
const byte PIN_D6 = 6;				// SYSTEM_CONTROL_1 (OVD_ON/OVD_OFF)
const byte PIN_D7 = 7;				// SYSTEM_CONTROL_2 (MAN/AUTO)
const byte PIN_D8 = 8;				// SYSTEM_CONTROL_LED
const byte PIN_D9 = 9;				// PUMP_RPM_1
const byte PIN_D10 = 10;			// FREQ_PWM (TIMER1)
const byte PIN_D11 = 11;			// PUMP_RPM_2
const byte PIN_D12 = 12;			// FLOW_LPM
const byte PIN_D13 = 13;			// PSU_RPM
const byte PIN_A0 = A0;				// TEMP_AN1
const byte PIN_A1 = A1;				// TEMP_AN2
const byte PIN_A2 = A2;				// PUMP_CONTROL (RAW_POT)
const byte PIN_A3 = A3;				// PUMP_VCC

const byte ADC_A0 = 0;									// ADC input channel PIN_A0
const byte ADC_A1 = 1;									// ADC input channel PIN_A1
const byte ADC_A2 = 2;									// ADC input channel PIN_A2
const byte ADC_A3 = 3;									// ADC input channel PIN_A3
const byte ADC_A4 = 4;									// ADC input channel PIN_A4
const byte ADC_A5 = 5;									// ADC input channel PIN_A5
const byte ADC_TEMP = 8;								// ADC input channel IC_TEMP
const byte ADC_VCC = 14;								// ADC input channel IC_VCC
const byte ADC_GND = 15;								// ADC input channel IC_GND

const unsigned int RPM_THRESHOLD = 500;							// defines Alarm activation lower threshold for RPM counters (in RPM)
const unsigned int FLOW_THRESHOLD = 400;			        		// defines Alarm activation lower threshold for FLOW Sensor (in LPM)
const unsigned int AN_TEMP_THRESHOLD = 500;						// defines Alarm activation higher threshold for Analog Temp sensors (in Celsuis)
const unsigned int IC_TEMP_THRESHOLD = 500;						// defines Alarm activation higher threshold for IC (ATmega328p) Internal sensor (in Celsuis)

const float TEMP_OFFSET_A0 = 1.00;							// TEMP_AN1 - calibration function: higher OFFSET precentage => lower temperature readings
const float TEMP_OFFSET_A1 = 1.00;							// TEMP_AN2 - calibration function: higher OFFSET precentage => lower temperature readings

const unsigned int TEMP_RESISTOR_A0 = 10000;	// TEMP_AN1_RESISTOR			// TEMP_AN1 - resistor value on PIN_A0 (in Ohms)
const unsigned int TEMP_RESISTOR_A1 = 10000;	// TEMP_AN2_RESISTOR			// TEMP_AN2 - resistor value on PIN_A1 (in Ohms)

const unsigned int FLOW_DIAMETER_OFFSET = 307;						// set for 10mm tube diameter 

OneWire wireBus(PIN_D2);  						        	// sets up One-Wire bus on PIN_D2

const unsigned int PC_SYSTEM_STATUS = 1001;	// SYSTEM_STATUS (OK/ALARM) 		// PC_SYSTEM_STATUS processing update command
const unsigned int PC_SYSTEM_CONTROL_1 = 1002;	// SYSTEM_CONTROL_1(OVD_OFF/OVD_ON)	// PC_SYSTEM_CONTROL_1 processing update command
const unsigned int PC_SYSTEM_CONTROL_2 = 1003;	// SYSTEM_CONTROL_2(MAN/AUTO)		// PC_SYSTEM_CONTROL_2 processing update command
const unsigned int PC_VCC_PUMP = 1004;		// VCC_PUMP				// PC_VCC_PUM processing update command
const unsigned int PC_VCC_ICP = 1005;		// VCC_ICP				// PC_VCC_ICP processing update command
const unsigned int PC_SYSTEM_MODE = 1006;	// SYSTEM_MODE (VCC/PWM)		// PC_SYSTEM_MODE processing update command
const unsigned int PC_PUMP_RPM_1 = 1007;	// PUMP_RPM_1				// PC_PUMP_RPM_1 processing update command
const unsigned int PC_PUMP_RPM_2 = 1008;	// PUMP_RPM_2				// PC_PUMP_RPM_2 processing update command
const unsigned int PC_PSU_RPM = 1009;		// PSU_RPM				// PC_PSU_RPM processing update command
const unsigned int PC_PUMP_FLOW = 1010;		// PUMP_FLOW				// PC_PUMP_FLOW processing update command
const unsigned int PC_DUTY_CYCLE_PER = 1011;	// PUMP_DUTY_CYCLE_PER			// PC_DUTY_CYCLE_PER processing update command
const unsigned int PC_TEMP_AN1 = 1012;		// TEMP_AN1				// PC_TEMP_AN1 processing update command
const unsigned int PC_TEMP_AN2 = 1013;		// TEMP_AN2 				// PC_TEMP_AN2 processing update command
const unsigned int PC_TEMP_DGP = 1014;		// TEMP_DGP (TEMP_OW_1)			// PC_TEMP_DGP processing update command
const unsigned int PC_TEMP_ICP = 1015;		// TEMP_ICP 				// PC_TEMP_ICP processing update command
const unsigned int PC_FREQ_VCC = 1016;		// FREQ_VCC				// PC_FREQ_VCC processing update command
const unsigned int PC_FREQ_PWM = 1017;		// FREQ_PWM				// PC_FREQ_PWM processing update command
const unsigned int PC_RAM_ICP = 1018;		// RAM_ICP 				// PC_RAM_ICP processing update command

const unsigned int PC_TEMP_OW_1 = 1019;		// TEMP_OW_2				// PC_TEMP_OW_1 processing update command
const unsigned int PC_TEMP_OW_2 = 1020;		// TEMP_OW_3				// PC_TEMP_OW_2 processing update command

const unsigned int PC_TEMP_I2C_1 = 1021;	// TEMP_I2C_1				// PC_TEMP_I2C_1 processing update command
const unsigned int PC_TEMP_I2C_2 = 1022;	// TEMP_I2C_2				// PC_TEMP_I2C_2 processing update command

const unsigned int FC_SYSTEM_CONTROL_1 = 2001;	// SYSTEM_CONTROL_1 (OVD_OFF/OVD_ON)	// FC_SYSTEM_CONTROL_1 processing update command
const unsigned int FC_SYSTEM_CONTROL_2 = 2002;	// SYSTEM_CONTROL_2 (MAN/AUTO)		// FC_SYSTEM_CONTROL_2 processing update command
const unsigned int FC_FAN_VCC_1 = 2003;		// FAN_VCC_1				// FC_FAN_VCC_1 processing update command
const unsigned int FC_FAN_VCC_2 = 2004;		// FAN_VCC_2				// FC_FAN_VCC_2 processing update command
const unsigned int FC_VCC_ICF = 2005;		// VCC_ICF (BOTTOM)			// FC_VCC_IC processing update command
const unsigned int FC_FAN_MODE_1 = 2006;	// FAN_MODE_1 (VCC_1/PWM_1)		// FC_FAN_MODE_1 processing update command
const unsigned int FC_FAN_RPM_1 = 2007;		// FAN_RPM_1				// FC_FAN_RPM_1 processing update command
const unsigned int FC_FAN_RPM_2 = 2008;		// FAN_RPM_2				// FC_FAN_RPM_2 processing update command
const unsigned int FC_FAN_RPM_3 = 2009;		// FAN_RPM_3				// FC_FAN_RPM_3 processing update command
const unsigned int FC_FAN_RPM_4 = 2010;		// FAN_RPM_4				// FC_FAN_RPM_4 processing update command
const unsigned int FC_DUTY_CYCLE_PER_1 = 2011;	// DUTY_CYCLE_PER_1			// FC_DUTY_CYCLE_PER_1 processing update command
const unsigned int FC_FAN_MODE_2 = 2012;	// FAN_MODE_2 (VCC_2/PWM_2)		// FC_FAN_MODE_2 processing update command
const unsigned int FC_FAN_RPM_5 = 2013;		// FAN_RPM_5				// FC_FAN_RPM_5 processing update command
const unsigned int FC_FAN_RPM_6 = 2014;		// FAN_RPM_6				// FC_FAN_RPM_6 processing update command
const unsigned int FC_DUTY_CYCLE_PER_2 = 2015;	// DUTY_CYCLE_PER_2			// FC_DUTY_CYCLE_PER_2 processing update command
const unsigned int FC_TEMP_AN1 = 2016;		// TEMP_AN1				// FC_TEMP_AN3 processing update command
const unsigned int FC_TEMP_AN2 = 2017;		// TEMP_AN2				// FC_TEMP_AN4 processing update command
const unsigned int FC_TEMP_DGF = 2018;		// TEMP_DGF (TEMP_OW_1)			// FC_TEMP_DG processing update command
const unsigned int FC_TEMP_ICF = 2019;		// TEMP_ICF (TOP)			// FC_TEMP_IC processing update command
const unsigned int FC_FREQ_VCC_1 = 2020;	// FREQ_VCC_1				// FC_FREQ_VCC_1 processing update command
const unsigned int FC_FREQ_PWM_1 = 2021;	// FREQ_PWM_1				// FC_FREQ_PWM_1 processing update command
const unsigned int FC_FREQ_VCC_2 = 2022;	// FREQ_VCC_2				// FC_FREQ_VCC_2 processing update command
const unsigned int FC_FREQ_PWM_2 = 2023;	// FREQ_PWM_2				// FC_FREQ_PWM_2 processing update command
const unsigned int FC_RAM_ICF = 2024;		// RAM_ICF (BOTTOM)			// FC_RAM_ICF processing update command

const unsigned int FC_TEMP_OW_3 = 2025;		// TEMP_OW_3				// FC_TEMP_OW_3 processing update command
const unsigned int FC_TEMP_OW_4 = 2026;		// TEMP_OW_4				// FC_TEMP_OW_4 processing update command

const unsigned int FC_TEMP_I2C_3 = 2027;	// TEMP_I2C_3				// FC_TEMP_I2C_3 processing update command
const unsigned int FC_TEMP_I2C_4 = 2028;	// TEMP_I2C_4				// FC_TEMP_I2C_4 processing update command

const unsigned int FC_PLACE_HOLDER = 2029;	// FC_PLACE_HOLDER			// FC_PLACE_HOLDER processing update command


// const byte M2_ADDRESS = 40;			// FAN_CONTROLLER_IC2			// M2 I2C device address

const byte M1_ADDRESS = 50;			// FAN_CONTROLLER_IC1			// M1 I2C device address

const byte PC_ADDRESS = 60;			// PC_I2C_ADDRESS			// PC I2C device address

const byte PC_PING = 100;

const byte PC_DATA_IN_COUNT = 30;

const byte PC_PACKAGE_1 = 245;
const byte PC_PACKAGE_2 = 246;
const byte PC_PACKAGE_3 = 247;
const byte PC_PACKAGE_4 = 248;

const byte MAX_OW_DEVICES = 3;           						// determines maximum number of One-Wire devices on bus
const byte OW_DEVICE_1 = 0;								// One-Wire device 1 location in device address arrey
const byte OW_DEVICE_2 = 1;								// One-Wire device 2 location in device address arrey
const byte OW_DEVICE_3 = 2;								// One-Wire device 3 location in device address arrey

const byte MAX_I2C_TEMP_DEVICES = 2;             					// sets maximum number of digital temperature sensors on I2C bus
const byte I2C_DEVICE_1 = 0;								// I2C_device_1 location in I2C_address arrey
const byte I2C_DEVICE_2 = 1;								// I2C_device_2 location in I2C_address arrey

volatile boolean got_data = false;
volatile byte data_in[PC_DATA_IN_COUNT] = {0};

volatile unsigned long currentBurp_D9 = 0;						// stores rpm count for ISR on PIN_D9
volatile unsigned long currentBurp_D11 = 0;						// stores rpm count for ISR on PIN_D11
volatile unsigned long currentBurp_D12 = 0;						// stores rpm count for ISR routine on PIN_D12
volatile unsigned long currentBurp_D13 = 0;						// stores rpm count for ISR on PIN_D13

volatile unsigned long ovf1 = 0;		// TIMER1_OVERFLOW			// stores TIMER1 overflow count
volatile unsigned long ovf2 = 0;		// TIMER2_OVERFLOW			// stores TIMER2 overflow count

volatile unsigned int current_duty_cycle;						// stores current duty_cycle value of FAN_CHANNEL_1

unsigned int freq_VCC = 0;			// FREQ_VCC				// stores freq_VCC (TIMER2) frequency reading
unsigned int freq_PWM = 0;			// FREQ_PWM				// stores freq_PWM (TIMER1) frequency reading

unsigned int ocr1b_data = 638;
unsigned int ocr2b_data = 78;

boolean system_status = false;			// SYSTEM_STATUS (OK/ALARM)		// SYSTEM_STATUS (0 => OK / 1 => ALARM)

boolean system_control_1 = true;		// OVD_OFF/OVD_ON			// stores state of swtich on PIN_D6 (OVD_ON/OVD_OFF)

boolean system_control_2 = true;		// MAN/AUTO				// stores state of swtich on PIN_D7 (MAN/AUTO)

boolean system_mode = true;			// VCC/PWM				// stores state of swtich on PIN_D5 (VCC/PWM)

unsigned int pump_control = 880;		// PUMP_CONTROL (RAW_POT)		// PUMP_CONTROL - raw pot reading for duty cycle calculation

unsigned int duty_cycle_per = 100;		// DUTY_CYCLE_PER

unsigned int vcc_pump = 0;			// VCC_PUMP				// stores VCC reading of PUMP1 supply

unsigned int pump_rpm_1 = 0;			// PUMP_RPM_1				// stores rpm reading on PIN_D9
unsigned int pump_rpm_2 = 0;			// PUMP_RPM_2				// stores rpm reading on PIN_D11
unsigned int psu_rpm = 0;			// PSU_RPM				// stores rpm reading on PIN_D13

unsigned int pump_flow = 0;			// PUMP_FLOW				// stores current flow rate (in LPM) on PIN_D12 (multiplied by 100)							

unsigned int temp_AN1 = 0;			// TEMP_AN1				// stores TEMP_AN1 readings (PIN_A0) 
unsigned int temp_AN2 = 0;			// TEMP_AN2				// stores TEMP_AN2 readings (PIN_A1)

unsigned int top_temp_AN = 550;			// TOP_TEMP_AN				// stores the current top temperature between temp_AN1 & temp_AN_2

unsigned int vcc_IC = 0;			// VCC_IC				// stores VCC reading of IC supply
unsigned int temp_IC = 0;			// TEMP_IC				// stores temperature reading of IC (alarm watch)
unsigned int ram_IC = 0;			// RAM_IC				// stores IC free ram 

boolean serial_open = false;								// stores state of serial port

byte wire_address [MAX_OW_DEVICES] [8] = {0};           				// sets up array of arrays for storing 8-byte address of each One-Wire device
byte total_OW_devices = 0;    	 							// stores current number of devices discovered on M2 One-Wire bus
boolean temp_OW_multiple = false;
unsigned int temp_OW_1 = 0;			// TEMP_OW_1				// stores temperature reading of OW_Device_1
unsigned int temp_OW_2 = 0;			// TEMP_OW_2				// stores temperature reading of OW_Device_2
unsigned int temp_OW_3 = 0;			// TEMP_OW_3				// stores temperature reading of OW_Device_3

byte I2C_address[MAX_I2C_TEMP_DEVICES] = {0};						// stores indivitual address of each I2C digital temperator sensor
byte total_I2C_devices = 0;            	                				// stores total number of digital temperature sensors discovered on I2C bus
boolean temp_I2C_present = false;
unsigned int temp_I2C_1 = 0;			// TEMP_I2C_1				// stores current temperature reading of I2C_device_1
unsigned int temp_I2C_2 = 0;			// TEMP_I2C_2				// stores current temperature reading of I2C_device_2

const unsigned long LONG_UPDATE_INTERVAL = 1555556;  					// stores long time interval (in uS) for data updates
const float LONG_UPDATE_DIVISOR = 1.555556;        	       				// stores long update divisor (in Sec) for data updates
unsigned long long_update_tick = 0;							// stores current time reference for long update loop
unsigned long long_update_tock = 0;							// stores previous time reference for long update loop

const unsigned long SHORT_UPDATE_INTERVAL = 333334;  					// stores short time interval (in uS) for data updates
unsigned long short_update_tick = 0;							// stores current time reference for short update loop
unsigned long short_update_tock = 0;							// stores previous time reference for short update loop

const unsigned long KICKSTART_INTERVAL = 8000;						// sets time interval for kickstart function (in mS)
const unsigned long STARTUP_INTERVAL = 10000;						// sets time interval for startup function (in mS)
const unsigned long ALARM_STARTUP_INTERVAL = 4000;					// sets time interval for alarm startup function (in mS)
const unsigned long ALARM_INTERVAL = 500000;						// sets time interval for alarm startup function (in milliseconds)
unsigned long currentStartupCount = 0;							// stores time reference for KICKSTART & ALARM_STARTUP functions
unsigned long currentAlarmCount = 0;							// stores current time reference (in nS) for alarm function
unsigned long previousAlarmCount = 0;							// stores current time reference (in nS) for alarm function


void setup() {

	DDRD |= (1 << DDD3); 				// VCC_DRIVE (TIMER2)		// VCC_DRIVE (TIMER2) - pinMode(PIN_D3, OUTPUT)
	DDRD |= (1 << DDD4); PORTD &= ~(1 << PORTD4);	// ALARM_OUT			// ALARM_OUT - pinMode(PIN_D4, OUTPUT); digitalWrite(PIN_D4, LOW)
	DDRD &= ~(1 << DDD5); 				// SYSTEM_MODE (VCC/PWM)	// SYSTEM_MODE (VCC/PWM) - pinMode(PIN_D5, INPUT)
	DDRD &= ~(1 << DDD6); PORTD |= (1 << PORTD6);	// SYSTEM_CONTROL_1 (OVD)	// SYSTEM_CONTROL_1 (OVD) - pinMode(PIN_D6, INPUT_PULLUP)
	DDRD &= ~(1 << DDD7); 				// SYSTEM_CONTROL_2 (MAN/AUTO)	// SYSTEM_CONTROL_2 (MAN/AUTO) - pinMode(PIN_D7, INPUT)
	DDRB |= (1 << DDB0); PORTB &= ~(1 << PORTB0); 	// SYSTEM_CONTROL_LED		// SYSTEM_CONTROL_LED - pinMode(PIN_D8, OUTPUT); digitalWrite(PIN_D8, LOW)
	DDRB &= ~(1 << DDB1);  				// PUMP_RPM_1 (NO_PULLUP)	// PUMP_RPM_1 - pinMode(PIN_D9, INPUT)
//	DDRB &= ~(1 << DDB1);  PORTB |= (1 << PORTB1);	// PUMP_RPM_1 (PULLUP)		// PUMP_RPM_1 - pinMode(PIN_D9, INPUT_PULLUP)
	DDRB |= (1 << DDB2); 				// PWM_DRIVE (TIMER1)		// PWM_DRIVE (TIMER1) - pinMode(PIN_D10, OUTPUT)
	DDRB &= ~(1 << DDB3);				// PUMP_RPM_2 (NO_PULLUP)	// PUMP_RPM_2 - pinMode(PIN_D11, INPUT)
//	DDRB &= ~(1 << DDB3); PORTB |= (1 << PORTB3);	// PUMP_RPM_2 (PULLUP)		// PUMP_RPM_2 - pinMode(PIN_D11, INPUT_PULLUP)
	DDRB &= ~(1 << DDB4); 				// FLOW_LPM (NO_PULLUP)		// FLOW_LPM - pinMode(PIN_D12, INPUT)
//	DDRB &= ~(1 << DDB4); PORTB |= (1 << PORTB4);	// FLOW_LEP (PULLUP)		// FLOW_LPM - pinMode(PIN_D12, INPUT_PULLUP)
	DDRB &= ~(1 << DDB5); 				// PSU_RPM (NO_PULLUP)		// PSU_RPM - pinMode(PIN_D13, INPUT)
//	DDRB &= ~(1 << DDB5); PORTB |= (1 << PORTB5);	// PSU_RPM (PULLUP)		// PSU_RPM - pinMode(PIN_D13, INPUT_PULLUP)
	DDRC &= ~(1 << DDC0);				// TEMP_AN1			// TEMP_AN1 - pinMode(PIN_A0, INPUT)
	DDRC &= ~(1 << DDC1);				// TEMP_AN2			// TEMP_AN2 - pinMode(PIN_A1, INPUT)
	DDRC &= ~(1 << DDC2);				// PUMP_CONTROL			// PUMP_CONTROL - pinMode(PIN_A2, INPUT)
	DDRC &= ~(1 << DDC3);				// VCC_PUMP			// VCC_PUMP - pinMode(PIN_A3, INPUT)

	ADMUX = 0; 									// resets entire ADMUX register to zero (ADC OFF, auto-trigger disabled)
	ADMUX |= (1 << REFS0);								// configures AVCC as ADC voltage reference
	ADMUX |= ADC_GND;								// sets ADC Mux input channel to GND							
        ADCSRA = 0;									// resets entire ADCSRA register to zero
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);				// sets ADC with prescaler 128 (16MHz / 128 = 125KHz)
//	ADCSRA |= (1 << ADPS2) | (1 << ADPS1);						// sets ADC with prescaler 64 (16MHz / 64 = 250KHz)
//	ADCSRA |= (1 << ADPS2) | (1 << ADPS0);						// sets ADC with prescaler 32 (16MHz / 32 = 500KHz)
	ADCSRA |= (1 << ADEN) | (1 << ADSC);						// turns on ADC & preforms initial conversion


// TIMER1 SETUP //

	cli();										// disables global interrupts
		TCCR1A = 0;								// resets entire Timer1 "Control Register A" to zero
											// (normal operation: OC1A & OC1B disconnected)
		TCCR1B = 0;								// resets entire Timer1 "Control Register B" to zero 
											// (Timer1 is OFF)										
		TIMSK1 |= (1 << TOIE1); 						// enables Timer1 "compare interrupt mask register A"
		TCCR1A |= (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);			// set up Timer1 in FAST PWM mode (together with TCCR1B settings below)
											// (Timer1 prescaler = 1)
		TCCR1B |= (1 << WGM13) | (1 << WGM12);					// clears OC1B on Compare Match mode (OC1A as "TOP" & OC1B as "BOTTOM")
		OCR1A = 639;								// sets Timer1 "TOP" to generate 25KHz frequency (16Mhz / 1 / 640)
											// (Timer1 OVF ISR invoked every 40uS)
		OCR1B = 638;								// sets initial duty cycle on PIN_D10 (OC1B) to maximum value ("TOP"-1)
		TCCR1B |= (1 << CS10);							// turn on Timer1 (with prescaler 1)

// TIMER2 SETUP //

		TCCR2A = 0;								// resets entire Timer2 "Control Register A" to zero
											// (normal operation: OC2A & OC2B disconnected)
		TCCR2B = 0;								// resets entire Timer2 "Control Register B" to zero
											// (Timer2 is OFF)
		TIMSK2 |= (1 << TOIE2);							// enables Timer2 "compare interrupt mask register A"
		TCCR2A |= (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);			// set ups Timer2 in FAST PWM mode (together with TCCR2B settings below)
		TCCR2B |= (1 << WGM22);							// clears OC2B on Compare Match mode (OC2A as "TOP" & OC2B as "BOTTOM")
		OCR2A = 79;								// sets Timer2 "TOP" to generate 25KHz frequency (16Mhz / 8 / 80 = 25KHz)
											// (Timer2 OVF ISR invoked every 40uS)
		OCR2B = 78;								// sets initial duty cycle on PIN_D3 (OC2B) to maximum value ("TOP"-1)
		TCCR2B |= (1 << CS21);							// turns on Timer2 with prescaler 8
	sei();										// enables global interrupts


  	Wire.begin(PC_ADDRESS);                	        				// joins I2C bus

	TWBR = 12; 									// TWBR 12: 400KHz (max); 32: 200KHz; 72: 100KHz (default)

	Wire.onReceive(receive_Call);

	Wire.onRequest(request_Call);


	PCintPort::attachInterrupt(PIN_D9, burpCount_D9,FALLING);			// attaches software interrupt on PIN_D9

	PCintPort::attachInterrupt(PIN_D11, burpCount_D11,FALLING); 			// attaches software interrupt on PIN_D11

	PCintPort::attachInterrupt(PIN_D12, burpCount_D12,FALLING);			// attaches software interrupt on PIN_D12

	PCintPort::attachInterrupt(PIN_D13, burpCount_D13,FALLING);			// attaches software interrupt on PIN_D13


	total_OW_devices = discover_OW_Devices();					// searches & stores total number of temperature sensors on One-Wire bus (max: 4 devices)

	if (total_OW_devices > 1) temp_OW_multiple = true;


	total_I2C_devices = discover_I2C_Temp();    					// searchs for new devices on I2C bus

	if (total_I2C_devices > 0) temp_I2C_present = true;

	if (!DEBUG_MODE) {

//		Serial.begin(115200);							// opens serial port & begins serial communication
		Serial.begin(230400);							// opens serial port & begins serial communication
//		Serial.begin(250000);							// opens serial port & begins serial communication
//		Serial.begin(345600);							// opens serial port & begins serial communication
//		Serial.begin(500000);							// opens serial port & begins serial communication

	} else {

		Serial.begin(115200);							// opens serial port & begins serial communication
	}
		
	check_Serial();									// checks if serial port is open
}


void loop() {

	if ((got_data) && (!DEBUG_MODE)) send_Data_Processing();

 	short_update_tock = micros();							// current time reference for short update function

	long_update_tock = micros();							// current time reference for long update function

	currentStartupCount = millis();							// current time reference for startup function


	if ((short_update_tock - short_update_tick) >= SHORT_UPDATE_INTERVAL) {		// update function preformed every 0.333334 secondS

		short_update_tick = short_update_tock;					// stores current time reference for next update

		system_status = update_System_Status();

		update_Duty_Cycle();							// updates of TIMER1 & TIMER2 duty cycles

		if ((!DEBUG_MODE) && (serial_open)) PC_Short_Processing_Print();	// prints updated data to processing application
	}


	if ((long_update_tock - long_update_tick) >= LONG_UPDATE_INTERVAL) {		// update function preformed every 1.555556 secondS

		long_update_tick = long_update_tock;					// stores current time reference for next update

		vcc_IC = update_Vcc_IC(); 						// reads vcc_IC

		freq_VCC = read_Frequency(PIN_D3);					// calculates pwm frequency on PIN_D3

		freq_PWM = read_Frequency(PIN_D10);					// calculates pwm frequency on PIN_D10

		vcc_pump = read_VCC_Pump(PIN_A3);					// reads VCC output to PUMP1

		pump_rpm_1 = read_RPM(PIN_D9);						// updates rpm reading on PIN_D9

		pump_rpm_2 = read_RPM(PIN_D11);						// updates rpm reading on PIN_D11

		temp_AN1 = update_Temp_AN(PIN_A0);		// TEMP_AN1		// reads analog temperature sensor on PIN_A0

		psu_rpm = read_RPM(PIN_D13);						// updates rpm reading on PIN_D13

		pump_flow = read_Flow(PIN_D12);						// updates flow meter reading on PIN_D12 (multiplied by 100)

		temp_AN2 = update_Temp_AN(PIN_A1);		// TEMP_AN2		// reads analog temperature sensor on PIN_A1

		top_temp_AN = update_Top_Temp_AN();		// TEMP_AN_TOP

		temp_IC = update_Temp_IC();						// reads temp_IC

		ram_IC = update_Ram_IC();						// reads ram_IC

		temp_OW_1 = read_OW_Temp(OW_DEVICE_1);		// TEMP_OW_1

		if (temp_OW_multiple) update_OW_Temp();		// TEMP_OW		// updates reading of One-Wire temperature sensors if available

		if (temp_I2C_present) update_I2C_Temp();	// TEMP_I2C		// updates reading of I2C temeprature sensors if available

		if (!DEBUG_MODE) {					// PC_MODE

			if (serial_open) PC_Long_Processing_Print();			// prints updated data to processing application
			else check_Serial();

		} else {						// DEBUG_MODE

			if (serial_open) print_Long_Update();				// preforms debugging serial print function
			else check_Serial();
		}
	}
}


ISR(TIMER1_OVF_vect) {									// TIMER1 ISR (pwm frequency counter)
  	ovf1++;										// counts the number of TIMER1 overflows
}

ISR(TIMER2_OVF_vect) {									// TIMER2 ISR (pwm frequency counter)
  	ovf2++;										// counts the number of TIMER2 overflows
}


void burpCount_D9() {									// ISR for interrupt triggers on PIN_D9
  	currentBurp_D9++;
}

void burpCount_D11() {									// ISR for interrupt triggers on PIN_D11
  	currentBurp_D11++;
}

void burpCount_D13() {									// ISR for interrupt triggers on PIN_D13
  	currentBurp_D13++;
}

void burpCount_D12() {									// ISR for interrupt triggers on PIN_D12
  	currentBurp_D12++;
}


void receive_Call (int numBytes) {

	if (numBytes == PC_DATA_IN_COUNT) {
		for (int i = 0; i < numBytes; i++) {
			data_in[i] = Wire.read();
		}
		got_data = true;
	}
}



void request_Call() {	

	Wire.write(PC_PING);
}


unsigned int read_Frequency (const byte pin) {

	volatile unsigned long current_freq;		    	       			// stores total overflow count
	cli();										// disables global interrupts
		switch (pin) {
			case (PIN_D3): current_freq = ovf2; ovf2 = 0; break;		// stores number of TIMER2 overflows & resets Timer2 overflow counter to zero
			case (PIN_D10): current_freq = ovf1; ovf1 = 0; break;		// stores number of TIMER1 overflows & resets Timer1 overflow counter to zero
		}
     	sei();										// enables global interrupts
        current_freq = round(current_freq / LONG_UPDATE_DIVISOR);
	return current_freq;	                                            		// returns new frequency value
}


void update_Duty_Cycle() {

	static unsigned int previous_pump_control;
	static unsigned int delta_pump_control;

	system_control_1 = !(bitRead(PIND, 6));			// OVD_ON/OVD_OFF					// digitalRead(PIN_D6) (0 => OVD_OFF; 1 => OVD_ON)

	system_control_2 = bitRead(PIND, 7);			// MAN/AUTO						// digitalRead(PIN_D7) (0 => MAN; 1 => AUTO)

	system_mode = bitRead(PIND, 5);				// VCC/PWM						// digitalRead(PIN_D5) (0 => VCC; 1 => PWM)

	if ((currentStartupCount >= KICKSTART_INTERVAL) && (!system_control_1)) { 	// OVD_OFF			// SYSTEM_CONTROL_1 => OVD_OFF

		if (bitRead(PINB, 0)) PORTB &= ~(1 << PORTB0);				// OVD_LED_OFF			// turns off SYSTEM_MODE_LED (PIN_D8)

		if (!system_control_2) {			// MANUAL						// SYSTEM_CONTROL => MANUAL

			pump_control = read_ADC(ADC_A2);								// reads pot on PIN_A2 for duty cycle calculation
 
          		if (pump_control < 40) pump_control = 40;

           		if (pump_control > 880) pump_control = 880;

			delta_pump_control = abs(pump_control - previous_pump_control);

			if ((delta_pump_control >= 25) || (pump_control < 65) || (pump_control > 850)) previous_pump_control = pump_control;
                        else pump_control =  previous_pump_control;
				
			if (!system_mode) {			// MANUAL_VCC						// PUMP_MODE_VCC (TIMER2)
			
				current_duty_cycle = map_Float(float(pump_control), 40.0, 880.0, 28.0, 78.0);		// calculates duty cycle on PIN_D3 (minimum duty set to 25%)

				if ((ocr2b_data != current_duty_cycle) || (ocr1b_data != 638)) {
					cli(); 										// disables global interrupts
						if (OCR2B != current_duty_cycle) OCR2B = current_duty_cycle;		// sets TIMER2 duty cycle to to new value
						if (OCR1B != 638) OCR1B = 638;						// sets TIMER1 duty cycle to maximum value
					sei();										// enables global interrupts
					ocr2b_data = current_duty_cycle;						// stores value of OCR2B register
					ocr1b_data = 638;								// stores value of OCR1B register
				}
				duty_cycle_per = map_Float(float(current_duty_cycle), 28.0, 78.0, 36.0, 100.0);

			} else {				// MANUAL_PWM						// PUMP_MODE_PWM (TIMER1)

				current_duty_cycle = map_Float(float(pump_control), 40.0, 880.0, 230.0, 638.0);		// calculates duty cycle on PIN_D10 (minimum duty set to 25%)

				if ((ocr1b_data != current_duty_cycle) || (ocr2b_data != 78)) {
					cli(); 										// disables global interrupts
						if (OCR1B != current_duty_cycle) OCR1B = current_duty_cycle;		// sets TIMER1 duty cycle to to new value
						if (OCR2B != 78) OCR2B = 78;						// sets TIMER2 duty cycle to maximum value
					sei();										// enables global interrupts
					ocr1b_data = current_duty_cycle;						// stores value of OCR1B register
					ocr2b_data = 78;								// stores value of OCR2B register
				}
				duty_cycle_per = map_Float(float(current_duty_cycle), 230.0, 638.0, 36.0, 100.0);
			}

		} else {					// AUTO							// SYSTEM_CONTROL => AUTO

			if (!system_mode) {			// AUTO_VCC						// PUMP_MODE_VCC (TIMER2)

				current_duty_cycle = map_Float(float(top_temp_AN), 200.0, 550.0, 28.0, 78.0);

				if ((ocr2b_data != current_duty_cycle) || (ocr1b_data != 638)) {
					cli(); 										// disables global interrupts
						if (OCR2B != current_duty_cycle) OCR2B = current_duty_cycle;		// sets TIMER2 duty cycle to to new value
						if (OCR1B != 638) OCR1B = 638;						// sets TIMER1 duty cycle to maximum value
					sei();										// enables global interrupts
					ocr2b_data = current_duty_cycle;						// stores value of OCR2B register
					ocr1b_data = 638;								// stores value of OCR1B register
				}
				duty_cycle_per = map_Float(float(current_duty_cycle), 28.0, 78.0, 36.0, 100.0);

			} else {				// AUTO_PWM						// PUMP_MODE_PWM (TIMER1)			

				current_duty_cycle = map_Float(float(top_temp_AN), 200.0, 550.0, 230.0, 638.0);

				if ((ocr1b_data != current_duty_cycle) || (ocr2b_data != 78)) {
					cli(); 										// disables global interrupts
						if (OCR1B != current_duty_cycle) OCR1B = current_duty_cycle;		// sets TIMER1 duty cycle to to new value
						if (OCR2B != 78) OCR2B = 78;						// sets TIMER2 duty cycle to maximum value
					sei();										// enables global interrupts
					ocr1b_data = current_duty_cycle;						// stores value of OCR1B register
					ocr2b_data = 78;								// stores value of OCR2B register
				}
				duty_cycle_per = map_Float(float(current_duty_cycle), 230.0, 638.0, 36.0, 100.0);
			}
		}

	} else {									// OVD_ON			// SYSTEM_CONTROL_1 => OVERDRIVE_ON

		if (!(bitRead(PINB, 0))) PORTB |= (1 << PORTB0);			// OVD_LED_ON			// turns on SYSTEM_CONTROL_LED (PIN_D8)

		system_control_1 = true;     			 							// changes system_control_1 to indicate OVD_ON

		cli(); 													// disables global interrupts
			if (OCR1B != 638) OCR1B = 638;									// sets TIMER1 (PWM) to max value

			if (OCR2B != 78) OCR2B = 78;									// sets TIMER2 (VCC) to max value
 		sei();													// enables global interrupts
		
		if (ocr1b_data != 638) ocr1b_data = 638;

		if (ocr2b_data != 78) ocr2b_data = 78;
	
		if (duty_cycle_per != 100) duty_cycle_per = 100;							// sets duty_cycle_per to max value (in percent)
	}
}


unsigned int read_ADC (byte adc_channel) {			        		// analogRead() of selected input source

	static unsigned int adc_delay; 							// stoes adc delay length between first & second conversion
        ADMUX = 0;                                                              	// resets entire ADMUX reguster to zero
 	if (adc_channel != ADC_TEMP) ADMUX |= (1 << REFS0) | adc_channel;		// sets AVCC as ADC voltage reference and selected ADC input channel
	else ADMUX |= (1 << REFS1) | (1 << REFS0) | adc_channel;			// sets Internal 1V1 as ADC voltage reference and selected ADC input channel
	switch (adc_channel) {
                case (ADC_TEMP): adc_delay = 6500; break;				// waits 6500uS for ADC_TEMP channel reading 
		case (ADC_VCC): adc_delay = 500; break;			       		// waits 500uS for ADC_VCC channel reading 
		default: adc_delay = 20; break;	    		                	// waits 20uS for ADC_VCC channel reading 
	}
        delayMicroseconds(adc_delay);   
	ADCSRA |= (1 << ADSC);				  	                	// turns on ADC & preforms initial conversion
	while (bit_is_set(ADCSRA, ADSC));  						// waits until ADC conversion is done (ADSC bit is cleared)
        if ((adc_channel == ADC_A2) || (adc_channel == ADC_A3)){
  	      ADCSRA |= (1 << ADSC);				                	// turns on ADC & preforms initial conversion
	      while (bit_is_set(ADCSRA, ADSC));  					// waits until ADC conversion is done (ADSC bit is cleared)
        } 
        return ADCW; 	                                      	    	    		// returns latest ADC reading
}


unsigned int map_Float (float x, float x1, float x2, float y1, float y2) {

	static unsigned int result;
	if ((x2 - x1) != 0) result = round((((x - x1) * (y2 - y1)) / (x2 - x1)) + y1);
        else result = 0;	
	return result;
}


boolean update_System_Status() {

	if ((currentStartupCount >= ALARM_STARTUP_INTERVAL) && 				// ALARM_ON 			// checks for Alarm conditions
	   ((pump_rpm_1 < RPM_THRESHOLD) || 
	   (pump_rpm_2 < RPM_THRESHOLD) ||    						
	   (pump_flow < FLOW_THRESHOLD) || 
	   ((PSU_ALARM) && (psu_rpm < RPM_THRESHOLD)) || 
	   (temp_AN1 > AN_TEMP_THRESHOLD) ||
	   (temp_AN2 > AN_TEMP_THRESHOLD) || 
	   (temp_IC > IC_TEMP_THRESHOLD))) {   

		currentAlarmCount = micros();
		if((currentAlarmCount - previousAlarmCount) >= ALARM_INTERVAL) {  
			PORTD ^= (1 << PORTD4);						// ALARM_TOGGLE			// toggles ALARM_LED on PIN_D4
                	previousAlarmCount = currentAlarmCount;               
           	}
		return true;
 
   	} else {									// ALARM_OFF

		if (bitRead(PIND, 4)) PORTD &= ~(1 << PORTD4);                 						// digitalWrite(PIN_D4, LOW)
		return false;
        }
}


unsigned int read_RPM (const byte pin) {

	static unsigned int current_rpm;						// stores current rpm reading on selected pin
//	static volatile unsigned int current_rpm;					// stores current rpm reading on selected pin
	static unsigned int previous_rpm;						// stores previous rpm reading on selected pin
	static unsigned int delta_rpm;							// stores delta rpm between previous & current readings
	switch (pin) {
		case (PIN_D9): 								// PUMP_RPM_1
		     	previous_rpm = pump_rpm_1;    	
			cli();								// disables global interrupts
				current_rpm = currentBurp_D9;				// obtains current rpm count from interrupt routine
				currentBurp_D9 = 0;					// resets rpm count on PIN_D9 to zero
			sei();								// enables global interrupts
			break;
		case (PIN_D11): 							// PUMP_RPM_2
		     	previous_rpm = pump_rpm_2;    	
			cli();								// disables global interrupts
				current_rpm = currentBurp_D11;				// obtains current rpm count from interrupt routine
				currentBurp_D11 = 0;					// resets rpm count on PIN_D11 to zero
			sei();								// enables global interrupts
			break;
		case (PIN_D13): 							// PSU_RPM
		     	previous_rpm = psu_rpm;    	
			cli();								// disables global interrupts
				current_rpm = currentBurp_D13;				// obtains current rpm count from interrupt routine
				currentBurp_D13 = 0;					// resets rpm count on PIN_D13 to zero
			sei();								// enables global interrupts
			break;
	}
	current_rpm = round((current_rpm * 30.0) / LONG_UPDATE_DIVISOR);		// calculates current rpm within the update time interval
	delta_rpm = abs(float(current_rpm) - float(previous_rpm));			// calculates delta between previous & current rpm readings 
	if (current_rpm < 200) current_rpm = 0;
	if (current_rpm > 4000) current_rpm = previous_rpm;
	if ((current_rpm > 0) && (current_rpm > previous_rpm))
	   current_rpm = round((current_rpm + previous_rpm) / 2.0);			// averages previous & current readings
	if ((delta_rpm < 15) && (current_rpm != 0)) current_rpm = previous_rpm;		// anti-jitter function
	if (current_rpm < 200) current_rpm = 0;
	return current_rpm;
}


unsigned int read_Flow (const byte pin) {						// Flow meter reading

	static unsigned int current_flow;						// stores current flow rpm reading on selected pin
	static unsigned int previous_flow;						// stores previous flow rpm reading on selected pin
	static unsigned int delta_flow;							// stores delta flow rpm between previous & current readings
     	previous_flow = pump_flow;    							// updates previous flow data reading
	cli();										// disables global interrupts
		current_flow = currentBurp_D12;						// obtains current flow rpm count from interrupt routine
		currentBurp_D12 = 0;							// resets flow rpm count on PIN_D12 to zero
	sei();										// enables global interrupts
	current_flow = round((float(current_flow) * 30.0) / LONG_UPDATE_DIVISOR);	// calculates current flow rpm within the update time interval
        current_flow = round((FLOW_DIAMETER_OFFSET * float(current_flow)) / 600.0);   	// The flow meter (Koolance INS-FM17N) employs a
		       									// "normally open magnetic reed switch" to generate 
						        				// a signal that corresponds to the impeller's rotation rate
						       					// Flow meter's range: 1.0-15.0 LPM; liquid temp: 0 - 70 C
						 					// General formulae for calculation: 
 											// Flow Rate(LPM) = (0.307 * Frequency(Hz)) (based on 10mm ID nuzzle)
											// Flow Rate(LPM) = (0.280 * Frequency(Hz)) (based on 6mm ID nuzzle)
	delta_flow = abs((float(current_flow)) - (float(previous_flow)));		// calculates delta between previous & current flow rpm readings 
 	if ((current_flow > 0) && (current_flow > previous_flow)) 			
	   current_flow = round((current_flow + previous_flow) / 2.0);			// averages previous & current readings
	if ((delta_flow < 15) && (current_flow > 0)) current_flow = previous_flow;	// anti-jitter function
	return current_flow;
}


unsigned int update_Temp_AN (const byte temp_pin) {							

	static float analog_temp;
	static unsigned int current_an_data;										// stores current reading for return on selected pin (multiplied by 10)
	static unsigned int previous_an_data_1;
	static unsigned int previous_an_data_2;
	static unsigned int delta_an_data;
	
	switch (temp_pin) {												// sets offset coefficient for selected pin

		case (PIN_A0):						// TEMP_AN1 

			current_an_data = read_ADC(ADC_A0);								// calculates temperature on selected pin

   			if (current_an_data != 0) {									// if result is different from zero:
	 			analog_temp = log((1024.0 * (float(TEMP_RESISTOR_A0) * TEMP_OFFSET_A0) / 		// calculates temperature reading in 0°C
					      float(current_an_data)) - (float(TEMP_RESISTOR_A0) * TEMP_OFFSET_A0));	// and multiplies result by 10
				current_an_data = round((1.0 / (0.001129148 + (0.000234125 *  				// returns analog temperature reading (*10) 
 	            				  analog_temp) + (0.0000000876741 * analog_temp * 
		    				  analog_temp * analog_temp)) - 273.15) * 10.0);
   	    		}
 			if (current_an_data <= 1500) {

				delta_an_data = abs(current_an_data - previous_an_data_1);
				if ((delta_an_data > 5) || (current_an_data < 50)) previous_an_data_1 = current_an_data;
				else current_an_data = previous_an_data_1;
			} else {
				current_an_data = 0;
			}
			break;


		case (PIN_A1): 						// TEMP_AN2

			current_an_data = read_ADC(ADC_A1);								// calculates temperature on selected pin

   			if (current_an_data != 0) {									// if result is different from zero:
	 			analog_temp = log((1024.0 * (float(TEMP_RESISTOR_A1) * TEMP_OFFSET_A1) / 		// calculates temperature reading in 0°C
					      float(current_an_data)) - (float(TEMP_RESISTOR_A1) * TEMP_OFFSET_A1));	// and multiplies result by 10
				current_an_data = round((1.0 / (0.001129148 + (0.000234125 *  				// returns analog temperature reading (*10) 
 	            				  analog_temp) + (0.0000000876741 * analog_temp * 
		    				  analog_temp * analog_temp)) - 273.15) * 10.0);
   	    		}
 			if (current_an_data <= 1500) {

				delta_an_data = abs(current_an_data - previous_an_data_2);
				if ((delta_an_data > 5) || (current_an_data < 50)) previous_an_data_2 = current_an_data;
				else current_an_data = previous_an_data_2;
			} else {
				current_an_data = 0;
			}
			break;
	}
	return current_an_data;
}


unsigned int update_Top_Temp_AN() {

	static unsigned int current_top_temp;
	static unsigned int previous_top_temp;
	static unsigned int delta_top_temp;

	if (temp_AN1 >= temp_AN2) current_top_temp = temp_AN1;
	else current_top_temp = temp_AN2;

	delta_top_temp = abs(current_top_temp - previous_top_temp);

	if (delta_top_temp > 10) previous_top_temp = current_top_temp;
	else current_top_temp = previous_top_temp;

	if (current_top_temp < 200) current_top_temp = 200;

	if (current_top_temp > 550) current_top_temp = 550;

	return current_top_temp;	
}


unsigned int read_VCC_Pump (const byte pin) {

	static byte adc_channel_pump;								// stores selected ADC channel in read_Temp_AN() function
	static unsigned int pump_vcc_reading;							// stores ADc reading on PIN_A3

	pump_vcc_reading = read_ADC(ADC_A3);							// updates ADC reading on PIN_A3
 	pump_vcc_reading = map_Float(pump_vcc_reading, 420, 788, 6390, 12280);			// maps ADC reading to actual voltage (in mV)
	return pump_vcc_reading;								// returns actual voltage readings (in mV)
}


unsigned int update_Vcc_IC() {

	static unsigned int current_adc_vcc;
	static unsigned int previous_adc_vcc;

        current_adc_vcc = read_ADC(ADC_VCC);
	
	if (current_adc_vcc != 0) current_adc_vcc = round(1125300.0 / float(current_adc_vcc));

	if ((abs(current_adc_vcc - previous_adc_vcc) >= 50) || (current_adc_vcc < 2000)) previous_adc_vcc = current_adc_vcc;
	else current_adc_vcc = previous_adc_vcc;							

	return current_adc_vcc;
}


unsigned int update_Temp_IC() {
	
	static unsigned int current_adc_temp;
	static unsigned int previous_adc_temp;
	static unsigned int delta_adc_temp;

        current_adc_temp = read_ADC(ADC_TEMP);

  	current_adc_temp = round((((float(current_adc_temp)) - 324.31) / 1.22) * 10.0);

	delta_adc_temp = abs(current_adc_temp - previous_adc_temp);

	if ((delta_adc_temp >= 10) || (current_adc_temp < 50)) previous_adc_temp = current_adc_temp;
	else current_adc_temp = previous_adc_temp;

	return current_adc_temp;
}


int update_Ram_IC() {

	extern int __heap_start, *__brkval; 
	int v; 
	return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


void check_Serial () {

	if (Serial) serial_open = true;									// checks if serial port is open
	else serial_open = false;
}


byte discover_OW_Devices() {										// One-wire device search and initialization

	static byte wire_device_count;									// stores number of address of each One-Wire discovered device

	while ((wire_device_count < MAX_OW_DEVICES) && 							// searches One-Wire bus for devices & stores the
	      (wireBus.search(wire_address[wire_device_count]))) {  					// indivitual 8-byte address of each discovered device
	    	wire_device_count++;
	}
	wireBus.reset();										// resets One-Wire bus
	if (wire_device_count == 0) {									// second attempt to discover devices if no devices found during first attempt
		delayMicroseconds(250);
		while ((wire_device_count < MAX_OW_DEVICES) && 						// re-searches bus for devices & stores the
		       	(wireBus.search(wire_address[wire_device_count]))) {   				// stores indivitual 8-byte address of each device
			wire_device_count++;
		}
  		wireBus.reset();									// resets One-Wire bus
	}
	if (wire_device_count > 0) {									// initializes temperature sensor wire devices that were discovered
		for (byte k=0; k < wire_device_count; k++) {						// initialization function for each discovered device
			if ((wire_address[k][0] == 0x28) &&			      			// if wire device is Dallas DS18B20 temperature sensor and
			   ((OneWire::crc8(wire_address[k], 7)) == (wire_address[k][7]))) {    		// crc valitation of device address is successful:
             	  		wireBus.reset();							// resets One-Wire bus
             	  		wireBus.select(wire_address[k]);					// selects a specific device
              	 		wireBus.write(0x44);         						// starts temperature reading (conversion) fuction
             	  		wireBus.reset();							// resets One-Wire bus
			} 
		}
	}
	return wire_device_count;	              							// returns total number of One-Wire devices discovered on bus
}


void update_OW_Temp() {

	switch (total_OW_devices) {									// reads available One-Wire temperature sensors on One-Wire bus
		case (2): temp_OW_2 = read_OW_Temp(OW_DEVICE_2); break;					// reads second One-Wire digital temperature sensor
		case (3): temp_OW_2 = read_OW_Temp(OW_DEVICE_2); 					// reads second One-Wire digital temperature sensor
			  temp_OW_3 = read_OW_Temp(OW_DEVICE_3); break;		 			// reads third One-Wire digital temperature sensor
	}
}


unsigned int read_OW_Temp (const byte wire_device_number) {						// One-wire digital temperature sensor reading

	static boolean wire_device_present;								// stores device presence (device indicates presence by sending 1 to bus master)
	static byte wire_raw_data[12];									// stores raw data of temeprature reading from device
	static float current_temp_wire_data;  								// stores new dats reading from device
	static byte wire_validation_flag[MAX_OW_DEVICES];						// stores value of crc validation flag counter for each One-Wire device
	static unsigned int new_temp_wire;								// stores new temperature reading from device
	static unsigned int previous_temp_wire[MAX_OW_DEVICES];						// stores previous temperature reading from device
	static unsigned int delta_temp_wire;

	new_temp_wire = 0;

	wire_device_present = wireBus.reset();                						// checks if device(s) is present on the bus

	if (wire_device_present) {   									// verifies device(s) presence
        	wireBus.select(wire_address[wire_device_number]);					// selects specific device for communication
		wireBus.write(0xBE);         								// requests raw data of latest temperature reading from device
		for (byte i = 0; i < 9; i++) {      		     					// reads 9 bytes of raw data
	      		wire_raw_data[i] = wireBus.read();						// resolution in set by default to 12-bits (750ms conversion time)
          	}
		if ((wire_raw_data[8]) == (OneWire::crc8(wire_raw_data, 8))) {					// crc valiation check of raw data (compares recieved crc data byte with calculated crc)
  			current_temp_wire_data = (((wire_raw_data[1] << 8) | wire_raw_data[0]) / 16.0);		// stores new temperature reading based on raw data calculation
			new_temp_wire = round(current_temp_wire_data * 10.0);					// calculates new temperature reading multiplied by 10
		}
	
		if (new_temp_wire < 1500) {					// LEGITIMATE VALUE

			wire_validation_flag[wire_device_number] = 0;						// resets crc validation flag count on successful validation
			delta_temp_wire = abs(new_temp_wire - previous_temp_wire[wire_device_number]);
			if ((delta_temp_wire > 5) || (new_temp_wire < 50)) previous_temp_wire[wire_device_number] = new_temp_wire;
			else new_temp_wire = previous_temp_wire[wire_device_number];

		} else {							// ILLEGITIMATE VALUE		// if reading value not valid
			wire_validation_flag[wire_device_number]++;						// increments data validation flag count by one
			if ((wire_validation_flag[wire_device_number]) > 1) {    				// if 2 or more crc checks are unsuccessful in a row
				wire_validation_flag[wire_device_number] = 2;					// resets wire validation flag number to 2 to prevent byte overflow
				new_temp_wire = 0;								// sets new temperature reading to 0°C
  			} else {										// if less than 3 crc checks unsuccessful
				new_temp_wire = previous_temp_wire[wire_device_number]; 			// sets previous reading of WIRE_DEVICE
			}
   	 	}
	}
	wireBus.reset();										// resets One-Wire bus
	wireBus.select(wire_address[wire_device_number]);						// re-selects device for communication
	wireBus.write(0x44); 										// starts new temperature reading (conversion) fuction
	wireBus.reset();										// resets One-Wire bus

	return new_temp_wire;
}	


byte discover_I2C_Temp() {									// I2c digital sensor search & log function

	static byte I2C_device_count;								// stores number of address of each I2C discovered device
	for (byte i = 72; i < 76; i++) {							// searches bus for devices & stores the indivitual
		if (I2C_device_count < MAX_I2C_TEMP_DEVICES) { 					// searches I2C bus for devices (MAX: 4 devices)
			Wire.beginTransmission (i);						
    			if (Wire.endTransmission() == 0) {					// checks if I2C device/s respond to I2C Master call
      				I2C_address[I2C_device_count] = i;				// stores indivitual 1-byte address of each discovered device
      				I2C_device_count++;		
      			}
		}
 	}
	return I2C_device_count;
}


void update_I2C_Temp() {

	switch (total_I2C_devices) {								// reads available temperature sensors on I2C bus
		case (1): temp_I2C_1 = read_I2C_Temp(I2C_DEVICE_1); break;
		case (2): temp_I2C_1 = read_I2C_Temp(I2C_DEVICE_1);
			  temp_I2C_2 = read_I2C_Temp(I2C_DEVICE_2); break;
	}
}


unsigned int read_I2C_Temp (const byte I2C_temp_device_number) {   				// I2C digital temperature sensor reading function

	static unsigned int I2C_device_address;							// stores I2C_address of selected device
//	static byte totalBytesRecieved;								// stores total number of bytes recieved from device
	static byte high_byte;           	             	    				// device sends MSB first
	static byte low_byte;               	         	    				// device sends LSB second
	static float current_temp_I2C_data;							// stores current value of device
	static byte I2C_validation_flag[MAX_I2C_TEMP_DEVICES];					// stores value of crc validation flag counter for each I2C digital sensor
	static unsigned int new_I2C_temp;
	static unsigned int previous_I2C_temp[MAX_I2C_TEMP_DEVICES];				// stores previous value of device
	static unsigned int delta_I2C_temp;

	switch(I2C_temp_device_number) {
		case (I2C_DEVICE_1): I2C_device_address = I2C_address[I2C_DEVICE_1]; break;	// sets address of I2C_DEVICE_1
		case (I2C_DEVICE_2): I2C_device_address = I2C_address[I2C_DEVICE_2]; break;	// sets address of I2C_DEVICE_2
	}

	new_I2C_temp = 0;

//	totalBytesRecieved = Wire.requestFrom(I2C_device_address, 2);      			// requests 2 bytes from device (function returns the number of bytes recieved
//	if (totalBytesRecieved == 2) {					// GOT_RAW_DATA		// if exactly 2 bytes were recieved from device

	if ((Wire.requestFrom(I2C_device_address, 2)) == 2) {		// GOT_RAW_DATA		// if exactly 2 bytes were recieved from device

		high_byte = Wire.read();							// reads MSB byte from device
 		low_byte = Wire.read();								// reads LSB byte from device
 		low_byte &= 224;                          					// removes all bits except the 3 left bits
   		low_byte = (low_byte >> 5);                            				// shifts remaining 5 bits for the right value
 		current_temp_I2C_data = (((high_byte * 8) + low_byte) * 0.125); 		// MSB * 8 (first bit is not 1, but 8, second is 16, and so on); 0.125°C/unit
		new_I2C_temp = round(current_temp_I2C_data * 10.0);				// calculates new temperature reading multiplied by 10
 	
		if (new_I2C_temp < 1500) {					// LEGITIMATE VALUE
			I2C_validation_flag[I2C_temp_device_number] = 0;				// resets crc validation flag count on legitimate reading value
 
			delta_I2C_temp = abs(new_I2C_temp - previous_I2C_temp[I2C_temp_device_number]);

			if ((delta_I2C_temp > 5) || (new_I2C_temp < 50)) previous_I2C_temp[I2C_temp_device_number] = new_I2C_temp;
			else new_I2C_temp = previous_I2C_temp[I2C_temp_device_number];

		} else {							// ILLEGITIMATE VALUE	// if reading value is illegitimate

			I2C_validation_flag[I2C_temp_device_number]++;					// increments data validation flag count by one
			if ((I2C_validation_flag[I2C_temp_device_number]) > 1) {			// if 2 or more crc validation checks unsuccessful in a row:
				I2C_validation_flag[I2C_temp_device_number] = 2;			// resets crc validation flag number to 2 to prevent byte  overflow
				new_I2C_temp = 0;							// sets reading to 0°C
			} else {									// if less than 2 crc validation checks unsuccessful in a row:
				new_I2C_temp = previous_I2C_temp[I2C_temp_device_number];
			}
		}

	}

	return new_I2C_temp;
}


void print_Long_Update() {

	if (PRINT_FIRST_ROW) {

		Serial.println(F(""));
		Serial.println(F(""));

		Serial.print(F("freq_VCC: "));		
		Serial.print((float(freq_VCC)/1000.0), 3);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("freq_PWM: "));		
		Serial.print((float(freq_PWM)/1000.0), 3);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("SYSTEM_STATUS: "));
		if (!system_status) Serial.print(F("OK"));
		else Serial.print(F("ALARM"));
		Serial.print('\t');
		Serial.print('\t');

		if (!system_control_1) Serial.print(F("OVD_OFF"));
		else Serial.print(F("OVD_ON"));
		Serial.print('\t');
		Serial.print('\t');

		if (!system_control_2) Serial.print(F("MAN"));
		else Serial.print(F("AUTO"));
		Serial.print('\t');
		Serial.print('\t');

		if (!system_mode) Serial.print(F("VCC"));
		else Serial.print(F("PWM"));
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("PUMP_CONTROL: "));		
		Serial.print(pump_control);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("DUTY_CYCLE: "));		
		Serial.print(duty_cycle_per);
		Serial.print(F("%"));
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("VCC_DRIVE: "));
		Serial.print(ocr2b_data);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("PWM_DRIVE: "));
		Serial.print(ocr1b_data);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("VCC_PUMP: "));		
		Serial.print((float(vcc_pump)/1000.0), 3);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("TEMP_AN1: "));		
		Serial.print((float(temp_AN1)/10.0), 1);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("TEMP_AN2: "));		
		Serial.print((float(temp_AN2)/10.0), 1);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("TOP_TEMP_AN: "));		
		Serial.print((float(top_temp_AN)/10.0), 1);
	}

	if (PRINT_SECOND_ROW) {
	
		Serial.println(F(""));
		Serial.println(F(""));

		Serial.print(F("PUMP_RPM_1: "));		
		Serial.print(pump_rpm_1);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("PUMP_RPM_2: "));		
		Serial.print(pump_rpm_2);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("PSU_RPM: "));		
		Serial.print(psu_rpm);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("PUMP_FLOW: "));		
		Serial.print((float(pump_flow)/100.0), 2);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("VCC_IC: "));		
		Serial.print((float(vcc_IC)/1000.0), 3);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("TEMP_IC: "));		
		Serial.print((float(temp_IC)/10.0), 1);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("RAM_IC: "));		
		Serial.print(ram_IC);
		Serial.print('\t');
		Serial.print('\t');

		Serial.print(F("TEMP_OW_1: "));		
		Serial.print((float(temp_OW_1)/10.0), 1);


		if (total_OW_devices > 1) {
			Serial.print('\t');
			Serial.print('\t');
			Serial.print(F("TEMP_OW_2: "));		
			Serial.print((float(temp_OW_2)/10.0), 1);
		}
	
		if (total_OW_devices > 2) {
			Serial.print('\t');
			Serial.print('\t');
			Serial.print(F("TEMP_OW_3: "));		
			Serial.print((float(temp_OW_3)/10.0), 1);
		}

		if (total_I2C_devices > 0) {
			Serial.print('\t');
			Serial.print('\t');
			Serial.print(F("temp_I2C_1: "));		
			Serial.print((float(temp_I2C_1)/10.0), 1);
		}

		if (total_I2C_devices > 1) {
			Serial.print('\t');
			Serial.print('\t');
			Serial.print(F("temp_I2C_2: "));		
			Serial.print((float(temp_I2C_2)/10.0), 1);
		}
	}
}


void PC_Short_Processing_Print() {

	PC_Processing_Print_Data(PC_SYSTEM_STATUS);
	PC_Processing_Print_Data(PC_SYSTEM_CONTROL_1);
	PC_Processing_Print_Data(PC_SYSTEM_CONTROL_2);
	PC_Processing_Print_Data(PC_SYSTEM_MODE);
	PC_Processing_Print_Data(PC_DUTY_CYCLE_PER);
}


void PC_Long_Processing_Print() {

	PC_Processing_Print_Data(PC_VCC_PUMP);
	PC_Processing_Print_Data(PC_VCC_ICP);
	PC_Processing_Print_Data(PC_PUMP_RPM_1);
	PC_Processing_Print_Data(PC_PUMP_RPM_2);
	PC_Processing_Print_Data(PC_PSU_RPM);
	PC_Processing_Print_Data(PC_PUMP_FLOW);
	PC_Processing_Print_Data(PC_TEMP_AN1);
	PC_Processing_Print_Data(PC_TEMP_AN2);
	PC_Processing_Print_Data(PC_TEMP_DGP);
	PC_Processing_Print_Data(PC_TEMP_ICP);
	PC_Processing_Print_Data(PC_FREQ_VCC);
	PC_Processing_Print_Data(PC_FREQ_PWM);
	PC_Processing_Print_Data(PC_RAM_ICP);
	if (total_OW_devices > 1) PC_Processing_Print_Data(PC_TEMP_OW_1);
	if (total_OW_devices > 2) PC_Processing_Print_Data(PC_TEMP_OW_2);
	if (total_I2C_devices > 0) PC_Processing_Print_Data(PC_TEMP_I2C_1);
	if (total_I2C_devices > 1) PC_Processing_Print_Data(PC_TEMP_I2C_2);
}


void PC_Processing_Print_Data (const unsigned int processing_type) {

	static unsigned int processing_current_data;
	static unsigned int processing_previous_data[14];
	static unsigned int processing_crc;
	static float processing_new_data;
	static byte processing_decimal_places;
	static boolean processing_update_data;

	processing_update_data = false;

	switch (processing_type) {

		case (PC_SYSTEM_STATUS):						// OK/ALARM
			processing_current_data = (unsigned int)system_status;
			processing_new_data = float(processing_current_data);
			processing_decimal_places = 0;
			processing_update_data = true;					
			break;

		case (PC_SYSTEM_CONTROL_1):						// OVD_OFF/OVD_ON
			processing_current_data = (unsigned int)system_control_1;
			processing_new_data = float(processing_current_data);
			processing_decimal_places = 0;
			processing_update_data = true;					
			break;

		case (PC_SYSTEM_CONTROL_2):						// MAN/AUTO
			processing_current_data = (unsigned int)system_control_2;
			processing_new_data = float(processing_current_data);
			processing_decimal_places = 0;
			processing_update_data = true;					
			break;

		case (PC_VCC_PUMP):							// VCC_PUMP
			processing_current_data = vcc_pump;
			processing_new_data = (float(processing_current_data) / 1000.0);
			processing_decimal_places = 3;
			processing_update_data = true;					
			break;

		case (PC_VCC_ICP):							// VCC_ICP
			processing_current_data = vcc_IC;
			if ((processing_current_data != processing_previous_data[0]) ||		// UPDATE
			   (currentStartupCount < STARTUP_INTERVAL) || 
			   (processing_current_data == 0)) { 
				processing_previous_data[0] = processing_current_data;
				processing_new_data = (float(processing_current_data) / 1000.0);
				processing_decimal_places = 3;
				processing_update_data = true;					
			}
			break;

		case (PC_SYSTEM_MODE):							// VCC/PWM
			processing_current_data = (unsigned int)system_mode;
			processing_new_data = float(processing_current_data);
			processing_decimal_places = 0;
			processing_update_data = true;					
			break;

		case (PC_PUMP_RPM_1):							// PUMP_RPM_1
			processing_current_data = pump_rpm_1;
			if ((processing_current_data != processing_previous_data[1]) ||		// UPDATE
//			   (processing_current_data == 0) ||
			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[1] = processing_current_data;
				processing_new_data = float(processing_current_data);
				processing_decimal_places = 0;
				processing_update_data = true;					
			}
			break;

		case (PC_PUMP_RPM_2):							// PUMP_RPM_2
			processing_current_data = pump_rpm_2;
			if ((processing_current_data != processing_previous_data[2]) ||		// UPDATE
//			   (processing_current_data == 0) ||
			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[2] = processing_current_data;
				processing_new_data = float(processing_current_data);
				processing_decimal_places = 0;
				processing_update_data = true;					
			}
			break;

		case (PC_PSU_RPM):							// PSU_RPM
			processing_current_data = psu_rpm;
			if ((processing_current_data != processing_previous_data[3]) ||		// UPDATE
//			   (processing_current_data == 0) ||
			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[3] = processing_current_data;
				processing_new_data = float(processing_current_data);
				processing_decimal_places = 0;
				processing_update_data = true;					
			}
			break;

		case (PC_PUMP_FLOW):							// PUMP_FLOW
			processing_current_data = pump_flow;
//			if ((processing_current_data != processing_previous_data[4]) ||		// UPDATE
//			   (processing_current_data == 0) ||
//			   (currentStartupCount < STARTUP_INTERVAL)) {
//				processing_previous_data[4] = processing_current_data;
				processing_new_data = (float(processing_current_data) / 100.0);
				processing_decimal_places = 2;
				processing_update_data = true;					
//			}
			break;

		case (PC_DUTY_CYCLE_PER):						// DUTY_CYCLE_PER
			processing_current_data = duty_cycle_per;
			processing_new_data = float(processing_current_data);
			processing_decimal_places = 0;
			processing_update_data = true;					
			break;

		case (PC_TEMP_AN1):							// TEMP_AN1
			processing_current_data = temp_AN1;
			processing_new_data = (float(processing_current_data) / 10.0);
			processing_decimal_places = 1;
			processing_update_data = true;					
			break;

		case (PC_TEMP_AN2):							// TEMP_AN2
			processing_current_data = temp_AN2;
			processing_previous_data[8] = processing_current_data;
			processing_new_data = (float(processing_current_data) / 10.0);
			processing_decimal_places = 1;
			processing_update_data = true;					
			break;

		case (PC_TEMP_DGP):							// TEMP_OW_1
			processing_current_data = temp_OW_1;
			if ((processing_current_data != processing_previous_data[5]) ||		// UPDATE
			   (processing_current_data == 0) ||
			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[5] = processing_current_data;
				processing_new_data = (float(processing_current_data) / 10.0);
				processing_decimal_places = 1;
				processing_update_data = true;					
			}
			break;

		case (PC_TEMP_ICP):							// TEMP_ICP
			processing_current_data = temp_IC;
			if ((processing_current_data != processing_previous_data[6]) ||		// UPDATE
			   (processing_current_data == 0) ||
			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[6] = processing_current_data;
				processing_new_data = (float(processing_current_data) / 10.0);
				processing_decimal_places = 1;
				processing_update_data = true;					
			}
			break;

		case (PC_FREQ_VCC):							// FREQ_VCC
			processing_current_data = freq_VCC;
			if ((processing_current_data != processing_previous_data[7]) ||		// UPDATE
			   (processing_current_data == 0) ||
			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[7] = processing_current_data;
				processing_new_data = (float(processing_current_data) / 1000.0);
				processing_decimal_places = 3;
				processing_update_data = true;					
			}
			break;

		case (PC_FREQ_PWM):							// FREQ_PWM
			processing_current_data = freq_PWM;
			if ((processing_current_data != processing_previous_data[8]) ||		// UPDATE
			   (processing_current_data == 0) ||
			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[8] = processing_current_data;
				processing_new_data = (float(processing_current_data) / 1000.0);
				processing_decimal_places = 3;
				processing_update_data = true;					
			}
			break;

		case (PC_RAM_ICP):							// RAM_ICP
			processing_current_data = ram_IC;
			if ((processing_current_data != processing_previous_data[9]) ||		// UPDATE
			   (processing_current_data == 0) ||
			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[9] = processing_current_data;
				processing_new_data = float(processing_current_data);
				processing_decimal_places = 0;
				processing_update_data = true;					
			}
			break;

		case (PC_TEMP_OW_1):							// TEMP_OW_2
			processing_current_data = temp_OW_2;
//			if ((processing_current_data != processing_previous_data[10]) ||		// UPDATE	
//			   (processing_current_data == 0) ||
//			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[10] = processing_current_data;				
				processing_new_data = (float(processing_current_data) / 10.0);
				processing_decimal_places = 1;
				processing_update_data = true;					
//			}
			break;

		case (PC_TEMP_OW_2):							// TEMP_OW_3
			processing_current_data = temp_OW_3;
//			if ((processing_current_data != processing_previous_data[11]) ||		// UPDATE	
//			   (processing_current_data == 0) ||
//			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[11] = processing_current_data;				
				processing_new_data = (float(processing_current_data) / 10.0);
				processing_decimal_places = 1;
				processing_update_data = true;					
//			}
			break;

		case (PC_TEMP_I2C_1):							// TEMP_I2C_1
			processing_current_data = temp_I2C_1;
//			if ((processing_current_data != processing_previous_data[12]) ||		// UPDATE	
//			   (processing_current_data == 0) ||
//			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[12] = processing_current_data;				
				processing_new_data = (float(processing_current_data) / 10.0);
				processing_decimal_places = 1;
				processing_update_data = true;					
//			}
			break;

		case (PC_TEMP_I2C_2):							// TEMP_I2C_2
			processing_current_data = temp_I2C_2;
//			if ((processing_current_data != processing_previous_data[13]) ||		// UPDATE	
//			   (processing_current_data == 0) ||
//			   (currentStartupCount < STARTUP_INTERVAL)) {
				processing_previous_data[13] = processing_current_data;				
				processing_new_data = (float(processing_current_data) / 10.0);
				processing_decimal_places = 1;
				processing_update_data = true;					
//			}
			break;
	}

	if (processing_update_data) {
	
		processing_crc = (processing_type + processing_current_data);
		if (processing_crc <= 9999) processing_crc += 9999;
		Serial.print(processing_type);					// PROCESSING_TYPE
		Serial.print(F(","));
		Serial.print(processing_crc);					// PROCESSING_CRC
		Serial.print(F(","));
	      	Serial.print(processing_new_data, processing_decimal_places); 	// PROCESSING_DATA
 	      	Serial.print(F("!"));
	}
}


void send_Data_Processing() {

	static union dataItem {byte b[2]; unsigned int i;} data_item_1, data_item_2, data_item_3, data_item_4, data_item_5, data_item_6, data_item_7,
							   crc_item_1, crc_item_2, crc_item_3, crc_item_4, crc_item_5, crc_item_6, crc_item_7;

	static boolean data_ready;

	if (data_in[0] == data_in[1]) {		// PACKAGE IDENTIFIER VERIFICATION

		cli();
			data_item_1.b[0] = data_in[2];
			data_item_1.b[1] = data_in[3];
			crc_item_1.b[0] = data_in[4];
			crc_item_1.b[1] = data_in[5];
			data_item_2.b[0] = data_in[6];
			data_item_2.b[1] = data_in[7];
			crc_item_2.b[0] = data_in[8];
			crc_item_2.b[1] = data_in[9];
			data_item_3.b[0] = data_in[10];
			data_item_3.b[1] = data_in[11];
			crc_item_3.b[0] = data_in[12];
			crc_item_3.b[1] = data_in[13];
			data_item_4.b[0] = data_in[14];
			data_item_4.b[1] = data_in[15];
			crc_item_4.b[0] = data_in[16];
			crc_item_4.b[1] = data_in[17];
			data_item_5.b[0] = data_in[18];
			data_item_5.b[1] = data_in[19];
			crc_item_5.b[0] = data_in[20];
			crc_item_5.b[1] = data_in[21];
			data_item_6.b[0] = data_in[22];
			data_item_6.b[1] = data_in[23];
			crc_item_6.b[0] = data_in[24];
			crc_item_6.b[1] = data_in[25];
			data_item_7.b[0] = data_in[26];
			data_item_7.b[1] = data_in[27];
			crc_item_7.b[0] = data_in[28];
			crc_item_7.b[1] = data_in[29];
		sei();

		data_ready = true;
	
		got_data = false;
	}

	if (data_ready) {

		data_ready = false;

		switch (data_in[0]) {		// PACKAGE IDENTIFIER			

			case (PC_PACKAGE_1):

				Serial.print(FC_SYSTEM_CONTROL_1);
				Serial.print(F(","));
				Serial.print(crc_item_1.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_1.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_SYSTEM_CONTROL_2);
				Serial.print(F(","));
				Serial.print(crc_item_2.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_2.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_FAN_MODE_1);
				Serial.print(F(","));
				Serial.print(crc_item_3.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_3.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_FAN_MODE_2);
				Serial.print(F(","));
				Serial.print(crc_item_4.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_4.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_DUTY_CYCLE_PER_1);
				Serial.print(F(","));
				Serial.print(crc_item_5.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_5.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_DUTY_CYCLE_PER_2);
				Serial.print(F(","));
				Serial.print(crc_item_6.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_6.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_TEMP_OW_3);
				Serial.print(F(","));
				Serial.print(crc_item_7.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_7.i) / 10.0), 1);
 	      			Serial.print(F("!"));

				break;

			case (PC_PACKAGE_2):

				Serial.print(FC_FAN_VCC_1);
				Serial.print(F(","));
				Serial.print(crc_item_1.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_1.i) / 1000.0), 3);
 	      			Serial.print(F("!"));

				Serial.print(FC_FAN_VCC_2);
				Serial.print(F(","));
				Serial.print(crc_item_2.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_2.i) / 1000.0), 3);
 	      			Serial.print(F("!"));

				Serial.print(FC_FREQ_VCC_1);
				Serial.print(F(","));
				Serial.print(crc_item_3.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_3.i) / 1000.0), 3);
 	      			Serial.print(F("!"));

				Serial.print(FC_FREQ_VCC_2);
				Serial.print(F(","));
				Serial.print(crc_item_4.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_4.i) / 1000.0), 3);
 	      			Serial.print(F("!"));

				Serial.print(FC_FREQ_PWM_1);
				Serial.print(F(","));
				Serial.print(crc_item_5.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_5.i) / 1000.0), 3);
 	      			Serial.print(F("!"));

				Serial.print(FC_FREQ_PWM_2);
				Serial.print(F(","));
				Serial.print(crc_item_6.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_6.i) / 1000.0), 3);
 	      			Serial.print(F("!"));

				Serial.print(FC_TEMP_OW_4);
				Serial.print(F(","));
				Serial.print(crc_item_7.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_7.i) / 10.0), 1);
 	      			Serial.print(F("!"));

				break;

			case (PC_PACKAGE_3):

				Serial.print(FC_TEMP_AN1);
				Serial.print(F(","));
				Serial.print(crc_item_1.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_1.i) / 10.0), 1);
 	      			Serial.print(F("!"));

				Serial.print(FC_TEMP_AN2);
				Serial.print(F(","));
				Serial.print(crc_item_2.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_2.i) / 10.0), 1);
 	      			Serial.print(F("!"));

				Serial.print(FC_TEMP_DGF);
				Serial.print(F(","));
				Serial.print(crc_item_3.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_3.i) / 10.0), 1);
 	      			Serial.print(F("!"));

				Serial.print(FC_TEMP_ICF);
				Serial.print(F(","));
				Serial.print(crc_item_4.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_4.i) / 10.0), 1);
 	      			Serial.print(F("!"));

				Serial.print(FC_VCC_ICF);
				Serial.print(F(","));
				Serial.print(crc_item_5.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_5.i) / 1000.0), 3);
 	      			Serial.print(F("!"));

				Serial.print(FC_RAM_ICF);
				Serial.print(F(","));
				Serial.print(crc_item_6.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_6.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_TEMP_I2C_3);
				Serial.print(F(","));
				Serial.print(crc_item_7.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_7.i) / 10.0), 1);
 	      			Serial.print(F("!"));


				break;

			case (PC_PACKAGE_4):

				Serial.print(FC_FAN_RPM_1);
				Serial.print(F(","));
				Serial.print(crc_item_1.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_1.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_FAN_RPM_2);
				Serial.print(F(","));
				Serial.print(crc_item_2.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_2.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_FAN_RPM_3);
				Serial.print(F(","));
				Serial.print(crc_item_3.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_3.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_FAN_RPM_4);
				Serial.print(F(","));
				Serial.print(crc_item_4.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_4.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_FAN_RPM_5);
				Serial.print(F(","));
				Serial.print(crc_item_5.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_5.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_FAN_RPM_6);
				Serial.print(F(","));
				Serial.print(crc_item_6.i);
				Serial.print(F(","));
	      			Serial.print(float(data_item_6.i), 0);
 	      			Serial.print(F("!"));

				Serial.print(FC_TEMP_I2C_4);
				Serial.print(F(","));
				Serial.print(crc_item_7.i);
				Serial.print(F(","));
	      			Serial.print((float(data_item_7.i) / 10.0), 1);
 	      			Serial.print(F("!"));

				break;
		}
	}
}


