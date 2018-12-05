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
int timeout;
void butt_init(void);
void settime(void);
int settime_pressed=0;
int setalarm_pressed=0;

//LCD Function Declarations
    void delayMs (uint32_t n);
    void nibblewrite(unsigned char data, unsigned char control);
    void command(unsigned char command);
    void data(unsigned char data);
    void LCDinit(void);
    void init_display_screen(void);
//UART Function Declarations
    void writeOutput(char *string); // write output charactrs to the serial port
    void readInput(char* string); // read input characters from INPUT_BUFFER that are valid
    void setupSerial(); // Sets up serial for use and enables interrupts
    void TA_init(void);
//ADC Function & temp sensor Declarations
    void SysTick_Init_interrupt(void);
    void SysTick_Handler(void);
    void settemp(void);
    void PortADC_init(void);
    void ADC14_init(void);




    char INPUT_BUFFER[BUFFER_SIZE];
    uint8_t storage_location = 0; // used in the interrupt to store new data
    uint8_t read_location = 0; // used in the main application to read valid data that hasn't been read yet

    char wordtime[] = "TIME";    //declaration of arrays
    char seconds[60];
    char minutes[60];
    char hours[60];
    int sec = 0;
    int min = 0;
    int hour = 1;
    int secflag = 0;
    int minflag = 0;
    int hourflag = 0;
//These flags will allow the program to determine if it only needs to print the ones digit
//or if it needs to print both tens and ones digits for numbers greater than 10


void main(void)
{

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    P1 -> SEL1 &= ~BIT0;  // initialize red LED for button test
    P1 -> SEL0 &= ~BIT0;
    P1 -> DIR |= BIT0;
    P1 -> OUT &= ~BIT0;

    int i,j; //loop integers

    butt_init();
    SysTick_Init_interrupt();     //initialization of SysTick with interrupt
    PortADC_init();               //initialization of ADC Ports
    ADC14_init();                 //initialization of ADC
    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);   //enables ADC interrupt in  NVIC
    LCDinit();            //initialization of LCD
    delayMs(10);

    TIMER32_1->LOAD = 3000000-1;
    TIMER32_1->CONTROL = 0xC2; /* no prescaler, periodic wrapping mode, disable interrupt, 32-bit timer. */

    __enable_irq();                  /* global enable IRQs */

   // init_display_screen();
    while(1)
    {
            settemp();
            command(0x85);  /* set cursor at beginning of first line */
                 for(j=0; j<4;j++)
                 {
                 data(wordtime[j]); //prints TIME to line 1
                 }

                 if(sec>=60)  //this section checks if the seconds should roll into a minute
                 {
                      sec = 0;
                      seconds[0] = '0';
                      seconds[1] = '0';
                      secflag = 0;
                      min = min + 1;
                 }
                 if(sec>=10)
                 {
                      secflag=1;
                 }

                 if(min>=60) //this section checks if minutes should roll into an hour
                 {
                     min = 0;
                     minutes[0] = '0';
                     minutes[1] = '0';
                     minflag = 0;
                     hour = hour +1;
                 }
                 if(min>=10)
                 {
                     minflag = 1;
                 }

                 if(hour > 12) //this section allows to change between AM and PM
                 {
                     hour = 1;
                     hourflag = 0;
                 }
                 if(hour >= 10)
                 {
                     hourflag = 1;
                 }

                 //printing the clock time to the LCD
                 command(0xC1);
                 sprintf(hours,"%d", hour); //displaying hours
                 if(hourflag == 0)
                 {
                     data(hours[0]);
                 }

                 else if(hourflag == 1)
                 {
                     for(i=0;i<2;i++)
                     {
                         data(hours[i]);
                     }
                 }

                 command(0xC3);
                 data(':');
                 command(0xC4);
                 sprintf(minutes, "%d", min); //displaying minutes
                 if(minflag == 0)
                 {
                     data('0');
                     command(0xC5);
                     data(minutes[0]);
                 }
                 else if(minflag == 1)
                 {
                      for(i=0;i<2;i++)
                      {
                          data(minutes[i]);
                      }
                 }

                 command(0xC7);
                 data(':');
                 command(0xC8);
                 sprintf(seconds, "%d", sec); //displaying seconds
                 if(secflag == 0)
                 {
                     data('0');
                     command(0xC9);
                     data(seconds[0]);
                 }
                 else if(secflag == 1)
                 {
                     for(i=0;i<2;i++)
                     {
                         data(seconds[i]);
                     }
                 }

                 sec = sec + 1;

                 while((TIMER32_1 -> RIS & 1) == 0); //waits 1 second until interrupt flag is set
                 TIMER32_1 -> INTCLR = 0; //clears interrupt flag


// main code serial input
//readInput(string); // Read the input up to \n, store in string.  This function doesn't return until \n is received
//        puts(string);
//        if(string[0] != '\0') // if string is not empty, check the inputted data.
//             {
//
//                if(string[0] == 'T') //Setting the time for the clock
//             {
//
//             }
//
//                if(string[0] == 'A') //setting the alarm time
//                {
//
//                }
//
//
//             }
    }
}




