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

    void butt_init(void);//initializes all 6 buttons as interrupts
    void TA2setup(void);//set up the sounder speaker and LEDs
    void alarmgooff(void);//function for when the alarm goes off
//flags used in functions
    int settime_pressed=0;
    int setalarm_pressed=0;
    int alarm_status=0;//alarm starts off
    int lightson=0;//lights start off
    int alarmfullsec = 0;
    int timefullsec= 0;
    int secflag = 0;
    int minflag = 0;
    int hourflag = 0;
    int alarmsecflag = 0;
    int alarmminflag = 0;
    int alarmhourflag = 0;
    int amflag=1;//start in am for time
    int pmflag=0;
    int alarmamflag=1;//start in am for alarm
    int alarmpmflag=0;
    int snooze=0;
    int speedset=0;
    int timeout;
//LCD Function Declarations
    void nibblewrite(unsigned char data, unsigned char control);
    void command(unsigned char command);
    void data(unsigned char data);
    void LCDinit(void);
    void SysTick_Init_interrupt(void);
    void init_display_screen(void);
//ADC Function Declarations
    void settemp(void);
    void PortADC_init(void);
    void ADC14_init(void);


    char INPUT_BUFFER[BUFFER_SIZE];
    uint8_t storage_location = 0; // used in the interrupt to store new data
    uint8_t read_location = 0; // used in the main application to read valid data that hasn't been read yet
    char snoozebut[] = "SNOOZE";//declaration of arrays
    char alarmstatusoff[] = "ALARM OFF";
    char alarmstatuson[] = "ALARM  ON";
    char wordtime[] = "TIME";
    char alarmtime[] = "ALARM";
    char am[] = "AM";
    char pm[] = "PM";
    char blank[] = " ";
    char seconds[60];
    char minutes[60];
    char hours[60];
    char alarmminutes[60];
    char alarmhours[60];
    //what to set the initial times at for alarm and time
    int sec = 0;
    int min = 0;
    int hour = 1;  //start at hour 1
    int  alarmmin = 6;//what to start alarm minutes at
    int  alarmhour = 1;//what to start alarm hours at
    int intense=99;//based on equation, initial intensity after lights turn on is 1%
//These flags will allow the program to determine if it only needs to print the ones digit
//or if it needs to print both tens and ones digits for numbers greater than 10


