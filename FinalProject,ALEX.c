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
int timeout;
void butt_init(void);
void settime(void);
void noisesetup(void);
void alarmgooff(void);
int settime_pressed=0;
int setalarm_pressed=0;
int alarm_status=0;
//LCD Function Declarations

    void delayMs (uint32_t n);
    void nibblewrite(unsigned char data, unsigned char control);
    void command(unsigned char command);
    void data(unsigned char data);
    void LCDinit(void);
    void SysTick_Init_interrupt(void);
    void init_display_screen(void);
//UART Function Declarations
    void writeOutput(char *string); // write output charactrs to the serial port
    void readInput(char* string); // read input characters from INPUT_BUFFER that are valid
    void setupSerial(); // Sets up serial for use and enables interrupts
    void TA_init(void);
//ADC Function Declarations
    void settemp(void);
    void PortADC_init(void);
    void ADC14_init(void);


    char INPUT_BUFFER[BUFFER_SIZE];
    uint8_t storage_location = 0; // used in the interrupt to store new data
    uint8_t read_location = 0; // used in the main application to read valid data that hasn't been read yet
    char snoozebut[] = "SNOOZE";
    char alarmstatusoff[] = "ALARM OFF";
    char alarmstatuson[] = "ALARM  ON";
    char wordtime[] = "TIME";    //declaration of arrays
    char alarmtime[] = "ALARM";
    char am[] = "AM";
    char pm[] = "PM";
    char blank[] = " ";
    char seconds[60];
    char minutes[60];
    char hours[60];
    int sec = 0;
    int min = 0;
    int hour = 1;
    int secflag = 0;
    int minflag = 0;
    int hourflag = 0;
    char alarmseconds[60];
    char alarmminutes[60];
    char alarmhours[60];
    int  alarmsec = 0;
    int  alarmmin = 2;
    int  alarmhour = 1;
    int  alarmsecflag = 0;
    int  alarmminflag = 0;
    int  alarmhourflag = 0;
    int  amflag=1;
    int  pmflag=0;
    int  alarmamflag=1;
    int alarmpmflag=0;
    int snooze=0;
    int speedset=0;
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
    noisesetup();
    butt_init();
    SysTick_Init_interrupt();    //initialization of SysTick interrupt
    LCDinit();            //initialization of LCD
    PortADC_init();               //initialization of ADC Ports
    ADC14_init();                 //initialization of ADC
    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);   //enables ADC interrupt in  NVIC
    delayMs(10);

    TIMER32_1->LOAD = 3000000-1;
    TIMER32_1->CONTROL = 0xC2; /* no prescaler, periodic wrapping mode, disable interrupt, 32-bit timer. */

    __enable_irq();                  /* global enable IRQs */

    while(1)
    {
        settemp();
            command(0x80);  /* set cursor at beginning of first line */
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
                     pmflag=1;

                 }
                 if(hour >= 10)
                 {
                     hourflag = 1;
                 }

                 //printing the clock time to the LCD
                 command(0x85);
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

                 command(0x87);
                 data(':');
                 command(0x88);
                 sprintf(minutes, "%d", min); //displaying minutes
                 if(minflag == 0)
                 {
                     data('0');
                     command(0x89);
                     data(minutes[0]);
                 }
                 else if(minflag == 1)
                 {
                      for(i=0;i<2;i++)
                      {
                          data(minutes[i]);
                      }
                 }

                 command(0x8A);
                 data(':');
                 command(0x8B);
                 sprintf(seconds, "%d", sec); //displaying seconds
                 if(secflag == 0)
                 {
                     data('0');
                     command(0x8C);
                     data(seconds[0]);
                 }
                 else if(secflag == 1)
                 {
                     for(i=0;i<2;i++)
                     {
                         data(seconds[i]);
                     }
                 }
 if (amflag==1)
             {
                 command(0x8E);
                 for(i=0;i<2;i++)
                 {
                     data(am[i]);
                 }
                 pmflag=0;
             }
 if (pmflag==1)
             {
                 command(0x8E);
                 for(i=0;i<2;i++)
                 {
                     data(pm[i]);
                 }
                 amflag=0;
             }
 if(alarm_status==0)
         {

             command(0xC0);
             for(i=0;i<9;i++)
                {
                   data(alarmstatusoff[i]);
                }
         }
