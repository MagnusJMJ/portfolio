#include <SPI.h>
#include "PN532_SPI.h"
#include "PN532.h"
#include "NfcAdapter.h"
#include <Adafruit_NeoPixel.h>


//LED
#define PIN 6
#define PIN2 5

Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(200, PIN2, NEO_GRB + NEO_KHZ800);

float min = 0, max = 254, current = 0, step = 0.6;

int ledvar = 1;



// FLAG_NONE used to signal nothing needs to be done
#define FLAG_NONE 0
// FLAG_IRQ_TRIGGERED used to signal an interrupt trigger
#define FLAG_IRQ_TRIGGERED 1
// FLAG_RESET_IRQ used to signal that the interrupt needs to be reset
#define FLAG_RESET_IRQ 2
// flags variable used to store the present flag
volatile int flags = FLAG_NONE;

String const Tag1 = "52 EA CD 26", Tag2 = "27 A8 41 AF", Tag3 = "37 0D 23 76";


// the interrupt we'll be using (interrupt 0) is located at digital pin 2
int const irqPin = 2; // interrupt pin

PN532_SPI interface(SPI, 10); // create a SPI interface for the shield with the SPI CS terminal at digital pin 10

NfcAdapter nfc = NfcAdapter(interface); // create an NFC adapter object

String scannedUID = ""; // this is where we'll store the scanned tag's UID

void setup(void) {
  // make LED pins outputs

  strip.begin();
  strip1.begin();
  strip.show(); // initialize all pixels to "off"
  strip1.show();

  Serial.begin(115200); // start serial comm
  Serial.println("NDEF Reader");
  nfc.begin(); // begin NFC comm


  // attach the function "irq" to interrupt 0 on the falling edges
  attachInterrupt(0, irq, FALLING); // digital pin 2 is interrupt 0, we'll call the irq function (below) on the falling edge of this pin
}

void loop(void) {
  int flag = getFlag(); // get the present flag

  current += step;

  if (current > max || current < min) {
    step *= -1;
  }

  if ( ledvar == 1)
  {
    Serial.println(current);
    uint16_t i, j;
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0, 0, current);
    }
   strip.show();
   strip1.show();
  }

  if ( ledvar == 2)
  {
    Serial.println(current);
    uint16_t i, j;
     for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, 0, 0, 0);
    }
    for (i = 0; i < strip1.numPixels(); i++) {
      strip1.setPixelColor(i, 0, 0, current);
    }
    strip.show();
    strip1.show();
  }



  switch (flag) // check which flag/signal we are on
  {
    case FLAG_NONE:
      // nothing needs to be done
      break;
    case FLAG_IRQ_TRIGGERED: // the interrupt pin has been triggered
      Serial.println("Interrupt Triggered");
      if (nfc.tagPresent())
      {
        // an NFC tag is present
        NfcTag tag = nfc.read(); // read the NFC tag
        scannedUID = tag.getUidString(); // get the NFC tag's UID
        if (Tag1.compareTo(scannedUID) == 0) // compare the NFC tag's UID with the correct tag's UID (a match exists when compareTo returns 0)
        {
          // the scanned NFC tag matches the saved myUID value
          Serial.println("1");
          // put your here to trigger the unlocking mechanism (e.g. motor, transducer)
        } else if (Tag2.compareTo(scannedUID) == 0) {
          // the scanned NFC tag's UDI does not match the myUID value
          Serial.println("2");
          ledvar = 2;
          // DO NOT UNLOCK! an incorrect NFC tag was used.
          // put your code here to trigger an alarm (e.g. buzzard, speaker) or do something else
        } else if (Tag3.compareTo(scannedUID) == 0) {
          // the scanned NFC tag's UDI does not match the myUID value
          Serial.println("3");
          // DO NOT UNLOCK! an incorrect NFC tag was used.
          // put your code here to trigger an alarm (e.g. buzzard, speaker) or do something else
        }
        // return to the original state
        setFlag(FLAG_NONE);
        reset_PN532_IRQ_pin();
      } else {
        // a tag was not present (the IRQ was triggered by some other action)
        setFlag(FLAG_NONE);
      }
      break;
    default:
      // do any other stuff for flags not handled above
      break;
  }
}

/*
  Name: setFlat
  Description: used to set actions/flags to be executed in the loop(void) function
  Parameters:
         int flag - the action/flag to store
  Returns: void
*/
void setFlag(int flag)
{
  flags = flag;
}

/*
  Name: getFlag
  Description: used to get the present flag/action
  Parameters: void
  Returns: int - the flags variable. The action/flag set by setFlag
*/
int getFlag()
{
  return flags;
}

/*
  Name: irq
  Description: Interrupt service routine (ISR). This function will be executed whenever there is a falling edge on digital pin 2 (the interrupt 0 pin)
  Parameters: void
  Returns: void
*/
void irq()
{
  if (getFlag() == FLAG_NONE) {
    setFlag(FLAG_IRQ_TRIGGERED);
  }
}
/*
  Name: reset_PN532_IRQ_pin
  Description: used to reset the PN532 interrupt request (IRQ) pin
  Parameters: void
  Returns: void
*/
void reset_PN532_IRQ_pin()
{
  nfc.tagPresent();
}

