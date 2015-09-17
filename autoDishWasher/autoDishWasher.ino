//define the data and clock pins
const int DataPin=7;
const int  ClkPin=8;
const int  LedPin1=10;
const int  LedPin2=5;
const int Dsp1Pin=3;
const int Dsp2Pin=4;
const int  KeyPin=6;

const int relayPin = 12;
const int soapPumpPin = 11;
const int dispenserInputPin = 10;
const int washCompletePin = 2;			// wash(clean LED )

//set all of the charater patterns in binary
byte zero  = B00000011;
byte one   = B10011111;
byte two   = B00100101;
byte three = B00001101;
byte four  = B10011001;
byte five  = B01001001;
byte six   = B01000001;
byte seven = B00011111;
byte eight = B00000001;
byte nine  = B00001001;
byte number[] = {zero, one, two, three, four, five, six, seven, eight, nine};

byte led1 = B11111111;				// active low (?)
byte led2 = B11111111;
byte digit1 = zero;
byte digit2 = zero;


const int startButton = A3;
const int myFavButton = A4;
const int normalWashButton = A5;

void setup()
{
  pinMode(ClkPin, OUTPUT);					 // make the clock pin an output
  pinMode(DataPin , OUTPUT);				 // make the data pin an output3
  pinMode(LedPin1 , OUTPUT);				// row of 8 LEDs
  pinMode(LedPin2 , OUTPUT);				// row of 8 LEDS
  pinMode(Dsp1Pin , OUTPUT);				// 8-segment display #1
  pinMode(Dsp2Pin , OUTPUT);				// 8-segment display #2
  pinMode(KeyPin, INPUT);					// row of 8 buttons

  pinMode(relayPin, OUTPUT);
  pinMode(washCompletePin, INPUT);
  pinMode(soapPumpPin, OUTPUT);
  pinMode(dispenserInputPin, INPUT);

  pinMode(startButton, A3);
  pinMode(myFavButton, A4);
  pinMode(normalWashButton, A5);

	digitalWrite(startButton, LOW);
	digitalWrite(myFavButton, LOW);
	digitalWrite(normalWashButton, LOW);
	digitalWrite(relayPin, HIGH);					// for relay HIGH = closed circuit ( equivalent to door closed)

  Serial.begin(9600);
  Serial.println("Automatic DishWasher");

}

void Display(int num)
{		
  digit1 = number[(int)(num/10)];
  digit2 = number[num-(int(num/10)*10)];
  
}

byte ScanKey()
{
  byte KeyState = B00000000;														 // variable for reading the pushbutton status
  shiftOut(DataPin, ClkPin, LSBFIRST, B11111110);
  if (digitalRead(KeyPin))
	  bitClear(KeyState, 0);
  else
	  bitSet(KeyState, 0);
	  
  shiftOut(DataPin, ClkPin, LSBFIRST, B11111101);
  if (digitalRead(KeyPin)){bitClear(KeyState, 1);}else{bitSet(KeyState, 1);}
  shiftOut(DataPin, ClkPin, LSBFIRST, B11111011);
  if (digitalRead(KeyPin)){bitClear(KeyState, 2);}else{bitSet(KeyState, 2);}
  shiftOut(DataPin, ClkPin, LSBFIRST, B11110111);
  if (digitalRead(KeyPin)){bitClear(KeyState, 3);}else{bitSet(KeyState, 3);}
  shiftOut(DataPin, ClkPin, LSBFIRST, B11101111);
  if (digitalRead(KeyPin)){bitClear(KeyState, 4);}else{bitSet(KeyState, 4);}
  shiftOut(DataPin, ClkPin, LSBFIRST, B11011111);
  if (digitalRead(KeyPin)){bitClear(KeyState, 5);}else{bitSet(KeyState, 5);}
  shiftOut(DataPin, ClkPin, LSBFIRST, B10111111);
  if (digitalRead(KeyPin)){bitClear(KeyState, 6);}else{bitSet(KeyState, 6);}
  shiftOut(DataPin, ClkPin, LSBFIRST, B01111111);
  if (digitalRead(KeyPin)){bitClear(KeyState, 7);}else{bitSet(KeyState, 7);}

  return KeyState;
}


boolean StartCounting = false;				
int LoopCount1 = 0;
int screenLoopCount = 0;
int counter3 = 0;
int ledFlashCount = 0;


int numberOfWashes = 1;
int currentWash = 1;
int screenMode = 1;					// 0 = setup # of washes. 1 = counting Down (machine running)

byte KeyTempLast;
boolean inputLast;
boolean cyclesComplete = false;
boolean LCDon = false;

