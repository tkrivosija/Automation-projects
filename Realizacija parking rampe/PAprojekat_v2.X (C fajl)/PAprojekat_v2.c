/*
 * File:   PAprojekat_v2.c
 * Author: User
 *
 * Created on March 1, 2021, 12:19 PM
 */


#pragma config FOSC=HS,WDTE=OFF,PWRTE=OFF,MCLRE=ON,CP=OFF,CPD=OFF,BOREN=OFF,CLKOUTEN=OFF
#pragma config IESO=OFF,FCMEN=OFF,WRT=OFF,VCAPEN=OFF,PLLEN=OFF,STVREN=OFF,LVP=OFF
#define _XTAL_FREQ 8000000
#define T RB0
#define Mgore RD0
#define Mdole RD1
#define IC RB1

#include <xc.h>

double ugao,napon;
char podizi = 0;
char spustaj = 0;
int sekunde=0,minute=0,sati=0;
int brojac=0;
int adresa=0;
int brojac_automobila=0;

void interrupt isr(void){
    /* Desio se prekid zbog pritiska tastera */
    if(INTCONbits.INTE && INTCONbits.INTF && ugao<=0) {
        INTCONbits.INTF = 0;
        eeprom_write(adresa++,brojac_automobila++);
        eeprom_write(adresa++,sati);
        eeprom_write(adresa++,minute);
        eeprom_write(adresa++,sekunde);
        /* Motor se podize */
        Mgore = 1;
        Mdole = 0;
        
        podizi = 1;
        spustaj = 0;
        
    }
    
    /* Prekid na donju ivicu - IR Senzor */
    if(INTCONbits.IOCIE && IOCBFbits.IOCBF1 && ugao>=90) {
        INTCONbits.IOCIF = 0;
        IOCBFbits.IOCBF1 = 0;
        
        /* Motor se spusta */
        Mgore = 0;
        Mdole = 1;
        
        podizi = 0;
        spustaj = 1;
    }
    
    /* Pozivanje prekida TMR0 za postavljanje AD konverzije*/
    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF) {
        INTCONbits.TMR0IF = 0;
        TMR0 = 130;
        
        
        if(brojac==125){
            sekunde++;
            brojac=0;
            if(sekunde==60){
                minute++;
                sekunde=0;
                if(minute==60){
                    sati++;
                    minute=0;
                    if(sati==24){
                        sati=0;
                    }
                }
            }
        }
        else brojac ++;
        ADCON0bits.GO = 1; // poziva se ADC prekid kad se zavrsi konverzija
        
    }
    
    /* ADC prekid */
    if(PIE1bits.ADIE && PIR1bits.ADIF) {
        PIR1bits.ADIF = 0;
        napon=5/1023.0*(ADRES);
        ugao=-72*napon+270;
        
        /* Rampa spustena */
        if(spustaj==1 && ugao<=0) {
            /* Iskljucio se motor - Da li ce pasti rampa??? */
            Mgore = 0;
            Mdole = 0;
            spustaj = 0;
            
        }
        
        /* Rampa podignuta */
        if(podizi==1 && ugao>=90) {
            /* Iskljucio se motor - Da li ce pasti rampa??? */
            Mgore = 0;
            Mdole = 0;
            podizi = 0;
            
        }
    }
    
}

/* Inicijalizacija ADC-a */
void adc_init(void) {
    /* ADCON0 */
    ADCON0bits.CHS4 = 0;
    ADCON0bits.CHS3 = 0;
    ADCON0bits.CHS2 = 0;
    ADCON0bits.CHS1 = 0;
    ADCON0bits.CHS0 = 0;
    ADCON0bits.ADON = 1;
    
    /* ADCON1 */
    ADCON1bits.ADFM = 1;
    ADCON1bits.ADCS2 = 1;
    ADCON1bits.ADCS1 = 1;
    ADCON1bits.ADCS0 = 1;
    ADCON1bits.ADNREF = 0;
    ADCON1bits.ADPREF1 = 0;
    ADCON1bits.ADPREF0 = 0;
}

/* Inicijalizacija EEPROM-a */
void eeprom_init(void) {
    EECON1bits.EEPGD = 0;
    EECON1bits.CFGS = 0;
    EECON1bits.WREN = 1;
}

/* Inicijalizacija INT prekida - RB0 (taster) */
void int_init(void) {
    OPTION_REGbits.INTEDG = 1;
}

/* Inicijalizacija TIMER0 prekida */
void tmr0_init(void) {
    OPTION_REGbits.TMR0CS = 0;
    OPTION_REGbits.TMR0SE = 0;
    /* Dodijelio sam prescaler */
    OPTION_REGbits.PSA = 0;
    /* prescaler = 128 */
    OPTION_REGbits.PS2 = 1;
    OPTION_REGbits.PS1 = 1;
    OPTION_REGbits.PS0 = 0;
    TMR0 = 130;
}

/* Inicijalizacija prekida za PORTB - RB1 (IR senzor) */
void portb_prekid_init(void) {
    IOCBNbits.IOCBN1 = 1;
}

/* Inicijalizacija portova */
void port_init(void) {
    ANSELA = 0xFF;
    TRISA = 0xFF;
    
    ANSELB = 0x00;
    TRISB = 0xFF;
    Mgore = 0;
    Mdole = 0;
    
    ANSELD = 0x00;
    TRISD = 0xFC;
    LATD = 0x00;
}

/* Inicijalizacija postavki za sve navedene prekide na jednom mjestu */
void prekidi_init(void) {
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    
    INTCONbits.TMR0IF = 0;
    INTCONbits.TMR0IE = 1;
    
    INTCONbits.INTF = 0;
    INTCONbits.INTE = 1;
    
    INTCONbits.IOCIF = 0;
    INTCONbits.IOCIE = 1;
    
    PIR1bits.ADIF = 0;
    PIE1bits.ADIE = 1;
    
    PIR2bits.EEIF = 0;
    PIE2bits.EEIE = 1;
}

void main(void) {
    port_init();
    
    int_init();
    tmr0_init();
    portb_prekid_init();
    
    eeprom_init();
    adc_init();
    
    prekidi_init();
    
    while(1);
    
    return;
}