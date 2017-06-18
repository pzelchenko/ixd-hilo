//
// Hi-Lo
// IxD Team -- Jayoung, Marcus, Peter, Tommy, Yoga
// Two-button steering-wheel hi-lo game (interface demonstration)
// Prof. Gu Zhenyu
// 2017 June
//
// Plays a game of hi-lo, computer guesses from 1 to 99 using a divider heuristic
// Input uses only two buttons, output uses voice
//

// THINGS TO DO TO IMPROVE:
// (1) Use 4K+ audio (currently 2K), and put on SD, OR do Chinese at about 3K?
// (2) Adjust number with floatguess to reduce number of tries
// (3) Allow user to adjust how "smart" the computer is
// (4) Allow taking turns, user to guess numbers
// (5) Implement other game choices

// audio output, using only PROGMEM (32K limit); see documentation below
#include <PCM.h>

// 4k data of only 01-20 takes up 30986 bytes (96%)
// 2k data full range with code takes almost 32K (98%), but sound is it's really unclear
#include "toohigh.h"
#include "toolow.h"
#include "only.h"
#include "tries.h"

#include "01.h"
#include "02.h"
#include "03.h"
#include "04.h"
#include "05.h"
#include "06.h"
#include "07.h"
#include "08.h"
#include "09.h"
#include "10.h"
#include "11.h"
#include "12.h"
#include "13.h"
#include "14.h"
#include "15.h"
#include "16.h"
#include "17.h"
#include "18.h"
#include "19.h"

#include "20.h"
#include "30.h"
#include "40.h"
#include "50.h"
#include "60.h"
#include "70.h"
#include "80.h"
#include "90.h"

#define SAMPLE_RATE 2000  // from PCM.c; this was reduced from 8000, so quality is very poor

const int lButtonPin = 7;     // left button
const int rButtonPin = 8;     // right button
const int ledLeft    = 2;     // left (0) LED
const int led1       = 3;     // second (1) LED
const int led2       = 4;     // second (1) LED
const int led3       = 5;     // second (1) LED
const int ledRight   = 6;     // right LED

// special sound codes (1-20 = directly; 21-27 = 30 to 90; 28-31 are special sounds, below)
// 32 and up are for expansion
const int soundToolow  = 28;  // "too low" sound
const int soundToohigh = 29;  // "too high" sound
const int soundOnly    = 30;  // "only" sound
const int soundTries   = 31;  // "tries" sound

int   guess = 0;              // guessing a random number between 1 and 99
int   guesses = 0;            // total number of guesses it took
int   guessed;                // 1 if sound for this guess has been output; 0 otherwise
float mult;                   // half-multiplier (e.g., 1st round = 50, 2nd round = 25, 3rd round = 12.5, etc.)

void playSound(int sound) {
  int dl;
  int delaytime;
  
  switch (sound) {
    case  1: dl = s_01_length; startPlayback(s_01_data, dl); break;
    case  2: dl = s_02_length; startPlayback(s_02_data, dl); break;
    case  3: dl = s_03_length; startPlayback(s_03_data, dl); break;
    case  4: dl = s_04_length; startPlayback(s_04_data, dl); break;
    case  5: dl = s_05_length; startPlayback(s_05_data, dl); break;
    case  6: dl = s_06_length; startPlayback(s_06_data, dl); break;
    case  7: dl = s_07_length; startPlayback(s_07_data, dl); break;
    case  8: dl = s_08_length; startPlayback(s_08_data, dl); break;
    case  9: dl = s_09_length; startPlayback(s_09_data, dl); break;
    case 10: dl = s_10_length; startPlayback(s_10_data, dl); break;
    case 11: dl = s_11_length; startPlayback(s_11_data, dl); break;
    case 12: dl = s_12_length; startPlayback(s_12_data, dl); break;
    case 13: dl = s_13_length; startPlayback(s_13_data, dl); break;
    case 14: dl = s_14_length; startPlayback(s_14_data, dl); break;
    case 15: dl = s_15_length; startPlayback(s_15_data, dl); break;
    case 16: dl = s_16_length; startPlayback(s_16_data, dl); break;
    case 17: dl = s_17_length; startPlayback(s_17_data, dl); break;
    case 18: dl = s_18_length; startPlayback(s_18_data, dl); break;
    case 19: dl = s_19_length; startPlayback(s_19_data, dl); break;
    case 20: dl = s_20_length; startPlayback(s_20_data, dl); break;
    
    // 21-27 = sounds for 30-90
    case 21: dl = s_30_length; startPlayback(s_30_data, dl); break;
    case 22: dl = s_40_length; startPlayback(s_40_data, dl); break;
    case 23: dl = s_50_length; startPlayback(s_50_data, dl); break;
    case 24: dl = s_60_length; startPlayback(s_60_data, dl); break;
    case 25: dl = s_70_length; startPlayback(s_70_data, dl); break;
    case 26: dl = s_80_length; startPlayback(s_80_data, dl); break;
    case 27: dl = s_90_length; startPlayback(s_90_data, dl); break;

    // 28-31 = additional sounds
    case 28: dl = s_toolow_length; startPlayback(s_toolow_data, dl); break;
    case 29: dl = s_toohigh_length; startPlayback(s_toohigh_data, dl); break;
    case 30: dl = s_only_length; startPlayback(s_only_data, dl); break;
    case 31: dl = s_tries_length; startPlayback(s_tries_data, dl); break;

  }
  delaytime = ((float)dl/SAMPLE_RATE) * 1000;
  delay(delaytime);
}