void readAndRespondButton()
{
	LoopCount1 = 0;	
	byte KeyTemp = ScanKey();
	//Serial.println(KeyTemp);		
	if (KeyTemp == 4)	screenMode = 0;		// Mode button(SW6). Can be configured only WHILE button is pressed
		
	if (KeyTemp != KeyTempLast)				// only runs on state changes
	{
		switch (KeyTemp)
		{						
			case 68:							//SW6 + SW2 = Up
				numberOfWashes++;
				break;
				
			case 132:							// SW6 + SW1 = down
				if (numberOfWashes > 1 )	numberOfWashes--;				
				break;
				
			case 8:						// ON/OFF (SW4)
				if (screenMode == 1)
				{
					StartCounting = !StartCounting;	
					if (LCDon)
					{
						
						digitalWrite(normalWashButton, HIGH);
						delay(700);
						digitalWrite(normalWashButton, LOW);
						delay(700);
					}
					else
					{
						Serial.println("Turning LCD ON");
						delay(500);
						digitalWrite(startButton, LOW);
						delay(500);
						digitalWrite(startButton, HIGH);
						delay(500);
						digitalWrite(startButton, LOW);
						LCDon = true;
						digitalWrite(relayPin, LOW);
						digitalWrite(relayPin, HIGH);
					}
					
				}
				break;
		
			case 4:
				screenMode = 0; break;
				
				// OTHER buttons 
			case 16:
				digitalWrite(startButton, HIGH);
				break;	
				
			case 32:/*
				digitalWrite(normalWashButton, HIGH);
				delay(700);
				digitalWrite(normalWashButton, LOW);
				delay(700);
				*/
				digitalWrite(soapPumpPin, HIGH);
				delay(200);
				digitalWrite(soapPumpPin, LOW);
				delay(200);
				break;	
			
			
			default:							// no buttons are pressed (Falling edge of all buttons)
				screenMode = 1;
				digitalWrite(startButton, LOW);
				break;

		}
	}	
	KeyTempLast = KeyTemp;
	
	
	if (screenLoopCount >= 2)			// run loop once every 2 loop iterations
	{		
		screenLoopCount= 0;
		switch (screenMode)
		{
			case 0:								// setup screen
				led2 = B11110111;	
				Display(numberOfWashes);
				if (!StartCounting)		currentWash = numberOfWashes;					
				cyclesComplete = false;
				break;
				
			case 1:								// counting down screen
				if (cyclesComplete)
					led2 = B10000000;			// lights on when cycle is done
				else
					led2 = B11111101;			// only single light when in running mode
				
				if (StartCounting)
				{	
					// FLASH led to let user know it's on				
					if (ledFlashCount < 2)
						led2 = B10111101;
					else if (ledFlashCount < 3)
						led2 = B11011101;				//off
					else if (ledFlashCount < 4)	
						led2 = B11101101;	
					else
						ledFlashCount = 0;
					ledFlashCount++;
					
					//										 <-----------					read for input from washer that cycle is over	
					int tempInput = digitalRead(washCompletePin);
					Serial.println(tempInput);
					if ( (tempInput == LOW) && (inputLast== HIGH) )				// on FALLING EDGE
					{
						LCDon = true;
						currentWash--;
						digitalWrite(relayPin, LOW);
						delay(1000);
						digitalWrite(normalWashButton, HIGH);
						delay(700);
						digitalWrite(normalWashButton, LOW);
						delay(700);
						digitalWrite(relayPin, HIGH);
					}
					else
					{
						//led2 = B11111101;			// only running on
						LCDon = false;
					}
					inputLast = tempInput;			
					
					
					if (currentWash == 0)								// countdown complete
					{
						StartCounting = false;
						cyclesComplete = true;
					}					
				}												
				Display(currentWash);					
				break;				
		}
	}
	screenLoopCount++;
	
}





void loop()
{ 
	
	if (LoopCount1 == 3)
		readAndRespondButton();
	
	int tempSoapSignalVal = digitalRead(dispenserInputPin);
	if (tempSoapSignalVal == HIGH)
		digitalWrite(soapPumpPin, HIGH);
	
	
	//  MULTIPLEXING: writes current state for LED2, DIGIT1 and digit2 COlumns
	// 3 ROWS: LED2 ROW,						SEGment1 ROw,					SEGMENT2 ROw
    shiftOut(DataPin, ClkPin, LSBFIRST, led2);	
    digitalWrite(LedPin2, LOW);   digitalWrite(Dsp1Pin, HIGH);    digitalWrite(Dsp2Pin, HIGH);
    delay(4);
    digitalWrite(LedPin2, HIGH);   digitalWrite(Dsp1Pin, HIGH);    digitalWrite(Dsp2Pin, HIGH);

    shiftOut(DataPin, ClkPin, LSBFIRST, digit1);
    digitalWrite(LedPin2, HIGH);   digitalWrite(Dsp1Pin, LOW);     digitalWrite(Dsp2Pin, HIGH);
    delay(4);
    digitalWrite(LedPin2, HIGH);   digitalWrite(Dsp1Pin, HIGH);    digitalWrite(Dsp2Pin, HIGH);
    
    shiftOut(DataPin, ClkPin, LSBFIRST, digit2);
    digitalWrite(LedPin2, HIGH);   digitalWrite(Dsp1Pin, HIGH);    digitalWrite(Dsp2Pin, LOW);
    delay(4);
    digitalWrite(LedPin2, HIGH);   digitalWrite(Dsp1Pin, HIGH);    digitalWrite(Dsp2Pin, HIGH);

    LoopCount1 += 1;
}