else
         {

             command(0xC0);
             for(i=0;i<9;i++)
                {
                   data(alarmstatuson[i]);
                }
         }
                 command(0x90);  /* set cursor at beginning of first line */
                 for(j=0; j<5;j++)
                 {
                 data(alarmtime[j]); //prints TIME to line 1
                 }
                 if(alarmsec>=60)  //this section checks if the seconds should roll into a minute
                        {
                             alarmsec = 0;
                             alarmseconds[0] = '0';
                             alarmseconds[1] = '0';
                             alarmsecflag = 0;
                             alarmmin = alarmmin + 1;
                        }
                        if(alarmsec>=10)
                        {
                             alarmsecflag=1;
                        }

                        if(alarmmin>=60) //this section checks if minutes should roll into an hour
                        {
                            alarmmin = 0;
                            alarmminutes[0] = '0';
                            alarmminutes[1] = '0';
                            alarmminflag = 0;
                            alarmhour = alarmhour +1;
                        }
                        if(alarmmin>=10)
                        {
                            alarmminflag = 1;
                        }

                        if(hour > 12) //this section allows to change between AM and PM
                        {
                            alarmhour = 1;
                            alarmhourflag = 0;
                            alarmpmflag=1;

                        }
                        if(alarmhour >= 10)
                        {
                            alarmhourflag = 1;
                        }
                 //printing the clock time to the LCD
                 command(0x96);
                 sprintf(alarmhours,"%d", alarmhour); //displaying hours
                 if(alarmhourflag == 0)
                 {
                     data(alarmhours[0]);
                 }

                 else if(alarmhourflag == 1)
                 {
                     for(i=0;i<2;i++)
                     {
                         data(alarmhours[i]);
                     }
                 }

                 command(0x98);
                 data(':');
                 command(0x99);
                 sprintf(alarmminutes, "%d", alarmmin); //displaying minutes
                 if(alarmminflag == 0)
                 {
                     data('0');
                     command(0x9A);
                     data(alarmminutes[0]);
                 }
                 else if(alarmminflag == 1)
                 {
                      for(i=0;i<2;i++)
                      {
                          data(alarmminutes[i]);
                      }
                 }

                 command(0x9B);
                 data(':');
                 command(0x9C);
                 sprintf(alarmseconds, "%d", alarmsec); //displaying seconds
                 if(alarmsecflag == 0)
                 {
                     data('0');
                     command(0x9C);
                     data(alarmseconds[0]);
                 }
                 else if(alarmsecflag == 1)
                 {
                     for(i=0;i<2;i++)
                     {
                         data(alarmseconds[i]);
                     }
                 }
                 if (alarmamflag==1)
                              {
                                  command(0x9E);
                                  for(i=0;i<2;i++)
                                  {
                                      data(am[i]);
                                  }
                                  alarmpmflag=0;
                              }
                  if (alarmpmflag==1)
                              {
                                  command(0x9E);
                                  for(i=0;i<2;i++)
                                  {
                                      data(pm[i]);
                                  }
                                  alarmamflag=0;
                              }
if(speedset==0)
{
    sec = sec + 1;
}
if(speedset==1)
{
    sec = sec + 60;
}

