

const  int8_t MAX_MESSAGE_LENGTH = 60;
static char message[MAX_MESSAGE_LENGTH];
static unsigned int message_index = 0;
const  int8_t START_PIN_NUMBER = 0;
const  int8_t MAX_PIN_NUMBER = 4;




int8_t Serial_Test_Pin;
const  int8_t  TESTED_PIN1 = 21;
const  int8_t TESTED_PIN2 = 4;





////////////////////////////// FUNCTIONS //////////////////////////////////////



void Static_Blink()
{
  digitalWrite(TESTED_PIN1, HIGH);  // turn the LED on (HIGH is the voltage level)
  digitalWrite(TESTED_PIN2, LOW);   // turn the LED off by making the voltage LOW
  delay(1500);                      // wait for a second
  digitalWrite(TESTED_PIN2, HIGH);  // turn the LED on (HIGH is the voltage level)
  digitalWrite(TESTED_PIN1, LOW);   // turn the LED off by making the voltage LOW
  delay(1500);
}




void blink_Pin(int8_t x )
{
  pinMode(x, OUTPUT);
  for (int8_t i = 0; i < 10; i++)
  {
    digitalWrite(x, LOW);   // turn the LED off by making the voltage LOW
    delay(500);                      // wait for a second
    digitalWrite(x, HIGH);  // turn the LED on (HIGH is the voltage level)
    delay(500);
  }
}





void Chose_Blink_Serial_Pin( char x[])
{
  if ( (x == "ALL") ||   (x == "all")  )
  {
    for ( int8_t j = START_PIN_NUMBER; j <= MAX_PIN_NUMBER; j++)
    {
      Serial.print("Testing Pin: ");
      Serial.println(j);
      blink_Pin( j );
    }
  }
  else
  {
    Serial.print("Testing Pin: ");
    Serial_Test_Pin = atoi(x);
    if ( ( Serial_Test_Pin > MAX_PIN_NUMBER ) || (Serial_Test_Pin < START_PIN_NUMBER) )
    {
      Serial.println( "INVALID PIN NUMBER " );
    }
    else
    {
      Serial.println(Serial_Test_Pin);
      blink_Pin( Serial_Test_Pin );
    }


  }

}


////////////////////////////// MAIN LOOPS //////////////////////////////////////





// the setup function runs once when you press reset or power the board
void setup()
{
  // initialize digital pin TESTED_PIN as an output.
  Serial.begin(9600);

}


////////////////////////////// VOID LOOP //////////////////////////////////////


// the loop function runs over and over again forever
void loop()
{
  char inByte;
  Serial.println("Waiting for pin number input  ");


//Check to see if anything is available in the serial receive buffer
  while (Serial.available() > 0)
  {

    //Read the next available byte in the serial receive buffer
    inByte = Serial.read();

    //Message coming in (check not terminating character) and guard for over message size
    if ( inByte != '!' && (message_index < MAX_MESSAGE_LENGTH - 1) )
    {
      //Add the incoming byte to our message
      message[message_index] = inByte;
      message_index++;
    }
    //Full message received...
    else
    {
      //Add null character to string
      message[message_index] = '\0';

      //Print the message
      Serial.print("I received: ");
      Serial.println(message);
      //Reset for the next message
      message_index = 0;
      Chose_Blink_Serial_Pin( message );



    }
  }
}









