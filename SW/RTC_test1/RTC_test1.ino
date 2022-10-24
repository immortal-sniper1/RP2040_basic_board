/*
 * Demonstrate the RTCx library.
 *
 * This sketch demonstrates the basic functionality of the RTCx
 * library. It can also be used to adjust and check the frequency
 * calibration settings on MCP7941x devices.
 *
 * On start-up the autoprobe feature is used to search the I2C bus to
 * find the first available real-time clock. The sketch indicates
 * which clock type (if any) has been found. If the clock is an
 * MCP7941x device the current calibration setting is printed.
 *
 * The clock can be set by writing the current unixtime (i.e., seconds
 * since 1970-01-01 00:00:00Z) prefixed by the character 'T' to
 * Serial. The string must be terminated by a NULL character, line
 * feed, or carriage return. E.g., "T1343779200" sets the clock to
 * 2012-08-01 00:00:00Z".
 *
 * The clock error, measured in seconds, can be found in a similar way
 * by sending the current unix time, prefixed by the character 'C',
 * e.g., "C1343779200".
 *
 * It is possible to set the SQW output frequency by sending "M0" (1Hz
 * output), "M1" (4096Hz output), "M2" (8192Hz output) or "M3"
 * (32768Hz output). If the clock is a MCP7941x device then sending
 * "M4" will enable the calibration mode and output a 64Hz signal
 * which is continously compensated with the frequency calibration
 * setting, thereby enabling the correct calibration value to be
 * found.
 */

#include <Wire.h>
#include <RTCx.h>



const uint8_t bufLen = 30;
char buffer[bufLen + 1] = {'\0'};
uint8_t bufPos = 0;
unsigned long last = 0;



void setup(void)
{
  if (F_CPU > 8000000UL)
    Serial.begin(115200);
  else
    Serial.begin(9600);

  delay(20000);
  Wire.setSDA(12);
  Wire.setSCL(13);
  Wire.begin();
  Serial.println();
  
  Serial.println("----------------------------------------------------------");
  Scani2c( 0);
  Serial.println("----------------------------------------------------------");
  Serial.println();
  rtc.setClock();
  delay(20000);
  //Serial.print(rtc.readClock()  );
  Serial.print(rtc.getDeviceName()  );
    

  //-------------------------------------------------
  Serial.println("Autoprobing for a RTC...");
  if (rtc.autoprobe())
  {
    // Found something, hopefully a clock.
    Serial.print("Autoprobe found ");
    Serial.print(rtc.getDeviceName());
    Serial.print(" at 0x");
    Serial.println(rtc.getAddress(), HEX);
  }
  else
  {
    // Nothing found at any of the addresses listed.
    Serial.println("No RTCx found, cannot continue");
    while (1)
      ;
  }

  // Enable the battery backup. This happens by default on the DS1307
  // but needs to be enabled on the MCP7941x.
  rtc.enableBatteryBackup();

  // rtc.clearPowerFailFlag();

  // Ensure the oscillator is running.
  rtc.startClock();

  if (rtc.getDevice() == RTCx::MCP7941x)
  {
    Serial.print("Calibration: ");
    Serial.println(rtc.getCalibration(), DEC);
    // rtc.setCalibration(-127);
  }

  rtc.setSQW(RTCx::freq4096Hz);
  delay(5000);
}





