/*
 * (PCM Audio speaker_pcm downloaded from http://playground.arduino.cc/Code/PCMAudio)
 *  
 * speaker_pcm
 *
 * Plays 8-bit PCM audio on pin 11 using pulse-width modulation (PWM).
 * For Arduino with Atmega168 at 16 MHz.
 *
 * Uses two timers. The first changes the sample value 8000 times a second.
 * The second holds pin 11 high for 0-255 ticks out of a 256-tick cycle,
 * depending on sample value. The second timer repeats 62500 times per second
 * (16000000 / 256), much faster than the playback rate (8000 Hz), so
 * it almost sounds halfway decent, just really quiet on a PC speaker.
 *
 * Takes over Timer 1 (16-bit) for the 8000 Hz timer. This breaks PWM
 * (analogWrite()) for Arduino pins 9 and 10. Takes Timer 2 (8-bit)
 * for the pulse width modulation, breaking PWM for pins 11 & 3.
 *
 * References:
 *     http://www.uchobby.com/index.php/2007/11/11/arduino-sound-part-1/
 *     http://www.atmel.com/dyn/resources/prod_documents/doc2542.pdf
 *     http://www.evilmadscientist.com/article.php/avrdac
 *     http://gonium.net/md/2006/12/27/i-will-think-before-i-code/
 *     http://fly.cc.fer.hr/GDM/articles/sndmus/speaker2.html
 *     http://www.gamedev.net/reference/articles/article442.asp
 *
 * Michael Smith <michael@hurts.ca>
 */

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

#define SAMPLE_RATE 8000

/*
 * The audio data needs to be unsigned, 8-bit, 8000 Hz, and small enough
 * to fit in flash. 10000-13000 samples is about the limit.
 *
 * sounddata.h should look like this:
 *     const int sounddata_length=10000;
 *     const unsigned char sounddata_data[] PROGMEM = { ..... };
 *
 * You can use wav2c from GBA CSS (OR use EncodeAudio? http://highlowtech.org/?p=1963):
 *     http://thieumsweb.free.fr/english/gbacss.html
 * Then add "PROGMEM" in the right place. I hacked it up to dump the samples
 * as unsigned rather than signed, but it shouldn't matter.
 *
 * http://musicthing.blogspot.com/2005/05/tiny-music-makers-pt-4-mac-startup.html
 * mplayer -ao pcm macstartup.mp3
 * sox audiodump.wav -v 1.32 -c 1 -r 8000 -u -1 macstartup-8000.wav
 * sox macstartup-8000.wav macstartup-cut.wav trim 0 10000s
 * wav2c macstartup-cut.wav sounddata.h sounddata
 *
 * (starfox) nb. under sox 12.18 (distributed in CentOS 5), i needed to run
 * the following command to convert my wav file to the appropriate format:
 * sox audiodump.wav -c 1 -r 8000 -u -b macstartup-8000.wav
 */

//#include "sounddata.h"
#include "toolow.h"
#include "toohigh.h"

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
/*Sketch uses 3824 bytes (11%) of program storage space. Maximum is 32256 bytes.
Global variables use 20014 bytes (977%) of dynamic memory, leaving -17966 bytes for local variables. Maximum is 2048 bytes. */
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


int      ledPin   = 13;
int      speakerPin = 11; // Can be either 3 or 11, two PWM outputs connected to Timer 2

volatile uint16_t sample;
byte lastSample;

int sounddata_length;

int       upPin    =  7;  // was 12; // pushbutton attached to pin 12
int       downPin  =  8;  // was 13; // pushbutton attached to pin [12 sic]

/*int sounddata_length;
signed char *sounddata_data[10000]; */

void stopPlayback()
{
    // Disable playback per-sample interrupt.
    TIMSK1 &= ~_BV(OCIE1A);

    // Disable the per-sample timer completely.
    TCCR1B &= ~_BV(CS10);

    // Disable the PWM timer.
    TCCR2B &= ~_BV(CS10);

    digitalWrite(speakerPin, LOW);
}

unsigned int sounddata_addr;

// This is called at 8000 Hz to load the next sample.
ISR(TIMER1_COMPA_vect) {
    if (sample >= sounddata_length) {
        if (sample == sounddata_length + lastSample) {
            stopPlayback();
        }
        else {
            if(speakerPin==11){
                // Ramp down to zero to reduce the click at the end of playback.
                OCR2A = sounddata_length + lastSample - sample;
            } else {
                OCR2B = sounddata_length + lastSample - sample;                
            }
        }
    }
    else {
        if(speakerPin==11){
            OCR2A = pgm_read_byte(sounddata_addr + sample);
        } else {
            OCR2B = pgm_read_byte(sounddata_addr + sample);            
        }
    }

    ++sample;
}


