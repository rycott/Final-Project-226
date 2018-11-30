#include "msp.h"
#include <stdio.h>
#include <string.h>

/* For LCD
 * D7 -> 4.7
 * D6 -> 4.6
 * D5 -> 4.5
 * D4 -> 4.4
 */
#define RS BIT1  //RS is 4.1
#define RW BIT2  //RW is on 4.2
#define EN BIT3  //EN is 4.3
#define BUFFER_SIZE 100
int settime_but = 0;
void butt_init(void);
//LCD Function Declarations
    void settime();
    void delayMs (uint32_t n);
    void nibblewrite(unsigned char data, unsigned char control);
    void command(unsigned char command);
    void data(unsigned char data);
    void LCDinit(void);
    void SysTick_init(void);
//UART Function Declarations
    void writeOutput(char *string); // write output charactrs to the serial port
    void readInput(char* string); // read input characters from INPUT_BUFFER that are valid
    void setupSerial(); // Sets up serial for use and enables interrupts
    void TA_init(void);

    char INPUT_BUFFER[BUFFER_SIZE];

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer
    butt_init();
    __enable_irq();                  /* global enable IRQs */
    while(1)
    {
        readInput(string); // Read the input up to \n, store in string.  This function doesn't return until \n is received
        puts(string);
        if(string[0] != '\0') // if string is not empty, check the inputted data.
             {

                if(string[0] == 'T') //Setting the time for the clock
             {

             }

                if(string[0] == 'A') //setting the alarm time
                {

                }


             }
    }
}





void butt_init(void)
{
    /* configure P1.1, P1.4 for switch inputs */
    P5->SEL1 &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5);    /* configure P1.1, P1.4 as simple I/O */
    P5->SEL0 &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5);
    P5->DIR &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5);     /* P1.1, P1.4 set as input */
    P5->REN |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5);      /* P1.1, P1.4 pull resistor enabled */
    P5->OUT |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5);      /* Pull up/down is selected by P1->OUT */
    P5->IES |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5);      /* make interrupt trigger on high-to-low transition */
    P5->IFG = 0;          /* clear pending interrupt flags */
    P5->IE |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5);       /* enable interrupt from P1.1, P1.4 */


    NVIC_EnableIRQ(PORT5_IRQn);      /* enable interrupt in NVIC */

}

void PORT5_IRQHandler(void) {
    int i;

    if(P5->IFG & BIT0)//SET TIME BUTTON
       {
           settime();
       }
    if(P5->IFG & BIT1)//SET ALARM BUTTON
       {

       }
    if(P5->IFG & BIT2)//ON/OFF/UP BUTTON
       {

       }
    if(P5->IFG & BIT3)//SNOOZE/DOWN
       {

       }
    if(P5->IFG & BIT4)//
       {

       }
    if(P5->IFG & BIT5)//
       {

       }

    P5->IFG &= ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5); /* clear the interrupt flag before return */
}
//----------LCD functions----------------------

void LCDinit(void)    //LCD info is in Lab 6 pt. 2 *************
{
    P4->DIR = 0xFF;  //set all of pin 4 ports as output
    delayMs(30);
    nibblewrite(0x30, 0);    //reset the LCD 3
    delayMs(10);
    nibblewrite(0x30, 0);    //reset the LCD 3
    delayMs(10);
    nibblewrite(0x30, 0);    //reset the LCD 3
    delayMs(10);
    nibblewrite(0x20, 0);    //set cursor HOME
    delayMs(10);

    command(0x28);  //4-bit interface
    command(0x06);  // move cursor right
    command(0x01);  //clear screen
    command(0x0F);  //turn on blinking cursor

}

void SysTick_init(void)
{
    SysTick -> CTRL = 0;          //initializes systick
    SysTick -> LOAD = 0X00FFFFFF;
    SysTick -> VAL = 0;
    SysTick -> CTRL = 0x00000005;
}

