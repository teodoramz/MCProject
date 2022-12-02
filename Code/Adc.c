#include "Adc.h"
#include "Uart.h"
#include <math.h>

#define ROTATION_SENSOR_CH (13) // PORT C PIN 2
#define SOUND_SENSOR_CH (11) // PORT B PIN 3


#define RED_LED_PIN (18) // PORT B
#define GREEN_LED_PIN (19) // PORT B
#define BLUE_LED_PIN (1) // PORT D


uint16_t lastValue;

void ADC0_Init() {
	
	// Activarea semnalului de ceas pentru modulul periferic ADC
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
	
	// Functia de calibrare
	ADC0_Calibrate();
	
	ADC0->CFG1 = 0x00;

	// Selectarea modului de conversie pe 16 biti single-ended --> MODE
	// Selectarea sursei de ceas pentru generarea ceasului intern --> ADICLK
	// Selectarea ratei de divizare folosit de periferic pentru generarea ceasului intern --> ADIV
	// Set ADC clock frequency fADCK less than or equal to 4 MHz (PG. 494)
	ADC0->CFG1 |= ADC_CFG1_MODE(3) |
							 ADC_CFG1_ADICLK(0) |
							 ADC_CFG1_ADIV(2);
	
	// DIFF = 0 --> Conversii single-ended (PG. 464)
	ADC0->SC1[0] = 0x00;
	ADC0->SC3 = 0x00;

	// Selectarea modului de conversii continue, 
	// pentru a-l putea folosi in tandem cu mecanismul de intreruperi
	ADC0->SC3 |= ADC_SC3_ADCO_MASK;
	
	// Activarea subsistemului de conversie prin aproximari succesive pe un anumit canal (PG.464)
	ADC0->SC1[0] |= ADC_SC1_ADCH(SOUND_SENSOR_CH);
	lastValue = 0;
	
	// Enables conversion complete interrupts
	ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;
	
	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);	
}


int ADC0_Calibrate() {
	
	// ===== For best calibration results =====
	
	ADC0_CFG1 |= ADC_CFG1_MODE(3)  |  				 // 16 bits mode
                ADC_CFG1_ADICLK(1)|  // Input Bus Clock divided by 2
                ADC_CFG1_ADIV(3);   // Clock divide by 8
	
	// The calibration will automatically begin if the SC2[ADTRG] is 0. (PG. 495)
	ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK;
	
	// Set hardware averaging to maximum, that is, SC3[AVGE]=1 and SC3[AVGS]=0x11 for an average of 32 (PG. 494)
	ADC0->SC3 |= (ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3));
	
	// To initiate calibration, the user sets SC3[CAL] (PG. 495)
	ADC0->SC3 |= ADC_SC3_CAL_MASK;
	
	// At the end of a calibration sequence, SC1n[COCO] will be set (PG. 495)
	while(!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));
	
	// At the end of the calibration routine, if SC3[CALF] is not
	// set, the automatic calibration routine is completed successfully. (PG. 495)
	if(ADC0->SC3 & ADC_SC3_CALF_MASK){
		return (1);
	}
	
	// ====== CALIBRATION FUNCTION (PG.495) =====
	
	// 1. Initialize or clear a 16-bit variable in RAM.
	uint16_t calibration_var = 0x0000;
	
	// 2. Add the plus-side calibration results CLP0, CLP1, CLP2, CLP3, CLP4, and CLPS to the variable.
	calibration_var += ADC0->CLP0;
	calibration_var += ADC0->CLP1;
	calibration_var += ADC0->CLP2;
	calibration_var += ADC0->CLP3;
	calibration_var += ADC0->CLP4;
	calibration_var += ADC0->CLPS;
	
	// 3. Divide the variable by two.
	calibration_var /= 2;
	
	// 4. Set the MSB of the variable. 
	calibration_var |= 0x8000;
	
	// 5. Store the value in the plus-side gain calibration register PG.
	ADC0->PG = ADC_PG_PG(calibration_var);
	
	// 6. Repeat the procedure for the minus-side gain calibration value.
	calibration_var = 0x0000;
	
	calibration_var += ADC0->CLM0;
	calibration_var += ADC0->CLM1;
	calibration_var += ADC0->CLM2;
	calibration_var += ADC0->CLM3;
	calibration_var += ADC0->CLM4;
	calibration_var += ADC0->CLMS;
	
	calibration_var /= 2;
	
	calibration_var |= 0x8000;
	
	ADC0->MG = ADC_MG_MG(calibration_var);
	
	// Incheierea calibrarii
	ADC0->SC3 &= ~ADC_SC3_CAL_MASK;
	
	return (0);
}



