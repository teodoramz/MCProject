#include "MKL25Z4.h"

extern uint16_t lastValue;

void ADC0_Init(void);
int ADC0_Calibrate(void);
void ADC0_IRQHandler(void);
void SND_SensorRez(int limit);
uint16_t ROT_SensorRez(void);
