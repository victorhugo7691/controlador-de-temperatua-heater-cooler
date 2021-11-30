
#include <string.h>

#define RS PORTEbits.RE2
#define EN PORTEbits.RE1

void comando_lcd(unsigned int comando)
{
    RS = 0;
    EN = 1;
    PORTD = comando;
    __delay_ms(2);
    EN = 0;
}

void dado_lcd(unsigned int dado)
{
    RS = 1;
    EN = 1;
    PORTD = dado;
    __delay_ms(2);
    EN = 0;
  
 }

void limpa_lcd( )
{
    comando_lcd(1);
}

void imprime_lcd(char frase[])
{
int indice = 0;
int tamanho = strlen(frase);
while(indice<tamanho)
    {
dado_lcd(frase[indice]);
indice++;
    }
}