//------------- button interrupt functions -----------------

void butt_init(void) //button initializations
{
    /* configure P5.0, P5.1 for switch inputs */
    P5->SEL1 &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);    /* configure P1.1, P1.4 as simple I/O */
    P5->SEL0 &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);
    P5->DIR &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);     /* P1.1, P1.4 set as input */
    P5->REN |= (BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);      /* P1.1, P1.4 pull resistor enabled */
    P5->OUT |= (BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);      /* Pull up/down is selected by P1->OUT */
    P5->IES |= (BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);      /* make interrupt trigger on high-to-low transition */
    P5->IFG = 0;          /* clear pending interrupt flags */
    P5->IE |= (BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);       /* enable interrupt from P1.1, P1.4 */


    NVIC_EnableIRQ(PORT5_IRQn);      /* enable interrupt in NVIC */

}
void PORT5_IRQHandler(void) //IRQ Handler for button interrupts
{
    int i;

    if(P5->IFG & BIT0)//SET TIME BUTTON
       {
           P1 -> OUT ^= BIT0;
       }
    if(P5->IFG & BIT1)//SET ALARM BUTTON
       {
        P1 -> OUT ^= BIT0;
       }
    if(P5->IFG & BIT2)//ON/OFF/UP BUTTON
       {
        P1 -> OUT ^= BIT0;
       }
    if(P5->IFG & BIT4)//SNOOZE/DOWN
       {
        P1 -> OUT ^= BIT0;
       }
    if(P5->IFG & BIT6)//CHANGE CLOCK SPEED 1 SECOND REAL TIME = 1 SECOND ALARM CLOCK TIME
       {
        P1 -> OUT ^= BIT0;
       }
    if(P5->IFG & BIT7)//CHANGE CLOCK SPEED 1 SECOND REAL TIME = 1 MINUTE ALARM CLOCK TIME
       {
        P1 -> OUT ^= BIT0;
       }

    P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7); /* clear the interrupt flag before return */
}

//----------LCD functions----------------------

