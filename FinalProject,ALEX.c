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
int lightson=0;
int intense=99;
int alarmfullsec=0;
int timefullsec=0;
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
    int  alarmmin = 6;
    int  alarmhour = 1;
    int  alarmsecflag = 0;
    int  alarmminflag = 0;
    int  alarmhourflag = 0;
    int  amflag=1;
    int  pmflag=0;
    int alarmamflag=1;
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
    __delay_cycles(30000);

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
                 else if (min<10)
                 {
                     minflag = 0;
                 }
                 if(min<0)
                 {
                     hour= hour-1;
                     min=59;
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
                     command(0x86);
                     data(hours[0]);
                 }

                 else if(hourflag == 1)
                 {
                     command(0x85);
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
                     command(0x88);
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
                 command(0x90);  /* set cursor at beginning of third line */
                 for(j=0; j<5;j++)
                 {
                 data(alarmtime[j]); //prints ALARMTIME to line 1
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
                        if(alarmmin<0)
                         {
                             alarmhour= alarmhour-1;
                             alarmmin=59;
                             alarmminflag = 1;
                         }

                        if(hour > 12) //this section allows to change between AM and PM
                        {
                            alarmhour = 1;
                            alarmhourflag = 0;
                            if(alarmamflag==1)
                            {
                                alarmpmflag=1;
                                alarmamflag=0;
                            }
                            else  if(alarmpmflag==1)
                            {
                                alarmpmflag=0;
                                alarmamflag=1;
                            }

                        }
                        if(alarmhour >= 10)
                        {
                            alarmhourflag = 1;
                        }
                 //printing the clock time to the LCD
                 command(0x95);
                 sprintf(alarmhours,"%d", alarmhour); //displaying hours
                 if(alarmhourflag == 0)
                 {
                     command(0x96);
                     data(alarmhours[0]);
                 }

                 else if(alarmhourflag == 1)
                 {
                     command(0x95);
                     for(i=0;i<2;i++)
                     {
                         data(alarmhours[i]);
                     }
                 }

                 command(0x97);
                 data(':');
                 command(0x98);
                 sprintf(alarmminutes, "%d", alarmmin); //displaying minutes
                 if(alarmminflag == 0)
                 {
                     command(0x98);
                     data('0');
                     command(0x99);
                     data(alarmminutes[0]);
                 }
                 else if(alarmminflag == 1)
                 {
                      for(i=0;i<2;i++)
                      {
                          data(alarmminutes[i]);
                      }
                 }

                 if (alarmamflag==1)
                              {
                                  command(0x9B);
                                  for(i=0;i<2;i++)
                                  {
                                      data(am[i]);
                                  }
                                  alarmpmflag=0;
                              }
                  if (alarmpmflag==1)
                              {
                                  command(0x9B);
                                  for(i=0;i<2;i++)
                                  {
                                      data(pm[i]);
                                  }
                                  alarmamflag=0;
                              }

if((alarmamflag==amflag) | (alarmpmflag==pmflag))
{
if(alarmhour==hour)
  {
    if(alarmmin<=(min+5))
    {
        if(alarm_status==1)
        {
            TIMER_A2->CCR[3]=999-(intense*10);
            lightson=1;
        }
    if(alarmmin==min)
    {
        if(alarm_status==1)
        {

                snooze=0;
                alarmgooff();

            }
        }
    }

  }
}
if(lightson==1)
{
    alarmfullsec=(alarmmin*60)+alarmsec;
    timefullsec=((min*60)+sec);
    intense=(alarmfullsec-timefullsec)/3;
    if(intense>=100)
       {
         intense=100;
       }
    TIMER_A2->CCR[3]=(100-intense)*9.99;

}
if(speedset==0)
{
    sec = sec + 1;
}
if(speedset==1)
{
    min = min + 1;
}
                 while((TIMER32_1 -> RIS & 1) == 0); //waits 1 second until interrupt flag is set
                 TIMER32_1 -> INTCLR = 0; //clears interrupt flag

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
                     if(amflag==1)
                     {
                        pmflag=1;
                        amflag=0;
                     }
                     else if(pmflag==1)
                     {
                         pmflag=0;
                         amflag=1;
                     }
                 }
               if(hour >= 10)
                 {
                     hourflag = 1;
                 }
               else if(hour<10)
                 {
                      hourflag = 0;
                 }
               if (amflag==1)
                            {
                                command(0x8E);
                                for(i=0;i<2;i++)
                                {
                                    data(am[i]);
                                }

                            }
                if (pmflag==1)
                            {
                                command(0x8E);
                                for(i=0;i<2;i++)
                                {
                                    data(pm[i]);
                                }

                            }

               command(0x85);
               sprintf(hours,"%d", hour); //displaying hours
                             if(hourflag == 0)
                             {
                                 command(0x86);
                                 data(hours[0]);
                                 command(0x85);
                                 data(blank[0]);

                             }

                             else if(hourflag == 1)
                             {
                                 command(0x85);
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
                                  hourflag = 1;
                                  if(amflag==1)
                                  {
                                    pmflag=1;
                                    amflag=0;
                                  }
                                 else if(pmflag==1)
                                  {
                                    pmflag=0;
                                    amflag=1;
                                  }
                              }
                  if(hour >= 10)
                    {
                        hourflag = 1;
                    }
                  else if(hour<10)
                  {
                       hourflag = 0;
                  }
                  if (amflag==1)
                               {
                               command(0x8E);
                               for(i=0;i<2;i++)
                               {
                                   data(am[i]);
                               }

                               }
                   if (pmflag==1)
                               {
                               command(0x8E);
                               for(i=0;i<2;i++)
                               {
                                   data(pm[i]);
                               }
                               }

                  sprintf(hours,"%d", hour); //displaying hours
                                if(hourflag == 0)
                                {
                                    command(0x86);
                                    data(hours[0]);
                                    command(0x85);
                                    data(blank[0]);
                                }

                                if(hourflag == 1)
                                {
                                    command(0x85);
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
                               else if(min<10)
                               {
                                   minflag = 0;
                               }
                               if(min<0)
                               {
                                   hour = hour-1;
                                   min = 59;
                                   minflag = 1;
                               }

                          command(0x87);
                          data(':');
                          command(0x88);
                          sprintf(minutes, "%d", min); //displaying minutes
                          if(minflag == 0)
                          {
                              command(0x88);
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
                                             else if(min<10)
                                             {
                                                 minflag = 0;
                                             }
                                             if(min<0)
                                             {
                                                 hour= hour-1;
                                                 min=59;
                                                 minflag = 1;
                                             }

                                        command(0x87);
                                        data(':');
                                        command(0x88);
                                        sprintf(minutes, "%d", min); //displaying minutes
                                        if(minflag == 0)
                                        {
                                            command(0x88);
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
                       if(alarmamflag==1)
                       {
                          alarmpmflag=1;
                          alarmamflag=0;
                       }
                       else if(alarmpmflag==1)
                       {
                         alarmpmflag=0;
                         alarmamflag=1;
                       }
                   }
                 if(alarmhour >= 10)
                   {
                       alarmhourflag = 1;
                   }

                 command(0x95);
                 sprintf(alarmhours,"%d", alarmhour); //displaying hours
                               if(alarmhourflag == 0)
                               {
                                   command(0x96);
                                   data(alarmhours[0]);
                                   command(0x95);
                                   data(blank[0]);
                               }

                               else if(alarmhourflag == 1)
                               {
                                   command(0x95);
                                   for(i=0;i<2;i++)
                                   {

                                       data(alarmhours[i]);
                                   }
                               }
                               if (alarmamflag==1)
                                           {
                                               command(0x9B);
                                               for(i=0;i<2;i++)
                                               {
                                                   data(am[i]);
                                               }

                                           }
                               if (alarmpmflag==1)
                                           {
                                               command(0x9B);
                                               for(i=0;i<2;i++)
                                               {
                                                   data(pm[i]);
                                               }

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
                                    if(alarmamflag==1)
                                    {
                                       alarmpmflag=1;
                                       alarmamflag=0;
                                    }
                                    else if(alarmpmflag==1)
                                    {
                                      alarmpmflag=0;
                                      alarmamflag=1;
                                    }
                                }
                    if(alarmhour >= 10)
                      {
                          alarmhourflag = 1;
                      }
                    if (alarmamflag==1)
                                  {
                                      command(0x9B);
                                      for(i=0;i<2;i++)
                                      {
                                          data(am[i]);
                                      }

                                  }
                    if (alarmpmflag==1)
                                  {
                                      command(0x9B);
                                      for(i=0;i<2;i++)
                                      {
                                          data(pm[i]);
                                      }

                                  }

                    command(0x95);
                    sprintf(alarmhours,"%d", alarmhour); //displaying hours
                                  if(alarmhourflag == 0)
                                  {
                                      command(0x96);
                                      data(alarmhours[0]);
                                      command(0x95);
                                      data(blank[0]);
                                  }

                                  else if(alarmhourflag == 1)
                                  {
                                      command(0x95);
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
                                 if(alarmmin>=10)
                                 {
                                     alarmminflag = 1;
                                 }
                                 else if(alarmmin<10)
                                 {
                                     alarmminflag = 0;
                                 }
                                 if(alarmmin<0)
                                 {
                                     alarmhour= alarmhour-1;
                                     alarmmin=59;
                                     alarmminflag = 1;
                                 }

                            command(0x97);
                            data(':');
                            command(0x98);
                            sprintf(alarmminutes, "%d", alarmmin); //displaying minutes
                            if(alarmminflag == 0)
                            {
                                command(0x98);
                                data('0');
                                command(0x99);
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
                                               if(alarmmin>=10)
                                               {
                                                   alarmminflag = 1;
                                               }
                                               else if(alarmmin<10)
                                               {
                                                   alarmminflag = 0;
                                               }
                                               if(alarmmin<0)
                                               {
                                                   alarmhour= alarmhour-1;
                                                   alarmmin=59;
                                                   alarmminflag = 1;
                                               }
                                          command(0x97);
                                          data(':');
                                          command(0x98);
                                          sprintf(alarmminutes, "%d", alarmmin); //displaying minutes
                                          if(alarmminflag == 0)
                                          {
                                              command(0x98);
                                              data('0');
                                              command(0x99);
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
                            command(0x99);
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
      if(snooze==0)
      {
        command(0xCA);
        for(i=0;i<6;i++)
        {
            data(snoozebut[i]);
        }
        alarmmin=min+10;
        __delay_cycles(3000000);
        command(0xCA);
        data(blank[0]);
        command(0xCB);
        data(blank[0]);
        command(0xCC);
        data(blank[0]);
        command(0xCD);
        data(blank[0]);
        command(0xCE);
        data(blank[0]);
        command(0xCF);
        data(blank[0]);
        snooze=1;
        lightson=0;
        TIMER_A2->CCR[3]=0;
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
        P6->SEL0 |= (BIT6);
        P6->SEL1 &= ~(BIT6);
        P6->DIR |= (BIT6);
        P6->OUT &= ~(BIT6);
        TIMER_A2->CCR[0] = 999;  //1000 clocks = 0.333 ms.  This is the period of everything on Timer A0.  0.333 < 16.666 ms so the on/off shouldn't
                                 //be visible with the human eye.  1000 makes easy math to calculate duty cycle.  No particular reason to use 1000.
        TIMER_A2->CCTL[3] = 0b0000000011100000;//using P6.7 which is TimerA2.4
        TIMER_A2->CCR[3]= 0;
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
    TIMER_A2->CCR[3]=999;
    TIMER_A2->CCR[4]=499;
    __delay_cycles(3000000);
    TIMER_A2->CCR[4]=0;
    __delay_cycles(3000000);
    }
}