void main(void)
{

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    P1 -> SEL1 &= ~BIT0;  // initialize on board red LED for visual representation of time speed changing
    P1 -> SEL0 &= ~BIT0;
    P1 -> DIR |= BIT0;
    P1 -> OUT &= ~BIT0;

    int i,j; //loop integers
    TA2setup();//initialization of sounder speaker
    butt_init();//initialization of 6 push buttons as interrupts
    SysTick_Init_interrupt();    //initialization of SysTick interrupt
    LCDinit();                   //initialization of LCD
    PortADC_init();              //initialization of ADC Ports
    ADC14_init();                //initialization of ADC
    NVIC->ISER[0] = 1 << ((ADC14_IRQn) & 31);   //enables ADC interrupt in  NVIC
    __delay_cycles(30000);//delay 10 ms in order to make sure all initializes occur

    TIMER32_1->LOAD = 3000000-1;//timer 32 is counting to a second each time
    TIMER32_1->CONTROL = 0xC2; /* no prescaler, periodic wrapping mode, disable interrupt, 32-bit timer. */

    __enable_irq();                  /* global enable IRQs */

    while(1)//infinite while loop
    {
            settemp();//have set temp sensor at start of while loop
            command(0x80);  /* set cursor at beginning of first line */
                 for(j=0; j<4;j++)
                 {
                 data(wordtime[j]); //prints TIME to line 1
                 }

                 if(sec>=60)  //this section checks if the seconds should roll into a minute
                 {
                      sec = 0;//resets seconds to 0
                      seconds[0] = '0';//resets the array values
                      seconds[1] = '0';
                      secflag = 0;//secflag = 0 as the seconds are now a single digit
                      min = min + 1;//adds 1 to minutes
                 }
                 if(sec>=10)
                 {
                      secflag=1;//secflag = 1 as it is now a 2 digit number for the LCD printing
                 }

                 if(min>=60) //this section checks if minutes should roll into an hour
                 {
                     min = 0;//resets minutes to 0
                     minutes[0] = '0';//resets the array values
                     minutes[1] = '0';
                     minflag = 0;//same as secflag as amount of digits are 1 instead of 2
                     hour = hour +1;//adds 1 to hours
                 }
                 if(min>=10)
                 {
                     minflag = 1;//minflag = 1 as it is now a 2 digit number for the LCD printing
                 }
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
                 if(hour >= 10)//same as sec and min with changing the LCD display from 1 digit to 2
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
//checks if any conversions need to occur
                 if(alarmmin>=60) //this section checks if alarm minutes should roll into an hour
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
                 //printing the alarm time to the LCD
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

if((alarmamflag==amflag) | (alarmpmflag==pmflag))//first checks if am or pm match on alarm and clock time
{
if(alarmhour==hour)//next checks the hours
  {
    if(alarmmin<=(min+5))//turns on the Wake Up LEDs if clock time has 5 minutes or less to the alarm time
    {
        if(alarm_status==1)//alarm has to be set to ON for lights to turn on
        {
            TIMER_A2->CCR[3]=999-(intense*10);//starts at 1% intensity
            lightson=1;//sets flag to on for if statement to calculate how intense the LEDs should be
        }
    if(alarmmin==min)//if minutes match
    {
        if(alarm_status==1)//turn alarm flag on for snooze interrupt
        {

                snooze=0;//clear snooze flag to say snooze has not been pushed
                alarmgooff();//go into alarm going off function

            }
        }
    }

  }
}
if(lightson==1)//if lights are turned on
{
    alarmfullsec=(alarmmin*60);//sets variable to total seconds alarm is set to
    timefullsec=((min*60)+sec);//sets variable to total seconds time is at
    intense=(alarmfullsec-timefullsec)/3;//intense updates .33% stronger each second so it is 1% stronger every 3 seconds
    if(intense>=100)//prevents intensity from being too high
       {
         intense=100;
       }
    TIMER_A2->CCR[3]=(100-intense)*9.99;//LED brightness is small to start and slowly increases
    //multiplied by 9.99 because CCR[0] is 999 so it allows for convert from intensity(1%-100%) to correct duty cycle
}
if(speedset==0)//if normal clock speed is selected
{
    sec = sec + 1;//seconds increase by 1 every real world second/ every time through while loop
}
if(speedset==1)//if 60x faster clock speed selected
{
    min = min + 1;//minutes increase by 1 every real world second/ every time through while loop
}
                 while((TIMER32_1 -> RIS & 1) == 0); //waits 1 second until interrupt flag is set
                 TIMER32_1 -> INTCLR = 0; //clears interrupt flag

    }
}




//------------- button interrupt functions -----------------

void butt_init(void) //button initializations
{
    /* configure P5.0, 5.1, 5.2, 5.4, 5.6 5.7, for button inputs */
    P5->SEL1 &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);    /* configure pins as simple I/O */
    P5->SEL0 &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);
    P5->DIR &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);     /* pins set as input */
    P5->REN |= (BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);      /* pins pull resistor enabled */
    P5->OUT |= (BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);      /* Pull up/down is selected */
    P5->IES |= (BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);      /* make interrupt trigger on high-to-low transition */
    P5->IFG = 0;          /* clear pending interrupt flags */
    P5->IE |= (BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);       /* enable interrupt from pins */


    NVIC_EnableIRQ(PORT5_IRQn);      /* enable interrupt in NVIC */

}
/*------set time and set alarm interrupts explained-------*/
/*For the first interrupt, the user is able to alter the time that the clock is at.
 * The first time the button is pressed, the hours are available to be changed using the 3rd and 4th buttons.
 * The 3rd button has the hours increase by one and the 4th decreases it by one.
 * After the hours go above 12 or below 1, the clock switches from am to pm or pm to am depending on what it was before
 * and changes them from 12 to 1 or 1 to 12
 * When the hours are good, press the first button again to move to changing the minutes.
 * The 3rd and 4th buttons do the same thing with 3rd raising minutes by one and 4th decreases by one.
 * When the minutes go past 59 or under 00, add an hour or subtract an hour and it will update after the
 * set time button is pressed for a 3rd time and the clock starts going again at the new time.
 *
 * The second button and interrupt sets the alarm and function the same as the clock timer with starting in hours
 * then switching to minutes after pressing the second button again and finalizing the alarm time with a third press of
 * the alarm set button.
 * This interrupt also uses the 3rd and 4th button to increment and decrement respectively, each by 1
 */
