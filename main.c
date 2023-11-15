/*
 * tde4-5.c
 *
 * Created: 11/15/2023 12:05:10 AM
 * Author : joao.moreti
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <util/delay.h> 


// BEGIN: declarando funções
void configPORT();
void configUSART();
void configAD();
int lerAD();
void ajustar_dados_AD(int valorAD, char *msg);
void decodificarBDC();
void enviar_dados_display(char msg);
void myPrintf(char* msg);
// END: declarando funções

// BEGIN: sequencia de execucao
int main(void)
{
	// configurcoes
	configPORT();
	configAD();
	configUSART();
    
	// variaveis
    char msg[20];

    while (1) {
	    int valorAD = lerAD();

	    ajustar_dados_AD(valorAD, msg);
	    myPrintf(msg); // warning1
	    _delay_ms(100);
    }
}
// END: sequencia de execucao

// BEGIN: definindo funcoes

void configPORT() {
	DDRD = 0b01111111; // PD0:7 --> input display 7 seg
	DDRB = 0b00001111; // PD0:3 --> input uln2003apg
}


void configUSART() {
	UBRR0 = 103; //
	UCSR0B = 0b00011100; // RXEN0 (bit 4) e TXEN0 (bit 3) --> rebimento e transmissao de dados habilitada
						// UCSZn2 (bit 2) = 1 configuracao em e UCSZn1:0 bit in UCSR0C
	UCSR0C = 0b00000110; // config para 8 bits de dados
}

void configAD() {
	ADMUX = 0b01100000; // bit 7:6 REFS[1:0] --> Vref = AVCC * colocar capacitor em AREF *
						// bit 3:0 MUX[3:0] --> ADC0
						// bit 5 ADLAR --> left adjust
	ADCSRA = 0b10000111; // bit 7 ADEN --> habilita AD 
					 // bit 6 ADSC --> inicia conversao AD (em 0 na configuracao)
					 // bits 2:0 ADPS[2:0] --> PRESCALER = 128
}

int lerAD() {
	ADCSRA = 0b11000000; // bit 6 ADSC = 1 --> inicia conversao AD
	while (ADCSRA & (1 << ADSC)); // bit 6 ADSC = 0 --> quando em zero quer dizer que terminou a conversao
	return(ADC);
}

void ajustar_dados_AD(int valorAD, char *msg) {
	msg[0] = valorAD / 1000 + 0x30;
	msg[1] = valorAD % 1000 / 100 + 0x30;
	msg[2] = valorAD % 1000 % 100 / 10 + 0x30;
	msg[3] = valorAD % 1000 % 100 % 10 + 0x30;
	msg[4] = '\n';
	msg[5] = '\r';
}


void myPrintf(char* msg) // warning2
{
	int tamanho = 6;
	
	int index = 0;
	while(index != tamanho)
	{
		UDR0 = msg[index];
		index++;
		_delay_ms(10);
	}
}

	

// END: definindo funcoes