void LCDinit(void) //LCD info is in Lab 6 pt. 2 *************
{
    P4->DIR = 0xFF;  //set all of pin 4 ports as output
    __delay_cycles(30000); //using delay cycles here because we want to use the SysTick for the Temp Sensor interrupt
    nibblewrite(0x30, 0);    //reset the LCD 3
    __delay_cycles(30000);
    nibblewrite(0x30, 0);    //reset the LCD 3
    __delay_cycles(30000);
    nibblewrite(0x30, 0);    //reset the LCD 3
    __delay_cycles(30000);
    nibblewrite(0x20, 0);    //set cursor HOME
    __delay_cycles(30000);

    command(0x28);  //4-bit interface
    command(0x06);  // move cursor right
    command(0x01);  //clear screen
    command(0x0F);  //turn on blinking cursor

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
void init_display_screen(void) //sets initial display of LCD
{
    int i;
    char time[]= "HH:MM:SS XM";
    char alarmstatus[]= "ALARM: ";
    char alarmtime[]= "HH:MM XM";
    char temperature[]= "XX.X F";
    command(1);
    command(0x80);
    for(i=0;i<11;i++)
    {
       data(time[i]);
    }
    command(0xC0);
    for(i=0;i<7;i++)
    {
       data(alarmstatus[i]);
    }
    command(0x90);
    for(i=0;i<8;i++)
    {
       data(alarmtime[i]);
    }
    command(0xD0);
    for(i=0;i<6;i++)
    {
       data(temperature[i]);
    }
}

//-----------time functions -----------------

void settime(void)//pin 5.2=On/Off/Up  5.4=Snooze/Down 5.0=Set Time
{
    int i=0;
    while(i==0)
    {


    if (P5->OUT &=~ BIT2)
    {
        if(hour > 12) //this section allows to change between AM and PM
          {
              hour = 1;
              hourflag = 0;
          }
        if(hour >= 10)
          {
              hourflag = 1;
          }
        hour=hour+1;
        command(0xC1);
        sprintf(hours,"%d", hour); //displaying hours
                      if(hourflag == 0)
                      {
                          data(hours[0]);
                      }

                      else if(hourflag == 1)
                      {
                          for(i=0;i<2;i++)
                          {
                              data(hours[i]);
                          }
                      }

    }
    if (P5->OUT &=~ BIT4)
       {
           if(hour < 1) //this section allows to change between AM and PM
                       {
                           hour = 12;
                           hourflag = 0;
                       }
           if(hour >= 10)
             {
                 hourflag = 1;
             }
           hour=hour-1;
           command(0xC1);
           sprintf(hours,"%d", hour); //displaying hours
                         if(hourflag == 0)
                         {
                             data(hours[0]);
                         }

                         else if(hourflag == 1)
                         {
                             for(i=0;i<2;i++)
                             {
                                 data(hours[i]);
                             }
                         }

       }
    if(P5->OUT &=~ BIT0)
    {
        i++;
    }
    }
   while(i==1)
   {
       if(min>=60) //this section checks if minutes should roll into an hour
                        {
                            min = 0;
                            minutes[0] = '0';
                            minutes[1] = '0';
                            minflag = 0;
                            hour = hour +1;
                        }
                        if(min>=10)
                        {
                            minflag = 1;
                        }
        command(0xC3);
                   data(':');
                   command(0xC4);
                   sprintf(minutes, "%d", min); //displaying minutes
                   if(minflag == 0)
                   {
                       data('0');
                       command(0xC5);
                       data(minutes[0]);
                   }
                   else if(minflag == 1)
                   {
                        for(i=0;i<2;i++)
                        {
                            data(minutes[i]);
                        }
                   }

                   command(0xC7);
       if(P5->OUT &=~ BIT0)
           {
               i++;
           }
   }
}

void setalarm(void)
{
    int i;
}


//-------------temp sensor functions-------------
void settemp(void)
{
    int i;
    char fer[100];
    char temp[]= "TEMP is XX.X F";
    float volt;
    float mvolt;
    float tempC;
    float tempF;
    int tempFint;
    ADC14->CTL0 |= ADC14_CTL0_SC;    //starts conversion
    while(!(ADC14->IFGR0));         //waits for it to complete
    float result = ADC14->MEM[0];         //get value from the ADC
    volt = (result*3.3)/16384;      //converts raw value to voltage

           mvolt = volt*1000;        //converts volts to mV

           tempC = (mvolt-500)/10;     //finds temp in Celsius
           tempF = (tempC * 9);  //converts celsius to fahrenheit
           tempF = (tempF/5);
           tempF = (tempF + 32);
           tempFint=tempF;
           if(timeout)
           {   //prints the RAW, converted, C and F values
           printf("\t RAW = %f\n\tVOLTAGE = %f\n\tCelcius = %f \n\tFarenheit = %f\n\n", result, volt, tempC, tempF);
           timeout = 0;  //clears interrupt flag
           }


           temp[8]=tempF/10;
           temp[9]=tempFint-(temp[0]*10);
           temp[11]=tempF-(temp[0]*10)-(temp[1]);

           command(0x90);
           for(i=0;i<14;i++)
           {
               data(temp[i]);
           }

}
void PortADC_init(void)
{
    P5->SEL0 |= BIT5;   //sets pin 5.5 as A0 input
    P5->SEL1 |= BIT5;
}
 void ADC14_init(void)
 {
     ADC14->CTL0 &= ~ADC14_CTL0_ENC;    //turns off ADC converter while initializing
     ADC14->CTL0 |= 0x04200210;         //16 sample clocks, SMCLK, S/H pulse
     ADC14->CTL1 =  0x00000030;         //14 bit resolution
     ADC14->CTL1 |= 0x00000000;         //convert for mem0 register
     ADC14->MCTL[0]=0x00000000;         //mem[0] to ADC14INCHx = 0
     ADC14->CTL0 |= ADC14_CTL0_ENC;     //enables ADC14ENC and starts ADC after configuration
 }

 void SysTick_Init_interrupt(void)
 {
     SysTick -> CTRL = 0;            //turns off SysTick timer
     SysTick -> LOAD = 750000;   //Load top value
     SysTick -> VAL = 0;             //clears value on SysTick
     SysTick -> CTRL = 0x00000007;   //enables SysTick and interrupts
 }

 void SysTick_Handler(void)
 {
    timeout = 1;
 }

void delayMs(uint32_t n)  //setup the delay function
{
  SysTick -> LOAD = (n*3000-1);
  SysTick -> VAL = 0;
  while((SysTick -> CTRL & 0x00010000) == 0);

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