void loop(void)
{
  struct RTCx::tm tm;
  if (millis() - last > 2000)
  {
    last = millis();
    rtc.readClock(tm);

    RTCx::printIsotime(Serial, tm).println();
    RTCx::time_t t = RTCx::mktime(&tm);

    Serial.print("unixtime = ");
    Serial.println(t);
    Serial.println("-----");
  }

  while (Serial.available())
  {
    char c = Serial.read();
    if ((c == '\r' || c == '\n' || c == '\0'))
    {
      if (bufPos <= bufLen && buffer[0] == 'C')
      {
        // Check time error
        buffer[bufPos] = '\0';
        RTCx::time_t pcTime = atol(&(buffer[1]));
        rtc.readClock(&tm);
        RTCx::time_t mcuTime = RTCx::mktime(&tm);
        Serial.print("MCU clock error: ");
        Serial.print(mcuTime - pcTime);
        Serial.println(" s");
        Serial.println("~~~~~");
      }
      if (bufPos <= bufLen && buffer[0] == 'T')
      {
        // Set time
        buffer[bufPos] = '\0';
        RTCx::time_t t = atol(&(buffer[1]));
        RTCx::gmtime_r(&t, &tm);
        rtc.setClock(&tm);
        Serial.println("Clock set");
        Serial.println(&(buffer[0]));
        RTCx::printIsotime(Serial, tm);
        Serial.println("~~~~~");
      }
      if (bufPos <= bufLen && buffer[0] == 'X')
      {
        // Set calibration value
        buffer[bufPos] = '\0';
        if (rtc.getDevice() == RTCx::MCP7941x)
        {
          int8_t oldCal = rtc.getCalibration();
          char *endptr;
          long cal = strtol(&(buffer[1]), &endptr, 0);
          if (cal >= -127 && cal <= 127 && endptr == &buffer[bufPos])
          {
            Serial.print("Previous calibration: ");
            Serial.println(oldCal, DEC);
            Serial.print("Calibration: ");
            Serial.println(cal, DEC);
            rtc.setCalibration(cal);
          }
          else
            Serial.println("Bad value for calibration");
        }
        else
        {
          Serial.println("Cannot set calibration: not a MCP7941x");
        }
      }
      if (bufPos <= bufLen && buffer[0] == 'M')
      {
        // Set SQW mode
        buffer[bufPos] = '\0';
        char *endptr;
        long mode = strtol(&(buffer[1]), &endptr, 0);
        if (mode >= RTCx::freq1Hz && mode <= RTCx::freqCalibration
            && endptr == &buffer[bufPos])
        {
          if (rtc.setSQW((RTCx::freq_t)mode))
          {
            Serial.print("SQW: ");
            Serial.println(mode, DEC);
          }
          else
            Serial.println("Could not set SQW");
        }
        else
          Serial.println("Bad value for SQW");
      }
      bufPos = 0;
    }
    else if (bufPos < bufLen)
      // Store character
      buffer[bufPos++] = c;
  }
}






void Scani2c(int wireSelection)
{

  //variable for error code
  byte errorCode;
  //variable for device address
  byte deviceAddress;
  //variable to hold total number of devices
  int totalDevices = 0;
  //Print to serial monitor
  Serial.printf("Now Scanning I2c port %d\n", wireSelection);
  //According to the I2c specification the first 8 and last 8 addresses are reserved
  //This loop loops through the addresses to find devices
  for (deviceAddress = 8; deviceAddress < 120; deviceAddress++ )
  {
    if (wireSelection == 0)
    {
      //Start wire 1 transmission on the current address
      Wire.beginTransmission(deviceAddress);
      //listen for a message from a device on that address
      errorCode = Wire.endTransmission();
    }
    else
    {
      //Start wire transmission on the current address
      Wire1.beginTransmission(deviceAddress);
      //listen for a message from a device on that address
      errorCode = Wire1.endTransmission();
    }


    //error code 0 will return if a device responds
    if (errorCode == 0)
    {
      //output the address to the serial monitor
      Serial.print("I2C device found at address 0x");
      //Add leading zero for addresses before 16
      if (deviceAddress < 16)
        Serial.print("0");
      Serial.print(deviceAddress, HEX);
      //Increment the device count
      totalDevices++;
    }
    //An error of 4 useually means a bad connection but can be some other failure.
    else if (errorCode == 4)
    {
      //print an error message
      Serial.print("Error at address 0x");
      if (deviceAddress < 16)
        Serial.print("0");
      Serial.println(deviceAddress, HEX);
    }
  }


  //if no dvices found let the user knoe.
  if (totalDevices == 0)
    Serial.println("No I2C devices found");
  else
    //otherwise tell the user how many devices were detected
    Serial.printf("\n%d", totalDevices);
  if (totalDevices > 0)
  {
    if (totalDevices < 2)
    {
      Serial.println(" I2c device found");
    }
    else
      Serial.println(" I2c devices found");
  }
  Serial.println("done\n");
}