if(alarmhour==hour)
{
    if(alarmmin==min)
    {
        if(alarm_status==1)
        {
            if((alarmamflag==amflag) | (alarmpmflag==pmflag))
            {
                snooze=0;
                alarmgooff();

            }
        }
    }

}
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
    /* configure P1.1, P1.4 for switch inputs */
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

    if(P5->IFG & BIT0)//SET TIME BUTTON BLACK
       {
        P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);
        settime_pressed++;

        int i;
           while(settime_pressed==1)
           {


           if (P5->IFG & BIT2)
           {
               P5->IFG &= ~BIT2;
               hour=hour+1;
               if(hour > 12) //this section allows to change between AM and PM
                 {
                     hour = 1;
                     hourflag = 0;
                     amflag=0;
                     pmflag=1;

                 }
               if(hour >= 10)
                 {
                     hourflag = 1;
                 }
               if (amflag==1)
                            {
                                command(0x8E);
                                for(i=0;i<2;i++)
                                {
                                    data(am[i]);
                                }
                                pmflag=0;
                            }
                if (pmflag==1)
                            {
                                command(0x8E);
                                for(i=0;i<2;i++)
                                {
                                    data(pm[i]);
                                }
                                amflag=0;
                            }

               command(0x85);
               sprintf(hours,"%d", hour); //displaying hours
                             if(hourflag == 0)
                             {
                                 data(hours[0]);
                              command(0x86);
                              data(blank[0]);

                             }

                             else if(hourflag == 1)
                             {
                                 for(i=0;i<2;i++)
                                 {
                                     data(hours[i]);
                                 }
                             }

           }
           if (P5->IFG & BIT4)
              {
               P5->IFG &= ~BIT4;
                   hour=hour-1;
                  if(hour < 1) //this section allows to change between AM and PM
                              {
                                  hour = 12;
                                  hourflag = 0;
                                  amflag=0;
                                  pmflag=1;
                              }
                  if(hour >= 10)
                    {
                        hourflag = 1;
                    }
                  if (amflag==1)
                               {
                                   command(0x8E);
                                   for(i=0;i<2;i++)
                                   {
                                       data(am[i]);
                                   }
                                   pmflag=0;
                               }
                   if (pmflag==1)
                               {
                                   command(0x8E);
                                   for(i=0;i<2;i++)
                                   {
                                       data(pm[i]);
                                   }
                                   amflag=0;
                               }

                  command(0x85);
                  sprintf(hours,"%d", hour); //displaying hours
                                if(hourflag == 0)
                                {
                                    data(hours[0]);
                                    command(0x86);
                                    data(blank[0]);
                                }

                                else if(hourflag == 1)
                                {
                                    for(i=0;i<2;i++)
                                    {
                                        data(hours[i]);
                                    }
                                }

              }
           if(P5->IFG & BIT0)
           {
               P5->IFG &= ~BIT0;
               settime_pressed++;
           }
           }
          while(settime_pressed==2)
          {
              if (P5->IFG & BIT2)
                           {
                            P5->IFG &= ~BIT2;
                            min++;
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
                          command(0x87);
                          data(':');
                          command(0x88);
                          sprintf(minutes, "%d", min); //displaying minutes
                          if(minflag == 0)
                          {
                              data('0');
                              command(0x89);
                              data(minutes[0]);
                          }
                          else if(minflag == 1)
                          {
                               for(i=0;i<2;i++)
                               {
                                   data(minutes[i]);
                               }
                          }
                           }
              if (P5->IFG & BIT4)
                                         {
                                          P5->IFG &= ~BIT4;
                                          min=min-1;
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
                                        command(0x87);
                                        data(':');
                                        command(0x88);
                                        sprintf(minutes, "%d", min); //displaying minutes
                                        if(minflag == 0)
                                        {
                                            data('0');
                                            command(0x89);
                                            data(minutes[0]);
                                        }
                                        else if(minflag == 1)
                                        {
                                             for(i=0;i<2;i++)
                                             {
                                                 data(minutes[i]);
                                             }
                                        }
                                         }
                          command(0x8A);
              if(P5->IFG & BIT0)
                  {
                      P5->IFG &= ~BIT0;
                      settime_pressed=0;
                  }
          }
           P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);

       }
    if(P5->IFG & BIT1)//SET ALARM BUTTON GREEN
       {
        P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);
          setalarm_pressed++;

          int i;
             while(setalarm_pressed==1)
             {


             if (P5->IFG & BIT2)
             {
                 P5->IFG &= ~BIT2;
                 alarmhour=alarmhour+1;
                 if(alarmhour > 12) //this section allows to change between AM and PM
                   {
                       alarmhour = 1;
                       alarmhourflag = 0;
                       alarmamflag=1;
                       alarmpmflag=0;

                   }
                 if(alarmhour >= 10)
                   {
                       alarmhourflag = 1;
                   }

                 command(0x96);
                 sprintf(alarmhours,"%d", alarmhour); //displaying hours
                               if(alarmhourflag == 0)
                               {
                                   data(alarmhours[0]);
                                   command(0x97);
                                   data(blank[0]);
                               }

                               else if(alarmhourflag == 1)
                               {
                                   for(i=0;i<2;i++)
                                   {
                                       data(alarmhours[i]);
                                   }
                               }
                               if (alarmamflag==1)
                                           {
                                               command(0x9E);
                                               for(i=0;i<2;i++)
                                               {
                                                   data(am[i]);
                                               }
                                               alarmpmflag=0;
                                           }
                               if (alarmpmflag==1)
                                           {
                                               command(0x9E);
                                               for(i=0;i<2;i++)
                                               {
                                                   data(pm[i]);
                                               }
                                               alarmamflag=0;
                                           }

             }
             if (P5->IFG & BIT4)
                {
                 P5->IFG &= ~BIT4;
                 alarmhour=alarmhour-1;
                    if(alarmhour < 1) //this section allows to change between AM and PM
                                {
                                    alarmhour = 12;
                                    alarmhourflag = 0;
                                    alarmamflag=0;
                                    alarmpmflag=1;
                                }
                    if(alarmhour >= 10)
                      {
                          alarmhourflag = 1;
                      }
                    if (alarmamflag==1)
                                  {
                                      command(0x9E);
                                      for(i=0;i<2;i++)
                                      {
                                          data(am[i]);
                                      }
                                      alarmpmflag=0;
                                  }
                      if (alarmpmflag==1)
                                  {
                                      command(0x9E);
                                      for(i=0;i<2;i++)
                                      {
                                          data(pm[i]);
                                      }
                                      alarmamflag=0;
                                  }

                    command(0x96);
                    sprintf(alarmhours,"%d", alarmhour); //displaying hours
                                  if(alarmhourflag == 0)
                                  {
                                      data(alarmhours[0]);
                                      command(0x97);
                                      data(blank[0]);
                                  }

                                  else if(alarmhourflag == 1)
                                  {
                                      for(i=0;i<2;i++)
                                      {
                                          data(alarmhours[i]);
                                      }
                                  }

                }
             if(P5->IFG & BIT1)
             {
                 P5->IFG &= ~BIT1;
                 setalarm_pressed++;
             }
             }
            while(setalarm_pressed==2)
            {
                if (P5->IFG & BIT2)
                             {
                              P5->IFG &= ~BIT2;
                              alarmmin++;
                if(alarmmin>=60) //this section checks if minutes should roll into an hour
                                 {
                                     alarmmin = 0;
                                     alarmminutes[0] = '0';
                                     alarmminutes[1] = '0';
                                     alarmminflag = 0;
                                     alarmhour = alarmhour +1;
                                 }
                                 if(min>=10)
                                 {
                                     alarmminflag = 1;
                                 }
                            command(0x98);
                            data(':');
                            command(0x99);
                            sprintf(alarmminutes, "%d", alarmmin); //displaying minutes
                            if(alarmminflag == 0)
                            {
                                data('0');
                                command(0x9A);
                                data(alarmminutes[0]);
                            }
                            else if(alarmminflag == 1)
                            {
                                 for(i=0;i<2;i++)
                                 {
                                     data(alarmminutes[i]);
                                 }
                            }
                             }
                if (P5->IFG & BIT4)
                                           {
                                            P5->IFG &= ~BIT4;
                                            alarmmin=alarmmin-1;
                              if(alarmmin>=60) //this section checks if minutes should roll into an hour
                                               {
                                                   alarmmin = 0;
                                                   alarmminutes[0] = '0';
                                                   alarmminutes[1] = '0';
                                                   alarmminflag = 0;
                                                   alarmhour = alarmhour +1;
                                               }
                                               if(min>=10)
                                               {
                                                   alarmminflag = 1;
                                               }
                                          command(0x98);
                                          data(':');
                                          command(0x99);
                                          sprintf(alarmminutes, "%d", alarmmin); //displaying minutes
                                          if(alarmminflag == 0)
                                          {
                                              data('0');
                                              command(0x9A);
                                              data(alarmminutes[0]);
                                          }
                                          else if(alarmminflag == 1)
                                          {
                                               for(i=0;i<2;i++)
                                               {
                                                   data(alarmminutes[i]);
                                               }
                                          }
                                           }
                            command(0x8C);
                if(P5->IFG & BIT1)
                    {
                        P5->IFG &= ~BIT1;
                        setalarm_pressed=0;
                    }
            }

        P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);
       }
    if(P5->IFG & BIT2)//ON/OFF/UP BUTTON RED
       {
        if(alarm_status==0)
                {
                    alarm_status= 1;
                }
                else
                {
                    alarm_status= 0;
                }
        P5->IFG &= ~(BIT0|BIT1|BIT4|BIT6|BIT7);
       }
    if(P5->IFG & BIT4)//SNOOZE/DOWN BLUE
       {
      if(snooze=0)
      {
        command(0x96);
        for(i=0;i<6;i++)
        {
            data(snoozebut[i]);
        }
        alarmmin=min+10;
        command(0x97);
        data(blank[0]);
        snooze=1;
      }

        P5->IFG &= ~(BIT0|BIT1|BIT2|BIT6|BIT7);
       }
    if(P5->IFG & BIT6)//CHANGE CLOCK SPEED 1 SECOND REAL TIME = 1 SECOND ALARM CLOCK TIME
       // WHITE
        {
        speedset=0;
        P1->OUT ^=BIT0;
        P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT7);
        }
    if(P5->IFG & BIT7)//CHANGE CLOCK SPEED 1 SECOND REAL TIME = 1 MINUTE ALARM CLOCK TIME
      //YELLOW
        {
        speedset=1;
        P1->OUT ^=BIT0;
        P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6);
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
    __delay_cycles(3000);
    P4-> OUT = data;
    P4-> OUT = 0;
}
void command(unsigned char command)  //function to write commands to the LCD
{
    nibblewrite(command & 0xF0, 0);
    nibblewrite(command << 4, 0);

    if (command < 4)
    {
        __delay_cycles(12000);
    }
    else
    {
        __delay_cycles(3000);
    }
}
void data(unsigned char data)  //function to write data to the LCD
{
    nibblewrite(data & 0xF0, RS);
    nibblewrite(data <<4 , RS);

    __delay_cycles(3000);
}

