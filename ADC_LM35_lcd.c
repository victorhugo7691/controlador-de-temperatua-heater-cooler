/*
 *  TEMPERATURA RA2
 */

#define _XTAL_FREQ 4000000    // Oscilador de 4 MHz

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.intermed.h"

#pragma config FOSC = HS        // Oscilador interno
#pragma config WDT = OFF        // Watchdog Timer desligado
#pragma config MCLRE = OFF      // Master Clear desabilitado

int contador, tempdisp;
char temperature[8];

float temperatura, tensao;


void setupADC(void) {
    
    TRISA = 0b00000111;         // Habilita pino A0-A2 como entrada
    
    ADCON1 = 0b00001100;        // pinos analogicos e referência
    
    ADCON2bits.ADCS = 0b110;    // Clock do AD: Fosc/64
    ADCON2bits.ACQT = 0b100;    // Tempo de aquisição: 8 Tad
    ADCON2bits.ADFM = 0b1;      // Formato: à direita
   
    ADCON0 = 0;                 
    ADCON0bits.CHS = 0b0010;     // Seleciona o canal AN2   
    ADCON0bits.ADON = 1;        // Liga o AD
}

void __interrupt() isr(void){// Função de interrupção
    //INTCON registrador
    if(INTCON3bits.INT1IF == 1) {
        
        INTCON3bits.INT1IF = 0;
       //Interrupção externa    
        tensao = 1;
    }
    
    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF) {
        /*Interrupção de por temporizador (TIMER) 
         bit 5 do registrador INTCON
         * TMR0IF informa a ocorrencia do overflow, bit 2 do registrador INTCON
         */
        if(temperatura >= 50 || temperatura <= 30){
            //Alerta
        }
        
		TMR0L = 5; //'recarga do timer0
        INTCONbits.TMR0IF = 0; // clear this interrupt condition
    }
            
}

void main(void) {
    
     TRISD = 0b00000000;         // Habilita porta D como saída dado LCD  
     TRISE = 0b00000000;        // porta E saida controle do LCD
     TRISC = 0b00000000;
     PORTC = 0b00100100;
     
    
     setupADC();
     
     ADCON1 = 15;
    
        comando_lcd(0b00111100);
        comando_lcd(0b00001100);
        limpa_lcd( );
    
while(1) {                                // Inicia loop infinito
		
       
        ADCON1 = 0b00001100;
        
        ADCON0bits.GO = 1;                    // Inicia a conversão A/D
		while (ADCON0bits.GO) {          // Aguarda fim da conversão
		}
		contador = (ADRESH * 0x100) + ADRESL;			// Transfere valor para variável
		tensao = ((5 * contador) * 0.001 );		// Calcula tensao real (/ 1024)
        temperatura = 100*tensao; // calcula temperatura  
        tempdisp = temperatura;
        
        __delay_ms(1000); //Atraso de 1s do display
        
        TRISD = 0; // Configura a porta D como sáida

        //ADCON1 = 15;
    
       // comando_lcd(0b00111100);
        //comando_lcd(0b00001100);
        //limpa_lcd( );
         
        comando_lcd(128);
        imprime_lcd("temperatura");
        
        comando_lcd(192);
        sprintf(temperature, "%3.2f", temperatura);
        
        imprime_lcd(temperature);       
        
         imprime_lcd(" graus Celsius");
        //__delay_ms(5000);
         
	}

    return;
}