void LedPlayAll(int state) {

  // state = LOW or HIGH
  digitalWrite(ledLeft, state);
  digitalWrite(led1, state);
  digitalWrite(led2, state);
  digitalWrite(led3, state);
  digitalWrite(ledRight, state);
  
}

void setup() {
  // put your setup code here, to run once:

  pinMode(lButtonPin, INPUT);
  pinMode(rButtonPin, INPUT);

  pinMode(ledLeft, OUTPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(ledRight, OUTPUT);

  guess = 50;
  mult = 25.0;
  guesses = 0;
  
//  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:

  int i;
  int sound;
  int remain;
  int lButtonState = 0;
  int rButtonState = 0;


  // put out a guess
  if (guessed == 0) {
    if (guess > 20) {
      playSound(int(guess/10.0) + 18);              // for example, if 36, then play the "30"
      remain = int(guess - int(guess/10.0) * 10);   // for example, if 36, then play the "6"
      if (remain > 0) playSound(remain);
    } else {
      playSound(guess);
    }
    guessed = 1;
  }

  lButtonState = digitalRead(lButtonPin);
  rButtonState = digitalRead(rButtonPin);

  digitalWrite(ledLeft, LOW);
  digitalWrite(ledRight, LOW);

  if ((lButtonState == HIGH) || (rButtonState == HIGH)) {

    if ((lButtonState == HIGH && rButtonState == LOW) || (rButtonState == HIGH && lButtonState == LOW)) {    
      delay(50);  // debounce
      lButtonState = digitalRead(lButtonPin);
      rButtonState = digitalRead(rButtonPin);
      if (lButtonState == HIGH) digitalWrite(ledLeft, HIGH); 
      if (rButtonState == HIGH) digitalWrite(ledRight, HIGH);

    }
    
    guesses++;
   
    // both buttons means guessed correctly!
    // say "that took only ... 6 ... tries!"
    if ((lButtonState == HIGH) && (rButtonState == HIGH)) {
      playSound(soundOnly);
      playSound(guesses);
      playSound(soundTries);

      // reset all of the indicators and play again
      guessed = 0;
      guess = 50;
      guesses = 0;
      mult = 25.0;
      
      // play a little light show, also buys time for a new game
      for (i = 0; i < 5; i++) {
        LedPlayAll(HIGH);
        delay(200);
        LedPlayAll(LOW);
        delay(200);
      }
    }
    else
    {

      // left button means too low, try a higher number
      if (lButtonState == HIGH) {
        Serial.println(mult);
        playSound(soundToolow);
        guess = int(guess + mult);
        if (guess > 99) guess = 99;
        guessed = 0;
      }
      else
      
      // right button means too high, try a lower number
      if (rButtonState == HIGH) {
         Serial.println(mult);
        playSound(soundToohigh);
        guess = int(guess - mult);
        if (guess < 1) guess = 1;
        guessed = 0;
      }
      mult = mult/2.0;
      if (mult < 1) mult = 1;
    }
  }  
}