void PORT5_IRQHandler(void) //IRQ Handler for button interrupts
{
    int i;

    if(P5->IFG & BIT0)//SET TIME BUTTON BLACK
       {
        P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);//clears all flags just in case other buttons flags were still on
        settime_pressed++;//changes flag to 1 from 0 so first while loop is entered
        int i;
           while(settime_pressed==1)//after 1 button press
           {
                   if (P5->IFG & BIT2)//if 3rd button pressed
                   {
                       P5->IFG &= ~BIT2;//clears interrupt flag immediately so it can be pressed repeatedly in loop
                       hour=hour+1;//add an hour
                       if(hour > 12) //this section allows to change between AM and PM
                         {
                             hour = 1;
                             hourflag = 0;
                             if(amflag==1)//to swtich between am and pm
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
                       if (amflag==1)//if printing am or pm
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
                                     command(0x86);//prints hours 1-9 next to ':' and removes character in front in case going from 10->9 or 12->1
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
           if (P5->IFG & BIT4)//if decrementing from 4th button being pushed
              {
               P5->IFG &= ~BIT4;//clears interrupt flag immediately so it can be pressed repeatedly in loop
                   hour=hour-1;//hours go down by 1
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
           if(P5->IFG & BIT0)//if 1st button pressed again move to next while loop
           {
               P5->IFG &= ~BIT0;//clears flag
               settime_pressed++;
           }
           command(0x85);//moves cursor to minutes
           }
          while(settime_pressed==2)//stays in loop till set time button pressed again
          {
              if (P5->IFG & BIT2)
                           {
                            P5->IFG &= ~BIT2;//clears interrupt flag immediately so it can be pressed repeatedly in loop
                            min++;//add one to minute for clock time
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
                                          P5->IFG &= ~BIT4;//clears interrupt flag immediately so it can be pressed repeatedly in loop
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
           P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);//clear all interrupts at end of set time interrupt to exit handler

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
                     P5->IFG &= ~BIT2;//clears interrupt flag immediately so it can be pressed repeatedly in loop
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
                 P5->IFG &= ~BIT4;//clears interrupt flag immediately so it can be pressed repeatedly in loop
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
             command(0x95);
             }
            while(setalarm_pressed==2)
            {
                if (P5->IFG & BIT2)
                             {
                              P5->IFG &= ~BIT2;//clears interrupt flag immediately so it can be pressed repeatedly in loop
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
                        P5->IFG &= ~BIT4;//clears interrupt flag immediately so it can be pressed repeatedly in loop
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
                           if(alarmmin<0)//checks if user lowers the minutes past 0 so that hours go back one and minutes go to 59
                           {
                               alarmhour= alarmhour-1;
                               alarmmin=59;
                               alarmminflag = 1;//changes flag as well for 2 digit number
                           }
                      command(0x97);
                      data(':');
                      command(0x98);
                      sprintf(alarmminutes, "%d", alarmmin); //displaying minutes and sets alarm minutes array equal
                                                             //to the value of the alarm minutes/ converts int to string
                      if(alarmminflag == 0)//if minutes are less than 10
                      {
                          command(0x98);//print leading 0
                          data('0');
                          command(0x99);//moves cursor over
                          data(alarmminutes[0]);//prints first value only of array
                      }
                      else if(alarmminflag == 1)//if minutes are 10 or more
                      {
                           for(i=0;i<2;i++)//prints first 2 values of alarmminutes array to LCD
                           {
                           data(alarmminutes[i]);
                           }
                      }
                }
                command(0x99);//sets cursor next to the minutes on alarm so user sees where they are changing
                if(P5->IFG & BIT1)//if alarmset button is pressed for 3rd time
                    {
                        P5->IFG &= ~BIT1;//clear flag
                        setalarm_pressed=0;//sets alarm button pressed flag to 0 to get out of while loop
                    }
            }

        P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7);//clear all interrupts at end of set alarm interrupt to exit handler
       }
    if(P5->IFG & BIT2)//ON/OFF/UP BUTTON RED
       {
        if(alarm_status==0)
            {
                alarm_status= 1;//if alarm was off is now set to on
            }
        else
            {
                alarm_status= 0;//if alarm wasn't set to off before it is now set to off
            }
        P5->IFG &= ~(BIT0|BIT1|BIT4|BIT6|BIT7);//clear all other flags in case they are on
       }
    if(P5->IFG & BIT4)//SNOOZE/DOWN BLUE
        {
          if(snooze==0)//if snooze = 0, alarm is going off, otherwise it does nothing when pressed
          {
            command(0xCA);//prints snooze on alarm status line past ON/OFF
            for(i=0;i<6;i++)
            {
            data(snoozebut[i]);
            }
            alarmmin=min+10;
            __delay_cycles(3000000);//keep snooze on screen for one second
            //clears the word snooze off of the LED after a second
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
            snooze=1;//snooze flag = 1 to get out of alarm go off function and return to main
            lightson=0;//turn off LEDs flag after snooze is pressed so they turn off as alarm and time have 10 minute difference
            //and the lights won't turn on until the difference is 5 minutes or less
            TIMER_A2->CCR[3]=0;//turn lights off
          }

            P5->IFG &= ~(BIT0|BIT1|BIT2|BIT6|BIT7);//clear all other flags in case they are on
       }
    if(P5->IFG & BIT6)//CHANGE CLOCK SPEED 1 SECOND REAL TIME = 1 SECOND ALARM CLOCK TIME
       // WHITE
        {
            speedset=0;//change speed flag to 0 to have clock only increase by 1 second for each real world second
            P1->OUT &=~ BIT0;//on board LED is off if normal clock speed is happening
            P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT7);//clear all other flags in case they are on
        }
    if(P5->IFG & BIT7)//CHANGE CLOCK SPEED 1 SECOND REAL TIME = 1 MINUTE ALARM CLOCK TIME
      //YELLOW
        {
            speedset=1;//change speed flag to 1 to have clock increase by 1 minute for each real world second
            P1->OUT |=BIT0;//turn on on board LED
            P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6);//clear all other flags in case they are on
        }

    P5->IFG &= ~(BIT0|BIT1|BIT2|BIT4|BIT6|BIT7); /* clear the interrupt flags before return */
}

