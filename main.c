#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xc.h>
#include "LCD_picdem2_2006_XC8.c"
#include "my_delay.c"

#pragma config WDT = OFF //desactiva watchdog timer
#pragma config OSC = HS //HS oscilador highspeed
#pragma config LVP=OFF //desactiva low voltage program

#define CLOCK 0
#define STOPWATCH 1

#define SET 0
#define SECONDS 1
#define MINUTES 2
#define HOUR 3

volatile int hour = 0, minutes = 0, seconds = 0;
volatile int swHour = 0, swMinutes = 0, swSeconds = 0;
volatile int state = 0, state_button1 = 0, state_button2 = 0;
volatile int set_stopwatch = 0, set = 0;
volatile int counter = 0;


char clockSTR[16];
char stopWatchSTR[16];


void interrupt_high(void){
    if(INTCONbits.INT0IE && INTCONbits.INT0IF){
        if (!T0CONbits.TMR0ON && !set){
            switch(state){
                case CLOCK:
                    //LCD_display(1,1,"                ");
                    state = 1;
                    counter = 0;
                    INTCONbits.INT0IF = 0;
                    break;
                case STOPWATCH:
                    LCD_display(1,1,"                ");
                    state = 0;
                    counter = 0;
                    INTCONbits.INT0IF = 0;
                    break;
            }
        }
        INTCONbits.INT0IF = 0; 
    }
    
    else if(INTCON3bits.INT2IE && INTCON3bits.INT2IF){
            switch(state_button2){
                case SET:
                    switch(state_button1){
                        case 0:
                            T0CONbits.TMR0ON = 1;
                            WriteTimer0(59286);
                            switch(state){
                                case CLOCK:
                                    counter = 0;
                                    break;
                                case STOPWATCH:
                                    if (!set_stopwatch){
                                        swHour = 0, swMinutes = 0, swSeconds = 0, counter = 0;
                                    }
                                    else{
                                       counter = 0, set_stopwatch = 0;
                                    }
                                    break;
                            }
                            state_button1 = 1;
                            break;
                        case 1:
                            T0CONbits.TMR0ON = 0;
                            state_button1 = 0;
                            break;
                    }
                    break;
                case SECONDS:
                    switch(state){
                        case CLOCK:
                            seconds++;
                            if(seconds == 60){
                                seconds = 0;    
                            } 
                            break;
                        case STOPWATCH:
                            swSeconds++;
                            if(swSeconds == 60){
                                swSeconds = 0;
                            }
                            break;
                    }
                    break;
                case MINUTES:
                    switch(state){
                        case CLOCK:
                            minutes++;
                            if(minutes == 60){
                                minutes = 0;
                            }
                            break;
                        case STOPWATCH:
                            swMinutes++;
                            if(swMinutes == 60){
                                swMinutes = 0;
                            }
                            break;
                    }
                    break;
                case HOUR:
                    switch(state){
                        case CLOCK:
                            hour++;
                            if(hour == 24){
                                hour = 0;   
                            }
                            break;
                        case STOPWATCH:
                            swHour++;
                            if(swHour == 100){
                                swHour = 0;
                            }
                            break;
                    }
                    break;
        }
        INTCON3bits.INT2IF = 0;
    }
    
    else if(INTCON3bits.INT1IE && INTCON3bits.INT1IF){
        if (!T0CONbits.TMR0ON){
            switch(state_button2){
                case SET:
                    state_button2 = 1;
                    set = 1;
                    break;
                case SECONDS:
                    switch(state){
                        case CLOCK:
                            break;
                        case STOPWATCH:
                            set_stopwatch = 1;
                            break;
                    }
                    state_button2 = 2;
                    break;
                case MINUTES:
                    switch(state){
                        case CLOCK:
                            break;
                        case STOPWATCH:
                            set_stopwatch = 1;
                            break;
                    }
                    state_button2 = 3;
                    break;
                case HOUR:
                    switch(state){
                        case CLOCK:
                            break;
                        case STOPWATCH:
                            set_stopwatch = 1;
                            break;
                    }
                    state_button2 = 0;
                    set = 0;
                    break;
            }  
        }
        INTCON3bits.INT1IF = 0;
    }
    
    else if (INTCONbits.TMR0IF && INTCONbits.TMR0IE){
        counter++;
        WriteTimer0(59286); 
        if (counter == 10){
            switch(state){
                case CLOCK:
                    seconds++;
                    if(hour == 23 && minutes == 59 && seconds > 59){
                            hour = 0;
                            minutes = 0;
                            seconds = 0;
                            LCD_display(2,3,"00");
                            LCD_display(2,6,"00");
                            LCD_display(2,9,"00");
                    }
                    else if (minutes == 59 && seconds > 59){
                            hour++;
                            minutes = 0;
                            seconds = 0;
                            LCD_display(2,3,"00");
                            LCD_display(2,6,"00");
                    }
                    else if (seconds > 59){
                            minutes++;
                            seconds = 0;
                            LCD_display(2,3,"00");
                    }
                    break;
                case STOPWATCH:
                    swSeconds++;
                    if(swHour == 99 && swMinutes == 59 && swSeconds > 59){
                            swHour = 0;
                            swMinutes = 0;
                            swSeconds = 0;
                            LCD_display(2,3,"00");
                            LCD_display(2,6,"00");
                            LCD_display(2,9,"00");
                    }
                    else if (swMinutes == 59 && swSeconds > 59){
                            swHour++;
                            swMinutes = 0;
                            swSeconds = 0;
                            LCD_display(2,3,"00");
                            LCD_display(2,6,"00");
                    }
                    else if (swSeconds > 59){
                            swMinutes++;
                            swSeconds = 0;
                            LCD_display(2,3,"00");
                    }
                    break;
            }
           counter = 0;
        }
        
        INTCONbits.TMR0IF = 0;
    }
}

