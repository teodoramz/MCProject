#include "Adc.h"
#include "Uart.h"
#include "gpio.h"

int main() {
	
	UART0_Init(115200);
	ADC0_Init();
	RGBLed_Init();
	
	for(;;) {
		
	}
}
