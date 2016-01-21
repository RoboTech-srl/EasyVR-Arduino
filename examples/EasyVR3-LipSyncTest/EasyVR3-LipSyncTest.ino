/**
  Example code for the EasyVR library v1.9
  Written in 2015 by RoboTech srl for VeeaR <http:://www.veear.eu>

  To the extent possible under law, the author(s) have dedicated all
  copyright and related and neighboring rights to this software to the
  public domain worldwide. This software is distributed without any warranty.

  You should have received a copy of the CC0 Public Domain Dedication
  along with this software.
  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include "Arduino.h"
#if !defined(SERIAL_PORT_MONITOR)
  #error "Arduino version not supported. Please update your IDE to the latest version."
#endif

#if defined(SERIAL_PORT_USBVIRTUAL)
  // Shield Jumper on HW (for Leonardo and Due)
  #define port SERIAL_PORT_HARDWARE
  #define pcSerial SERIAL_PORT_USBVIRTUAL
#else
  // Shield Jumper on SW (using pins 12/13 or 8/9 as RX/TX)
  #include "SoftwareSerial.h"
  SoftwareSerial port(12, 13);
  #define pcSerial SERIAL_PORT_MONITOR
#endif

#include "EasyVR.h"

EasyVR easyvr(port);

unsigned long t;

void setup()
{
  // setup PC serial port
  pcSerial.begin(9600);

  // bridge mode?
  int mode = easyvr.bridgeRequested(pcSerial);
  switch (mode)
  {
  case EasyVR::BRIDGE_NONE:
    // setup EasyVR serial port
    port.begin(9600);
    // run normally
    pcSerial.println(F("---"));
    pcSerial.println(F("Bridge not started!"));
    break;
    
  case EasyVR::BRIDGE_NORMAL:
    // setup EasyVR serial port (low speed)
    port.begin(9600);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("---"));
    pcSerial.println(F("Bridge connection aborted!"));
    break;
    
  case EasyVR::BRIDGE_BOOT:
    // setup EasyVR serial port (high speed)
    port.begin(115200);
    // soft-connect the two serial ports (PC and EasyVR)
    easyvr.bridgeLoop(pcSerial);
    // resume normally if aborted
    pcSerial.println(F("---"));
    pcSerial.println(F("Bridge connection aborted!"));
    break;
  }

  // initialize EasyVR  
  while (!easyvr.detect())
  {
    pcSerial.println(F("EasyVR not detected!"));
    delay(1000);
  }

  pcSerial.print(F("EasyVR detected, version "));
  pcSerial.println(easyvr.getID());
  
  if (easyvr.getID() <= EasyVR::EASYVR3_1)
  {
    pcSerial.println(F("Update firmware to use Lip-Sync!"));
    for(;;);
  }

  pcSerial.println(F("---"));

  if (!easyvr.realtimeLipsync(EasyVR::RTLS_THRESHOLD_DEF, 0))
  {
    pcSerial.println(F("Failed to start Lip-Sync!"));
    for(;;);
  }

  delay(100); // wait for lipsync to be ready
}

void loop()
{
  // fetch new lipsync value
  int8_t pos = -1;
  if (easyvr.fetchMouthPosition(pos))
  {
    t = millis();
    
    // map mouth position (0-31) to the pwm range (0-255)
    uint8_t pwm = (pos << 3) | (pos >> 2);
    analogWrite(A3, pwm);

    pcSerial.print(pos, DEC);
    pcSerial.print(" , ");
    pcSerial.println(pwm, DEC);
  }
  else
  {
    t = millis();
    pcSerial.println("no lipsync");
  }
  
  // wait for next lipsync value
  // lipsync refreshes every 27ms, but we wait 25ms
  // the next fetch will synchronize
  while (t+25 < millis())
  {
    // do something else
    delay(1);
  }
}