//-----------time functions -----------------


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
    int ten;
    int single;
    int decimal;
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


           temp[8]=(tempFint/10) + '0';
           ten=(tempFint/10);
           temp[9]=(tempFint-(ten*10))+'0';
           single=(tempFint-(ten*10));
           temp[11]=((tempF-(ten*10)-(single))*10)+'0';
           decimal=(tempF-(ten*10)-(single));

           command(0xD0);
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

void noisesetup(void)
{
        P6->SEL0 |= (BIT7);
        P6->SEL1 &= ~(BIT7);
        P6->DIR |= (BIT7);
        P6->OUT &= ~(BIT7);
        TIMER_A2->CCR[0] = 999;  //1000 clocks = 0.333 ms.  This is the period of everything on Timer A0.  0.333 < 16.666 ms so the on/off shouldn't
                                 //be visible with the human eye.  1000 makes easy math to calculate duty cycle.  No particular reason to use 1000.
        TIMER_A2->CCTL[4] = 0b0000000011100000;//using P6.7 which is TimerA2.4
        TIMER_A2->CCR[4]= 0;
        //The next line turns on all of Timer A2.  None of the above will do anything until Timer A2 is started.
        TIMER_A2->CTL = 0b0000001000010100;  //up mode, smclk, taclr to load.  Up mode configuration turns on the output when CCR[1] is reached
                                             //and off when CCR[0] is reached. SMCLK is the master clock at 3,000,000 MHz.  TACLR must be set to load
                                             //in the changes to CTL register.
}
void alarmgooff(void)
{
    while(snooze==0)
    {
    TIMER_A2->CCR[4]=499;
    __delay_cycles(3000000);
    TIMER_A2->CCR[4]=0;
    __delay_cycles(3000000);
    }
}
