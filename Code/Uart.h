#include "MKL25Z4.h"

extern char c;
extern char buffer[32];

extern int write, read;
extern int print_buffer;
extern int full;


void UART0_Transmit(uint8_t data); // Functie folostia pentru a trimite un octet catre interfata UART
uint8_t UART0_receive(void); // Functie ce returneaza un octet de pe interfata UART, atunci cand acesta exista un buffer
void UART0_Init(uint32_t baud_rate); // Initializare a modulului UART pentru trasmiterea datelor, ce primeste ca parametru baud rate-ul dorit
void UART0_IRQHandler(void); // Handler-ul de intreruperi