void startPlayback(int clip)
{

signed char *sounddata_data[10000];
    
    switch (clip) {
      case   1:  sounddata_data[0] = &s_01_data[0]; sounddata_length = s_01_length; break;
      case   2:  sounddata_data[0] = &s_02_data[0]; sounddata_length = s_02_length; break;
      case   3:  sounddata_data[0] = &s_03_data[0]; sounddata_length = s_03_length; break;
      case   4:  sounddata_data[0] = &s_04_data[0]; sounddata_length = s_04_length; break;
      case   5:  sounddata_data[0] = &s_05_data[0]; sounddata_length = s_05_length; break;

    }

sounddata_addr = sounddata_data[0];
      
    pinMode(speakerPin, OUTPUT);

    // Set up Timer 2 to do pulse width modulation on the speaker
    // pin.

*  // Use internal clock (datasheet p.160)
    ASSR &= ~(_BV(EXCLK) | _BV(AS2));

    // Set fast PWM mode  (p.157)
    TCCR2A |= _BV(WGM21) | _BV(WGM20);
    TCCR2B &= ~_BV(WGM22);

    if(speakerPin==11){
        // Do non-inverting PWM on pin OC2A (p.155)
        // On the Arduino this is pin 11.
        TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
        TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));
        // No prescaler (p.158)
        TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

        // Set initial pulse width to the first sample.
        OCR2A = pgm_read_byte(&sounddata_data[0]);
    } else {
        // Do non-inverting PWM on pin OC2B (p.155)
        // On the Arduino this is pin 3.
        TCCR2A = (TCCR2A | _BV(COM2B1)) & ~_BV(COM2B0);
        TCCR2A &= ~(_BV(COM2A1) | _BV(COM2A0));
        // No prescaler (p.158)
        TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

        // Set initial pulse width to the first sample.
        OCR2B = pgm_read_byte(&sounddata_data[0]);
    }

    // Set up Timer 1 to send a sample every interrupt.
    cli();

    // Set CTC mode (Clear Timer on Compare Match) (p.133)
    // Have to set OCR1A *after*, otherwise it gets reset to 0!
    TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
    TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));

    // No prescaler (p.134)
    TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);

    // Set the compare register (OCR1A).
    // OCR1A is a 16-bit register, so we have to do this with
    // interrupts disabled to be safe.
    OCR1A = F_CPU / SAMPLE_RATE;    // 16e6 / 8000 = 2000

    // Enable interrupt when TCNT1 == OCR1A (p.136)
    TIMSK1 |= _BV(OCIE1A);

    lastSample = pgm_read_byte(&sounddata_data[sounddata_length-1]);
    sample = 0;
    sei();
}

void setup()
{
// audio pins
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
    
// input pins
    pinMode(upPin, INPUT);   // sets pin 12 as pushbutton INPUT
    pinMode(downPin, INPUT);   // sets pin 13 as pushbutton INPUT
    
    startPlayback(02);

    startPlayback(01);
    
/*    startPlayback(02);
    startPlayback(03);
    startPlayback(04);
    startPlayback(05);*/

}

void loop()
{

  loop();
//       stopPlayback();
/*currUpState = digitalRead(upPin);
if (prevUpState != currUpState)             // has the state changed from
{                                           // HIGH to LOW or vice versa
prevUpState = currUpState;
if (currUpState == HIGH)                    // If the button was pressed
counter++;                                  // increment the counter by one
//delay(1);
}

if(counter > 5)
counter -= 1;
show(counter); // display the current digit
currDownState = digitalRead(downPin);
if (prevDownState != currDownState)         // has the state changed from
{                                           // HIGH to LOW or vice versa
prevDownState = currDownState;
if (currDownState == HIGH)                  // If the button was pressed
counter -= 1;                               // decrement the counter by one
//delay(1);
}
if(counter < 0)
counter++;
show(counter);                              // display the current digit
}

void show( byte number)
{
digitalWrite(latchPin, LOW); // Set latchPin LOW while clocking these 8 bits in to the register
for (int k=0; k <= 7; k++)
{
digitalWrite(clockPin, LOW); // clockPin LOW prior to sending a bit
if ( data & (1 << k) )
{
digitalWrite(dataPin, HIGH); // turn “On”
}
else
{
digitalWrite(dataPin, LOW); // turn “Off”
}
digitalWrite(clockPin, HIGH); // and clock the bit in
}
digitalWrite(clockPin, LOW);  //stop shifting out data
digitalWrite(latchPin, HIGH); //set latchPin to high to lock and send data
}; */
}