//----------LCD functions----------------------

void LCDinit(void) //LCD info is in Lab 6 pt. 2 *************
{
    P4->DIR = 0xFF;  //set all of pin 4 ports as output
    __delay_cycles(30000); //using delay cycles here because we want to use the SysTick for the Temp. Sensor interrupt
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
//-------------temp sensor functions-------------
void settemp(void)
{
    int i;
    char temp[]= "TEMP is XX.X F";//array to be altered with new values
    float volt;
    float mvolt;
    float tempC;
    float tempF;
    int tempFint;
    int ten;
    int single;
    ADC14->CTL0 |= ADC14_CTL0_SC;    //starts conversion
    while(!(ADC14->IFGR0));         //waits for it to complete
    float result = ADC14->MEM[0];         //get value from the ADC
    volt = (result*3.3)/16384;      //converts raw value to voltage

           mvolt = volt*1000;        //converts volts to mV

           tempC = (mvolt-500)/10;     //finds temp in Celsius
           tempF = (tempC * 9);  //converts celsius to fahrenheit
           tempF = (tempF/5);
           tempF = (tempF + 32);
           tempFint=tempF;//converts float to int for easier conversion to char
           if(timeout)
           {   //prints the RAW, converted, C and F values
           printf("\t RAW = %f\n\tVOLTAGE = %f\n\tCelcius = %f \n\tFarenheit = %f\n\n", result, volt, tempC, tempF);//used as check that data was being received
           timeout = 0;  //clears interrupt flag
           }

           //temp[8] is X in 10s spot, temp[9] is X in 1s spot, temp[11] is after decimal point and is X in decimal to the tenths spot
           temp[8]=(tempFint/10) + '0';//takes integer of temp. in f and separates the 10 digit from the single digit
           //adds '0' to convert the int to char of same number
           ten=(tempFint/10);//saves same int for later math
           temp[9]=(tempFint-(ten*10))+'0';//saves the single digit in the ones place
           single=(tempFint-(ten*10));//saves singles digit for math
           temp[11]=((tempF-(ten*10)-(single))*10)+'0';//uses the saved math digits with the float temp. in f
           //to get the decimal value alone and then multiplies by 10 to be a singles digit
           //that '0' can be added to for correct char
           command(0xD0);//sets cursor to start of 4th line
           for(i=0;i<14;i++)
           {
               data(temp[i]);//prints the entire temperature string with the 3 Xs replaced with the number characters
               //from the equation above
           }

}
void PortADC_init(void)
 {
    P5->SEL0 |= BIT5;   //sets pin 5.5 as A0 input
    P5->SEL1 |= BIT5;   //sets pin to receive from temp sensor
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
     SysTick -> LOAD = 750000;   //Load top value for 250 Ms
     SysTick -> VAL = 0;             //clears value on SysTick
     SysTick -> CTRL = 0x00000007;   //enables SysTick and interrupts
 }

 void SysTick_Handler(void)
 {
    timeout = 1;//resets SysTick flag when interrupt occurs
 }

void TA2setup(void)
{
        P6->SEL0 |= (BIT6|BIT7);//initialize pins 6.6 and 6.7 for TIMER_A2 and as outputs for PWM
        P6->SEL1 &= ~(BIT6|BIT7);
        P6->DIR |= (BIT6|BIT7);
        P6->OUT &= ~(BIT6|BIT7);
        TIMER_A2->CCR[0] = 999;  //1000 clocks = 0.333 ms.  This is the period of everything on Timer A2.  0.333 < 16.666 ms so the on/off shouldn't
                                 //be visible with the human eye.  1000 makes easy math to calculate duty cycle.  No particular reason to use 1000.
        TIMER_A2->CCTL[3] = 0b0000000011100000;//using P6.6 which is TimerA2.3
        TIMER_A2->CCR[3]= 0;//LED starts off
        TIMER_A2->CCTL[4] = 0b0000000011100000;//using P6.7 which is TimerA2.4
        TIMER_A2->CCR[4]= 0;//Speaker starts off
        //The next line turns on all of Timer A2.  None of the above will do anything until Timer A2 is started.
        TIMER_A2->CTL = 0b0000001000010100;  //up mode, smclk, taclr to load.  Up mode configuration turns on the output when CCR[1] is reached
                                             //and off when CCR[0] is reached. SMCLK is the master clock at 3,000,000 MHz.  TACLR must be set to load
                                             //in the changes to CTL register.
}
void alarmgooff(void)//function to have alarm go off
{
    while(snooze==0)//stays in loop until snooze interrupt occurs and has snooze = 1
    {
    TIMER_A2->CCR[3]=999;//have LEDs at full brightness
    TIMER_A2->CCR[4]=499;//turn on speaker at 50% duty cycle
    __delay_cycles(3000000);//wait a second
    TIMER_A2->CCR[4]=0;//turn off the speaker
    __delay_cycles(3000000);//wait a second
    }
}
