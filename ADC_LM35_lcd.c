/*
 * Controle de temperatura utilizando cooler, heater
 */
#define _XTAL_FREQ 4000000    // Oscilador de 4 MHz

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.intermed.h"

#pragma config FOSC = HS        // Oscilador interno
#pragma config WDT = OFF        // Watchdog Timer desligado
#pragma config MCLRE = ON      // Master Clear desabilitado

int contador, tempdisp, contadorInt;
char temperature[8];
float temperatura;
float tensao, velocidadeCooler;
int temperaturaMax = 60;
int temperaturaMin  = 43;

void setupADC(void) {
    
    TRISA = 0b00000111;         // Habilita pino A0-A2 como entrada
    ADCON1 = 0b00001100;        // pinos analogicos e referência //configuração dos pinos que serão utilizados como entrada analógica através do registrador 
    
    ADCON2bits.ADCS = 0b110;    // Clock do AD: Fosc/64
    ADCON2bits.ACQT = 0b100;    // Tempo de aquisição: 8 Tad
    ADCON2bits.ADFM = 0b1;      // Formato: à direita
   
    ADCON0 = 0;                 
    ADCON0bits.CHS = 0b0010;     // Seleciona o canal AN2   
    ADCON0bits.ADON = 1;        // Liga o AD
}

void controlaVelocidade(){  // Incrementa ou decrementa o ciclo em relação do cooler
    if(temperatura < 50){
        CCPR1L = CCPR1L - 30;
    }
    
    if(temperatura > 60){
        CCPR1L = CCPR1L + 80;
    } else if(temperatura >= 55){
        CCPR1L = CCPR1L + 5;
    }
}
void alarmeTemperatura(){
    if(temperatura > temperaturaMax){
        limpa_lcd( );
        __delay_ms(1000);
        comando_lcd(128);
        imprime_lcd("PERIGO ALTA");
        comando_lcd(192);
        imprime_lcd("TEMPERATURA");
        __delay_ms(1000);
        limpa_lcd( );
    }
    if(temperatura < temperaturaMin){
        limpa_lcd( );
        __delay_ms(1000);
        comando_lcd(128);
        imprime_lcd("PERIGO BAIXA");
        comando_lcd(192);
        imprime_lcd("TEMPERATURA");
        __delay_ms(1000);
        limpa_lcd( );
    }
}

void __interrupt() isr(void){// Função de interrupção
    //INTCON registrador
    if(INTCON3bits.INT1IF == 1) { //Interrupção externa   
        
        INTCON3bits.INT1IF = 0;
        if(temperatura >= 20){ //Ao clicar na interrupção externa a temperatura será reduzida.
            temperatura = temperatura - 10;
        }
    }
    
    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF) {
        /*Interrupção de por temporizador (TIMER) 
         bit 5 do registrador INTCON
         * TMR0IF informa a ocorrencia do overflow, bit 2 do registrador INTCON
         */
        contadorInt++;
        if(contadorInt>=8){
            controlaVelocidade();
            contadorInt=0;
        }
        
        
		TMR0L = 7; //'recarga do timer0
        INTCONbits.TMR0IF = 0; // clear this interrupt condition
    }
            
}

void main(void) {
    temperatura = 48; //temperatura inicial
    PORTAbits.RA4 = 0; //'pino contador T0CKI configurado 

    INTCONbits.GIE =1; //Habilita interrupção global, bit 7 do registrador INTCON
    INTCONbits.TMR0IE = 1; //Habilita a interrupção do TMR0
    //Prescalar configurado para 1:128 O prescaler define o número de vezes que um evento deve ocorrer, antes que o registrador TMR0L seja incrementado
    T0CON = 0B11000110; //Configura o prescalar, o prescaler define o número de vezes que um evento deve ocorrer
    //tempo = 256x128x1 = 32,768 ms
    TMR0L = 7;  // 1x128x(256-7) =  31,872ms 
    
    TRISD = 0b00000000;         // Habilita porta D como saída dado LCD  
    TRISE = 0b00000000;        // porta E saida controle do LCD
    TRISC = 0b00000000;
    PORTC = 0b00100100; //estados do pino da porta
    ADCON1 = 15; //pinos analogicos e referência
    
    setupADC();
     
    TMR2 = 0x00;   // Começa a contar de 0
    PR2 = 250;     // Período do sinal PWM
    CCP1CON = 0b00001100;       //Ativa PWM 
    CCPR1L = 240;                   // Duty Cycle / registro do ciclo
    TMR2IF = 0;                     // Limpa flag do TMR2
    TRISCbits.RC2 = 0;              // "liga" bit de saída
    T2CON = 0b00000100;             // Liga timer 2
    
    INTCON3bits.INT1IE = 1;
    INTCON3bits.INT1IF = 0;
    INTCON2bits.INTEDG1 = 1;
    
    comando_lcd(0b00111100);
    comando_lcd(0b00001100);
    limpa_lcd( );
    
while(1) {                                // Inicia loop infinito
		
        ADCON1 = 0b00001100; //pinos analogicos e referência
        
        ADCON0bits.GO = 1;                    // Inicia a conversão A/D
		while (ADCON0bits.GO) {          // Aguarda fim da conversão
		}
		contador = (ADRESH * 0x100) + ADRESL;			// Transfere valor para variável
		tensao = ((5 * contador) * 0.001 );		// Calcula tensao real (/ 1024)
        temperatura = 100*tensao; // calcula temperatura  
        tempdisp = temperatura;
        
        __delay_ms(1000); //Atraso de 1s do display
        
        TRISD = 0; // Configura a porta D como sáida
        comando_lcd(128);
        imprime_lcd("Temperatura");
        
        comando_lcd(192);
        sprintf(temperature, "%3.2f", temperatura);
        
        imprime_lcd(temperature);       
        imprime_lcd(" Graus Celsius");
        __delay_ms(500); //Atraso de 1s do display
        alarmeTemperatura();
	}

    return;
}