void delayMs(uint32_t n)  //setup the delay function
{
  SysTick -> LOAD = (n*3000-1);
  SysTick -> VAL = 0;
  while((SysTick -> CTRL & 0x00010000) == 0);

    }

void nibblewrite(unsigned char data, unsigned char control)
{
    data &= 0xF0;         //takes bits and sends them to LCD used by data()
    control &= 0x0F;
    P4->OUT = data|control;
    P4->OUT = data|control|EN;
    delayMs(1);
    P4-> OUT = data;
    P4-> OUT = 0;
}

void command(unsigned char command)  //function to write commands to the LCD
{
    nibblewrite(command & 0xF0, 0);
    nibblewrite(command << 4, 0);

    if (command < 4)
    { delayMs(4);
    }
    else
    {
        delayMs(1);

    }
}

void data(unsigned char data)  //function to write data to the LCD
{
    nibblewrite(data & 0xF0, RS);
    nibblewrite(data <<4 , RS);

    delayMs(1);
}

//-------- UART functions --------------

/*----------------------------------------------------------------
 * void writeOutput(char *string)
 *
 * Description:  This is a function similar to most serial port
 * functions like printf.  Written as a demonstration and not
 * production worthy due to limitations.
 * One limitation is poor memory management.
 * Inputs: Pointer to a string that has a string to send to the serial.
 * Outputs: Places the data on the serial output.
----------------------------------------------------------------*/
void writeOutput(char *string)
{
    int i = 0;  // Location in the char array "string" that is being written to

    while(string[i] != '\0') {
        EUSCI_A0->TXBUF = string[i];
        i++;
        while(!(EUSCI_A0->IFG & BIT1));
    }
}

/*----------------------------------------------------------------
 * void readInput(char *string)
 *
 * Description:  This is a function similar to most serial port
 * functions like ReadLine.  Written as a demonstration and not
 * production worthy due to limitations.
 * One of the limitations is that it is BLOCKING which means
 * it will wait in this function until there is a \n on the
 * serial input.
 * Another limitation is poor memory management.
 * Inputs: Pointer to a string that will have information stored
 * in it.
 * Outputs: Places the serial data in the string that was passed
 * to it.  Updates the global variables of locations in the
 * INPUT_BUFFER that have been already read.
----------------------------------------------------------------*/
void readInput(char *string)
{
    int i = 0;  // Location in the char array "string" that is being written to

    // One of the few do/while loops I've written, but need to read a character before checking to see if a \n has been read
    do
    {
        // If a new line hasn't been found yet, but we are caught up to what has been received, wait here for new data
        while(read_location == storage_location && INPUT_BUFFER[read_location] != '\n');
        string[i] = INPUT_BUFFER[read_location];  // Manual copy of valid character into "string"
        INPUT_BUFFER[read_location] = '\0';
        i++; // Increment the location in "string" for next piece of data
        read_location++; // Increment location in INPUT_BUFFER that has been read
        if(read_location == BUFFER_SIZE)  // If the end of INPUT_BUFFER has been reached, loop back to 0
            read_location = 0;
    }
    while(string[i-1] != '\n'); // If a \n was just read, break out of the while loop

    string[i-1] = '\0'; // Replace the \n with a \0 to end the string when returning this function
}

/*----------------------------------------------------------------
 * void EUSCIA0_IRQHandler(void)
 *
 * Description: Interrupt handler for serial communication on EUSCIA0.
 * Stores the data in the RXBUF into the INPUT_BUFFER global character
 * array for reading in the main application
 * Inputs: None (Interrupt)
 * Outputs: Data stored in the global INPUT_BUFFER. storage_location
 * in the INPUT_BUFFER updated.
----------------------------------------------------------------*/
void EUSCIA0_IRQHandler(void)
{
    if (EUSCI_A0->IFG & BIT0)  // Interrupt on the receive line
    {
        INPUT_BUFFER[storage_location] = EUSCI_A0->RXBUF; // store the new piece of data at the present location in the buffer
        EUSCI_A0->IFG &= ~BIT0; // Clear the interrupt flag right away in case new data is ready
        storage_location++; // update to the next position in the buffer
        if(storage_location == BUFFER_SIZE) // if the end of the buffer was reached, loop back to the start
            storage_location = 0;
    }
}

