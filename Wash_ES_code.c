//* Project Name: Embedded Systems - Washing Machine *   //

#include "driverlib.h"
#include <msp430.h>

volatile unsigned short switch1 = 0;

#define D1 800000

void main(void) {

    WDT_A_hold(WDT_A_BASE); //stop watchdog timer
    LCD_init();
    displayScrollText("WElCOME. PLEASE WAIT. WASHING MACHINE IS STARTING...");
	
    //---------------------------------------Part 1: Door and water sensor inputs--------------------------------------//

    //Door input is manual by switch, water input is automatic//
    //Pins 1.5 and 2.7 are used on breadboard for door and water sensors respectively and pin 2.5 is pullup resistor//

    //Setup Door input on pin 2.7 using LED//
    GPIO_setAsOutputPin( GPIO_PORT_P2, GPIO_PIN7 ); // set P2.7 as output pin
    GPIO_setOutputHighOnPin( GPIO_PORT_P2, GPIO_PIN7 ); // set P2.7 to high

    //Setup Water input on pin 1.5//

    GPIO_setAsOutputPin( GPIO_PORT_P1, GPIO_PIN5 ); //set P1.5 as output pin
    GPIO_setOutputLowOnPin( GPIO_PORT_P1, GPIO_PIN5 );//set pin 1.5 to 0 or low

    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P2, GPIO_PIN5 );

    //---------------------------------------Part 2: Wash and Spin Cycles------------------------------------------//


	//Wash cycle setup//
    GPIO_setAsOutputPin( GPIO_PORT_P1, GPIO_PIN4 );
    GPIO_setOutputHighOnPin( GPIO_PORT_P1, GPIO_PIN4 );

    //Spin cycle setup//
    GPIO_setAsOutputPin( GPIO_PORT_P1, GPIO_PIN6 );
    GPIO_setOutputLowOnPin( GPIO_PORT_P1, GPIO_PIN6 );

    PMM_unlockLPM5();

    initTimers();

    __bis_SR_register( GIE );         // Enable interrupts globally

    while(1) {

          switch1 = GPIO_getInputPinValue( GPIO_PORT_P2, GPIO_PIN5 );

        if ( switch1 == GPIO_INPUT_PIN_LOW ){
           GPIO_setOutputLowOnPin( GPIO_PORT_P2, GPIO_PIN7 ); // Turn off the Door LED for door lock
            GPIO_setOutputHighOnPin( GPIO_PORT_P1, GPIO_PIN5 ); // Turn on the Water LED
            _delay_cycles (D1);
            GPIO_setOutputLowOnPin( GPIO_PORT_P1, GPIO_PIN5 ); // Turn off the Water LED 
        }
}
}

void initTimers(void)
{
    Timer_A_initContinuousModeParam initContParam = { 0 };
    initContParam.clockSource =                 TIMER_A_CLOCKSOURCE_ACLK;       // Use ACLK (slower clock)
    initContParam.clockSourceDivider =          TIMER_A_CLOCKSOURCE_DIVIDER_16;  // Input clock = ACLK for timing of 30s
    initContParam.timerInterruptEnable_TAIE =   TIMER_A_TAIE_INTERRUPT_ENABLE;  // Enable TAR -> 0 interrupt
    initContParam.timerClear =                  TIMER_A_DO_CLEAR;               // Clear TAR & clock divider
    initContParam.startTimer =                  false;                          // Don't start the timer, yet

    Timer_A_initContinuousMode( TIMER_A1_BASE, &initContParam );
    Timer_A_clearTimerInterrupt( TIMER_A1_BASE );
    Timer_A_startCounter(
            TIMER_A1_BASE,
            TIMER_A_CONTINUOUS_MODE
    );
}


#pragma vector=TIMER1_A1_VECTOR  //for Wash Cycle
__interrupt void timer1_ISR (void)
{
    switch( __even_in_range( TA1IV, TA1IV_TAIFG )) {
        case TA1IV_NONE: break;                 // (0x00) None
        case TA1IV_TACCR1:                      // (0x02) CCR1 IFG
            _no_operation();
            break;
        case TA1IV_TACCR2:                      // (0x04) CCR2 IFG
            _no_operation();
            break;
        case TA1IV_3: break;                    // (0x06) Reserved
        case TA1IV_4: break;                    // (0x08) Reserved
        case TA1IV_5: break;                    // (0x0A) Reserved
        case TA1IV_6: break;                    // (0x0C) Reserved
        case TA1IV_TAIFG:                       // (0x0E) TA1IFG - TAR overflow
            GPIO_setOutputLowOnPin( GPIO_PORT_P1, GPIO_PIN4 );  // Turn Wash cycle LED off
            GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN6 );  // Toggle Spin cycle LED 
            break;
        default: _never_executed();
    }
}
#pragma vector=TIMER1_A2_VECTOR  //For Spin cycle
__interrupt void timer2_ISR (void)
{
    switch( __even_in_range( TA1IV, TA1IV_TAIFG )) {
        case TA1IV_NONE: break;                 // (0x00) None
        case TA1IV_TACCR1:                      // (0x02) CCR1 IFG
            _no_operation();
            break;
        case TA1IV_TACCR2:                      // (0x04) CCR2 IFG
            _no_operation();
            break;
        case TA1IV_3: break;                    // (0x06) Reserved
        case TA1IV_4: break;                    // (0x08) Reserved
        case TA1IV_5: break;                    // (0x0A) Reserved
        case TA1IV_6: break;                    // (0x0C) Reserved
        case TA1IV_TAIFG:                       // (0x0E) TA1IFG - TAR overflow
                GPIO_toggleOutputOnPin( GPIO_PORT_P1, GPIO_PIN6 );  // Toggle Spin Cycle LED 
            break;
        default: _never_executed();
    }
}
