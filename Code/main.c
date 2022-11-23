#include "Adc.h"
#include "Uart.h"

int main() {
	
	UART0_Init(115200);
	ADC0_Init();
	
	for(;;) {

	}
	
}