/*----------------------------------------------------------------
 * void setupP1()
 * Sets up P1.0 as a GPIO output initialized to 0.
 *
 * Description:
 * Inputs: None
 * Outputs: Setup of P.1 to an output of a 0.
----------------------------------------------------------------*/
void setupP1()
{
    P1->SEL0 &= ~BIT0; //GPIO
    P1->SEL1 &= ~BIT0;
    P1->DIR  |=  BIT0; //OUTPUT
    P1->OUT  &= ~BIT0; //Start as off
}

/*----------------------------------------------------------------
 * void setupSerial()
 * Sets up the serial port EUSCI_A0 as 115200 8E2 (8 bits, Even parity,
 * two stops bit.)  Enables the interrupt so that received data will
 * results in an interrupt.
 * Description:
 * Inputs: None
 * Outputs: None
----------------------------------------------------------------*/
void setupSerial()
{
    P1->SEL0 |=  (BIT2 | BIT3); // P1.2 and P1.3 are EUSCI_A0 RX
    P1->SEL1 &= ~(BIT2 | BIT3); // and TX respectively.

    EUSCI_A0->CTLW0  = BIT0; // Disables EUSCI. Default configuration is 8N1
    EUSCI_A0->CTLW0 |= BIT7; // Connects to SMCLK BIT[7:6] = 10
    EUSCI_A0->CTLW0 |= (BIT(15)|BIT(14)|BIT(11));  //BIT15 = Parity, BIT14 = Even, BIT11 = Two Stop Bits
    // Baud Rate Configuration
    // 3000000/(16*115200) = 1.628  (3 MHz at 115200 bps is fast enough to turn on over sampling (UCOS = /16))
    // UCOS16 = 1 (0ver sampling, /16 turned on)
    // UCBR  = 1 (Whole portion of the divide)
    // UCBRF = .628 * 16 = 10 (0x0A) (Remainder of the divide)
    // UCBRS = 3000000/115200 remainder=0.04 -> 0x01 (look up table 22-4)
    EUSCI_A0->BRW = 1;  // UCBR Value from above
    EUSCI_A0->MCTLW = 0x01A1; //UCBRS (Bits 15-8) & UCBRF (Bits 7-4) & UCOS16 (Bit 0)

    EUSCI_A0->CTLW0 &= ~BIT0;  // Enable EUSCI
    EUSCI_A0->IFG &= ~BIT0;    // Clear interrupt
    EUSCI_A0->IE |= BIT0;      // Enable interrupt
    NVIC_EnableIRQ(EUSCIA0_IRQn);
}

void TA_init(void)
{
P2 -> SEL0 |= (BIT4|BIT5|BIT6);
P2 -> SEL1 &=~ (BIT4|BIT5|BIT6);
P2 -> DIR |= (BIT4|BIT5|BIT6);

TIMER_A0 -> CCR[0] = 64000;  //PWM period

TIMER_A0 -> CCR[1]=32000;
TIMER_A0 -> CCTL[1] = TIMER_A_CCTLN_OUTMOD_7;
TIMER_A0 -> CTL = TIMER_A_CTL_TASSEL_2 |TIMER_A_CTL_MC_1| TIMER_A_CTL_CLR;

TIMER_A0 -> CCR[2] = 32000;
TIMER_A0 -> CCTL[2] = TIMER_A_CCTLN_OUTMOD_7;
TIMER_A0 -> CCR[3] = 32000;
TIMER_A0 -> CCTL[3] = TIMER_A_CCTLN_OUTMOD_7;

}