void interrupt high_isr(void){
    interrupt_high();
}

void main(void){
    //T0CON = 0b10000011;
    //Ativar timer0
    RCONbits.IPEN = 1;//Ativa prioridade de interrupção rb1(INT1) e rb2(INT2)
    INTCONbits.GIEH = 1;//Ativa interrupções a nivel global
    INTCONbits.GIEL = 1;//ativa interrupções a nivel dos perifericos
    INTCONbits.TMR0IF = 0;//reinicia a flag do timer 0 (TMR0)
    INTCONbits.TMR0IE = 1;//Força a desativação do TMR0
    INTCON2bits.TMR0IP = 1;//Ativação de interrupção de overflow do timer
    
    //ativar interrupção 0(INT0)
    ADCON1 =0x0F;//define todos portos analogicos para digitais
    INTCONbits.INT0IE = 1;//ativa interrupção 0(INT0)
    INTCONbits.INT0IF = 0;//Força o reinicio da flag do INT0
    INTCON2bits.INTEDG0 = 0;//define se a leitura do botao ocorre na descida ou na subida
    
    INTCON3bits.INT1IE = 1;//ativa interrupção 0(INT0)
    INTCON3bits.INT1IF = 0;//Força o reinicio da flag do INT0
    INTCON2bits.INTEDG1 = 0;//define se a leitura do botao ocorre na descida ou na subida
    
    INTCON3bits.INT2IE = 1;//ativa interrupção 0(INT0)
    INTCON3bits.INT2IF = 0;//Força o reinicio da flag do INT0
    INTCON2bits.INTEDG2 = 0;//define se a leitura do botao ocorre na descida ou na subida
    
    //configuração do timer
    T0CONbits.TMR0ON = 0;//Desativa o timer
    T0CONbits.T08BIT = 0;//configuração do tamanho do relogio 8bits ou 16bits
    T0CONbits.T0CS = 0;//Seleção da entrada do relogio
    T0CONbits.T0SE = 0;//Nao Aplicavel nesta situação
    T0CONbits.PSA = 0;//Ativa ou desativa o prescaler
    T0CONbits.T0PS2 = 0;
    T0CONbits.T0PS1 = 1;//configura o valor a usar no prescaler
    T0CONbits.T0PS0 = 1;
    
    LCD_init(); // inicia a comunicação com o LCD
    LCD_Clear(); // limpa o lcd
    
    WriteTimer0(59286);//define a contagem para começar a 59286

    while(1){
        switch(state_button2){
            case SET:
                LCD_normal();
                    switch(state){
                        case CLOCK:
                            LCD_display(1,3,"CLOCK");
                            LCD_display(2,5,":");//7
                            LCD_display(2,8,":");//10
                            LCD_display(2,11,":");//13
                            break;
                        case STOPWATCH:
                            LCD_display(1,3,"STOPWATCH");
                            LCD_display(2,5,":");//7
                            LCD_display(2,8,":");//10
                            LCD_display(2,11,":");//13
                            break;
                    }
                    switch(state){
                        case CLOCK:
                            if(hour < 10){
                                itoa(clockSTR, hour, 10);
                                LCD_display(2,3,"0");
                                LCD_display(2,4,clockSTR);
                            }
                            else{
                                itoa(clockSTR, hour, 10);
                                LCD_display(2,3,clockSTR);
                            }

                            if(minutes < 10){
                                itoa(clockSTR, minutes, 10);
                                LCD_display(2,6,"0");
                                LCD_display(2,7,clockSTR);
                            }
                            else{
                                itoa(clockSTR, minutes, 10);
                                LCD_display(2,6,clockSTR);
                            }

                            if(seconds < 10){
                                itoa(clockSTR, seconds, 10);
                                LCD_display(2,9,"0");
                                LCD_display(2,10,clockSTR);
                            }
                            else{
                                itoa(clockSTR, seconds, 10);
                                LCD_display(2,9,clockSTR);
                            }

                            itoa(clockSTR, counter, 10);
                            LCD_display(2,12,clockSTR);

                            break;
                        case STOPWATCH:
                            if(swHour < 10){
                                itoa(clockSTR, swHour, 10);
                                LCD_display(2,3,"0");
                                LCD_display(2,4,clockSTR);
                            }
                            else{
                                itoa(clockSTR, swHour, 10);
                                LCD_display(2,3,clockSTR);
                            }

                            if(swMinutes < 10){
                                itoa(clockSTR, swMinutes, 10);
                                LCD_display(2,6,"0");
                                LCD_display(2,7,clockSTR);
                            }
                            else{
                                itoa(clockSTR, swMinutes, 10);
                                LCD_display(2,6,clockSTR);
                            }

                            if(swSeconds < 10){
                                itoa(clockSTR, swSeconds, 10);
                                LCD_display(2,9,"0");
                                LCD_display(2,10,clockSTR);
                            }
                            else{
                                itoa(clockSTR, swSeconds, 10);
                                LCD_display(2,9,clockSTR);
                            }

                            itoa(clockSTR, counter, 10);
                            LCD_display(2,12,clockSTR);
                            break;
                    }
                break;
            case SECONDS:
                switch(state){
                    case CLOCK:
                        if (seconds < 10)
                            LCD_display(2,10," ");
                        LCD_blink_position(2,10);
                        delay(50);
                        itoa(clockSTR, seconds, 10);
                        LCD_display(2,9,clockSTR);
                        break;
                    case STOPWATCH:
                        if (swSeconds < 10)
                            LCD_display(2,10," ");
                        LCD_blink_position(2,10);
                        delay(50);
                        itoa(clockSTR, swSeconds, 10);
                        LCD_display(2,9,clockSTR);
                        break;
                }
                
                break;
            case MINUTES:
                switch(state){
                    case CLOCK:
                        if (minutes < 10)
                            LCD_display(2,7," ");
                        LCD_blink_position(2,7);
                        delay(50);
                        itoa(clockSTR, minutes, 10);
                        LCD_display(2,6,clockSTR);
                        break;
                    case STOPWATCH:
                        if (swMinutes < 10)
                            LCD_display(2,7," ");
                        LCD_blink_position(2,7);
                        delay(50);
                        itoa(clockSTR, swMinutes, 10);
                        LCD_display(2,6,clockSTR);
                        break;
                }
                
                break;
            case HOUR:
                switch(state){
                    case CLOCK:
                        if (hour < 10)
                            LCD_display(2,4," ");
                        LCD_blink_position(2,4);
                        delay(50);
                        itoa(clockSTR, hour, 10);
                        LCD_display(2,3,clockSTR);
                        break;
                    case STOPWATCH:
                        if (swHour < 10)
                            LCD_display(2,4," ");
                        LCD_blink_position(2,4);
                        delay(50);
                        itoa(clockSTR, swHour, 10);
                        LCD_display(2,3,clockSTR);
                        break;
                }
                break;
        }    
    }
}

