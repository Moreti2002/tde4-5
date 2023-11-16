/*
 * tde4-5.c
 *
 * Created: 11/15/2023 12:05:10 AM
 * Author : joao.moreti
 */ 

#define F_CPU 16000000 
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <util/delay.h>

char caractere = 0;
void myPrintf(char* msg, int tamanho);
void ConfigAD(void);
int CapturaAD(void);
void SelCanalAd(char canal);
int decimalPara7Segmentos(int digito);
void escreverBCD();

void mostrar_degrau(int habilita_display[], int valorAD);
void mostrar_volts(int habilita_display[], int valorAD);
int estado_botoes();
void configTimer();
void mostrarContagem(int numero, int habilita_display[]);

/* ======================== VARIAVEIS GLOBAIS ============================ */
int contagem = 0;

/* ================================================================= */

/* ======================== INTERRUPCOES ============================ */
// interrupcao de comparacao
ISR(TIMER1_COMPA_vect) {
	contagem++;
}


/* ================================================================= */


// -------------------------------------------------------------------
int main(void)
{
	int valorAD = 0;

	// -------------------------------------------------------------------
	DDRD = 0b01111111;
	DDRB = 0b00001111;

	ConfigAD();
	SelCanalAd(0);
	configTimer();
	
	while (1) {
		valorAD = CapturaAD();
		
		int habilita_display[] = {0b00001000, 0b00000100, 0b00000010, 0b00000001}; // ML, CT, DZ, UN
			
		int estado = estado_botoes();
		
		switch(estado) {
			case 1:
			// mostrar em volts
				mostrar_volts(habilita_display, valorAD);
				//PORTB = habilita_display[0];
				//PORTD = decimalPara7Segmentos(1);
				break;
			case 2:
			// mostrar em degrau
				mostrar_degrau(habilita_display, valorAD);
				//PORTB = habilita_display[1];
				//PORTD = decimalPara7Segmentos(1);
				break;
			case 3:
			// TIMER 0000 - 9999
				if (contagem <= 9999) {
					mostrarContagem(contagem, habilita_display); // Exibe a contagem nos displays
					} else {
					contagem = 0; // Reseta a contagem para reiniciar de 0000
				}
				//PORTB = habilita_display[2];
				//PORTD = decimalPara7Segmentos(1);
				break;
			
			case 4:
			// TIMER 5959 - 0000
				PORTB = habilita_display[3];
				PORTD = decimalPara7Segmentos(1);
				break;
			
		}
	}
}

void mostrar_degrau(int habilita_display[], int valorAD) {
	
	int ML = valorAD / 1000;
	int CT = valorAD % 1000 / 100;
	int DZ = valorAD % 1000 % 100 / 10;
	int UN = valorAD % 1000 % 100 % 10;
	
	int magnitude[] = {ML, CT, DZ, UN};
	
	for (int j = 0; j < 4; j++) {
		PORTB = 0b00000000;
		_delay_us(5);
		for (int i = 0; i < 250; i++) { // 10*10^(-6) * 250 = 2,5 ms --> f = 1/s = 1KHz
			PORTB = habilita_display[j];
			PORTD = decimalPara7Segmentos(magnitude[j]);
			_delay_us(10);
		}
	}
}

void mostrar_volts(int habilita_display[], int valorAD) {
	float Volts = valorAD*(5.0/1024.0); // 5v dividido para cada degrau. Total de degrau = 2^(10) = 1024
	

    int ML = (int)Volts; // Parte inteira
    int unidades_decimal = (int)((Volts - ML) * 1000); // removendo parte inteira e isolando parte decimal

    int CT = unidades_decimal / 100;
    int DZ = (unidades_decimal % 100) / 10;
    int UN = unidades_decimal % 10;
	
	int magnitude_volts[] = {ML, CT, DZ, UN};
	
	for (int j = 0; j < 4; j++) {
		PORTB = 0b00000000;
		_delay_us(5);
		for (int i = 0; i < 250; i++) { // 10*10^(-6) * 250 = 2,5 ms --> f = 1/s = 1KHz
			PORTB = habilita_display[j];
			PORTD = decimalPara7Segmentos(magnitude_volts[j]);
			_delay_us(10);
		}
	}	
}


int estado_botoes() {
	int bt1 = PINB & (1 << PINB5); // botao branco 
	int bt2 = PINB & (1 << PINB4); // botao amarelo
	
	if (bt1 == 0b00000000) {
		// Conversor AD
		if (bt2 == 0b00000000) {
			// mostra em Volts
			return 1;
		}
		// mostrar em degrau
		return 2;
	} else {
		// TIMER
		if (bt2 == 0b00000000) {
			// TIMER 0000 ate 9999
			return 3;
		}
		return 4;

	}
}

void ConfigAD(void)
{
	ADMUX = 0b01000000; // Vref = Avcc; ADLAR = 0; MUX =0 - para 10 bits de conversor
	ADCSRA = 0b10000111; //ADEN = 1; PRESCALER 128
}
void SelCanalAd(char canal)
{
	canal = 0b11100000 || canal;
	ADMUX = ADMUX & canal; // Vref = Avcc; ADLAR = 0; MUX =0
	_delay_us(10); // Tempo de adequação das chaves e capacitores internos
}
int CapturaAD(void)
{
	ADMUX = 0b01000000;
	ADCSRA = ADCSRA | 0b01000000; //ADEN = 1; ADSC = 1 para iniciar a conversão
	while (ADCSRA & 0b01000000); // aguarda o bit ADSC retornar a 0 indicando fim de conversão
	return(ADC);
}

int decimalPara7Segmentos(int digito) {
	int mapa[] = {
		0b00111111, // 0
		0b00000110, // 1
		0b01011011, // 2
		0b01001111, // 3
		0b01100110, // 4
		0b01101101, // 5
		0b01111101, // 6
		0b00000111, // 7
		0b01111111, // 8
		0b01101111  // 9
	};


	return mapa[digito]; // valor correspondente ao digito

}

void configTimer() {
	
    TCCR1B = 0b00001101; // Modo CTC e PRESCALER = 1024
    OCR1A = 15625; // Valor para 1 segundo de contagem (16MHz / 1024)
    TIMSK1 = TIMSK1 = 0b00000010;; // interrupção por comparacao A habilitada
    sei(); // habilitando interrupcoes
	
}

void mostrarContagem(int numero, int habilita_display[]) { 
		int ML = numero / 1000;
		int CT = (numero % 1000) / 100;
		int DZ = (numero % 100) / 10;
		int UN = numero % 10;
		
		int magnitude_numero[] = {ML, CT, DZ, UN};
		
		for (int j = 0; j < 4; j++) {
			PORTB = 0b00000000;
			_delay_us(5);
			for (int i = 0; i < 250; i++) { // 10*10^(-6) * 250 = 2,5 ms --> f = 1/s = 1KHz
				PORTB = habilita_display[j]; // ERRO1
				PORTD = decimalPara7Segmentos(magnitude_numero[j]);
				_delay_us(10);
			}
		}		
}