void SND_SensorRez(int limit){
	
	int PIN_COLOR;
	uint16_t analog_input;
	double dbValue;
	int i;
	char v[16];
	int count = 0;
	uint16_t c;
	
	//configurare canal senzor de sunet
	ADC0->SC1[0] = 0x00;
	ADC0->SC3 = 0x00;
	ADC0->SC3 |= ADC_SC3_ADCO_MASK;
	ADC0->SC1[0] |= ADC_SC1_ADCH(SOUND_SENSOR_CH);
	ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;
	
	
	//punerea pe 0 logic  a ledurilor verde si rosu
	GPIOB_PSOR |= (1<<RED_LED_PIN);
	GPIOB_PSOR |= (1<<GREEN_LED_PIN);
	

	//citirea valorii transmise de senzor
	analog_input = (uint16_t) ADC0->R[0];
	
	// conversia valorii primite in decibeli
	dbValue = log10(analog_input)*(double)20;
	
	//rotunjirea acesteia
	int value = (int) dbValue;
	
	
	// configurarea zonelor verde - valori mici, galben - valori medii, rosu - valori mari
	if(value < 40){
				PIN_COLOR = 0;
	}
	if(value >= 40 && value < 80)
	{
		PIN_COLOR = 1;
	}
	if(value >= 80 && value < 120)
	{
		PIN_COLOR = 2;
	}
	
	
	// daca senzorul detecteaza mai mult decat limita admisa, aprinde un led
	if(value > limit){
		
		//aprinderea este in functie de zona de culoare corespunzatoare
		if(PIN_COLOR == 0){ 
			GPIOB_PTOR |= (1<<GREEN_LED_PIN);
		}
		if(PIN_COLOR == 1){ 
			GPIOB_PTOR |= (1<<GREEN_LED_PIN);
			GPIOB_PTOR |= (1<<RED_LED_PIN);
		}
		if(PIN_COLOR == 2){ 
			GPIOB_PTOR |= (1<<RED_LED_PIN);
		}
	}
	else{
		//stingerea ledurilor daca valoarea inregistrata de senzor este sub limita
		if(PIN_COLOR == 0){ 
				GPIOB_PSOR |= (1<<GREEN_LED_PIN);
		}
		if(PIN_COLOR == 1){ 
			  GPIOB_PSOR |= (1<<GREEN_LED_PIN);
				GPIOB_PSOR |= (1<<RED_LED_PIN);
		}
		if(PIN_COLOR == 2){ 
				GPIOB_PSOR |= (1<<RED_LED_PIN);
		}
	}
	
	
	
	
	//afisarea valorii obtinute
	
	for(i = 0; i < 16; i++){
		v[i] = '-';
	}
	
	
	UART0_Transmit('S');
	UART0_Transmit('N');
	UART0_Transmit('D');
	UART0_Transmit(':');
	
	
	while(value!=0){
		v[count] = value%10 + 0x30;
		value = value/10;
		count = count + 1;
	}

	for(i = 0; i < count ; i++){
		UART0_Transmit(v[count - i - 1]);
	}
	UART0_Transmit(0x0A);
	UART0_Transmit(0x0D);
}

uint16_t ROT_SensorRez(){
	
	int count = 0;
	uint16_t c;
	int i;
	char v[16];
	uint16_t aux;
	
	
	//configurare canal senzor de rotatie
	ADC0->SC1[0] = 0x00;
	ADC0->SC3 = 0x00;
	ADC0->SC3 |= ADC_SC3_ADCO_MASK;
	ADC0->SC1[0] |= ADC_SC1_ADCH(ROTATION_SENSOR_CH);
	ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;
	
	//citirea valorii transmise de senzor
	uint16_t analog_input = (uint16_t) ADC0->R[0];

	//salvare valoare transmisa intr-o variabila auxiliara
	lastValue = analog_input;
	
	
	 
	//afisarea valorii obtinute
	
	for(i = 0; i < 16; i++){
		v[i] = '-';
	}
	
	UART0_Transmit('R');
	UART0_Transmit('O');
	UART0_Transmit('T');
	UART0_Transmit(':');
	
	while(analog_input!=0){
		v[count] = analog_input%10 + 0x30;
		analog_input = analog_input/10;
		count = count + 1;
	}
	
	for(i = 0; i < count ; i++){
		UART0_Transmit(v[count - i - 1]);
	}

	UART0_Transmit(0x0A);
	UART0_Transmit(0x0D);
	
	//transpun valoarea de la senzorul de rotatie in intervalul 0 - 120db
	uint16_t rezult = (lastValue * 120)/65535;
	
	
	
	return rezult;
}

void ADC0_IRQHandler(){
		
		int limit;
	
		limit = ROT_SensorRez();
		
		SND_SensorRez(limit);
	
}
