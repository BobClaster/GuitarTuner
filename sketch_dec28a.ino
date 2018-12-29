//sine wave freq detection with 38.5kHz sampling rate and interrupts
//by Amanda Ghassaei
//https://www.instructables.com/id/Arduino-Frequency-Detection/
//July 2012

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
*/
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
//clipping indicator variables
boolean clipping = 0;

//data storage variables
byte newData = 0;
byte prevData = 0;

//freq variables
unsigned int timer = 0;//counts period of wave
unsigned int period;
int frequency;

void setup(){
  
  Serial.begin(9600);
  lcd.begin(16, 2);
  
  pinMode(13,OUTPUT);//led indicator pin
  
  cli();//diable interrupts
  
  //set up continuous sampling of analog pin 0
  
  //clear ADCSRA and ADCSRB registers
  ADCSRA = 0;
  ADCSRB = 0;
  
  ADMUX |= (1 << REFS0); //set reference voltage
  ADMUX |= (1 << ADLAR); //left align the ADC value- so we can read highest 8 bits from ADCH register only
  
  ADCSRA |= (1 << ADPS2) | (1 << ADPS0); //set ADC clock with 32 prescaler- 16mHz/32=500kHz
  ADCSRA |= (1 << ADATE); //enabble auto trigger
  ADCSRA |= (1 << ADIE); //enable interrupts when measurement complete
  ADCSRA |= (1 << ADEN); //enable ADC
  ADCSRA |= (1 << ADSC); //start ADC measurements
  
  sei();//enable interrupts
}

ISR(ADC_vect) {//when new ADC value ready

  prevData = newData;//store previous value
  newData = ADCH;//get value from A0
  if (prevData < 127 && newData >=127){//if increasing and crossing midpoint
    period = timer;//get period
    timer = 0;//reset timer
  }
  
  
  if (newData == 0 || newData == 1023){//if clipping
    PORTB |= B00100000;//set pin 13 high- turn on clipping indicator led
    clipping = 1;//currently clipping
  }
  
  timer++;//increment timer at rate of 38.5kHz
}

int what_is_the_note(int frq) {
  int chord[6] = {330, 247, 196, 147, 110, 82};
  int nota = 0;
  int sub_frq = 1000;

  for (int i=0; i<6; i++) {
    int sub = abs(chord[i] - frq);
    if (sub < sub_frq) {
      sub_frq = sub;
      nota = i;
    }
  }
  
  return nota;
}

void loop(){
  if (clipping){//if currently clipping
    PORTB &= B11011111;//turn off clippng indicator led
    clipping = 0;
  }
  char nota = "";
  

  frequency = 38462/period/15.865;//timer rate/period

  if(what_is_the_note(frequency)==5) nota = 'E';
  if(what_is_the_note(frequency)==4) nota = 'A';
  if(what_is_the_note(frequency)==3) nota = 'd';
  if(what_is_the_note(frequency)==2) nota = 'g';
  if(what_is_the_note(frequency)==1) nota = 'b';
  if(what_is_the_note(frequency)==0) nota = 'e';
  
  lcd.clear();
  lcd.print(nota);
//  Serial.print(frequency);
//  Serial.println(" hz");
  
  delay(100);
}
