/*
  Arduino Speed Test Game

  This is a reflex game loosely based on the Finnish Nopeustesti.
  There are four buttons and four LEDs.
  You have to press the right button after the LED lights up.
  If you press the wrong button the game ends and you have to restart it with Arduino's reset.
  The whole sequence gets generated to memory so you can play it as a memory game as well,
  trying to remember the correct sequence ;-).
  You can determine the length of the sequence and it will just start from the beginning once
  all the elements are looped through.

  This scetch doesn't have a scorekeeping display implemented
  
  The circuit:
  INPUTS:
  * Pins 4-7: push-button, in input pullup-mode (you have to connect the button between input pin and groung)
  OUTPUTS:
  * Pins 8-11: LED in series with a 220 ohm resistor

  Created 18.2.2022
  By Valtti Rinnemaa

  https://github.com/valttir/arduino-speed-test

*/

#define debounceDelay 50  // The debounce setting you should have depends on the type of buttons you are using
#define sequenceLength 1000  // Length of the random sequence defined here
#define startDelay 2000  // This is the delay we want to give the player before the game begins
#define maxSpeed 320  // Maximum speed the game will reach

unsigned short score;

// These are the intervals to show and hide the led when the game begins:
unsigned long intervalShow = 500;
unsigned long intervalHide = 300;

// For keeping time:
unsigned long previousMillis = 0;

// Modifier for increasing difficulty:
unsigned int increaseDifficulty = 0;

byte sequence[sequenceLength];

void setup()
{ 
  // Setting pins 4-7 to input pullup
  for (byte i = 4; i <= 7; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  // Setting pins 8-11 to output
  for (byte i = 8; i <= 11; i++) {
    pinMode(i, OUTPUT);
  }
  startNewGame();
}

void loop()
{ 
  showLed();
  hideLed();
  pollButtons();
} 

void startNewGame()
{ 
  // Set the score to 0 and generate a sequence of random numbers ranging 1-4
  score = 0;
  randomSeed(analogRead(0));
  for (int i = 0; i < 1000; i++) {
    sequence[i] = random(1, 5);  
  }
  // Some time for the player before the game begins
  delay(startDelay);
}

void showLed()
{ 
  // This will keep up with the element of the sequence we are looping through
  static int sequenceElement = 0;

  // Once the time interval minus the modifier is reached, the LED will light up:
  if (millis() - previousMillis >= intervalShow - increaseDifficulty) {
    switch (sequence[sequenceElement]) {
    case 1:
      digitalWrite(8, HIGH);
      break;
    case 2:
      digitalWrite(9, HIGH);
      break;
    case 3:
      digitalWrite(10, HIGH);
      break;
    case 4:
      digitalWrite(11, HIGH);
      break;
    }
    // It then moves to next element in the sequence:
    sequenceElement++;

    // Once the end of the sequence is reached, it will loop back to [0]
    if (sequenceElement == sequenceLength) {
      sequenceElement = 0;
    }
    // This takes a timestamp for timing the next "round"
    previousMillis = millis();

    // The game will get faster over time, limited by maxSpeed
    increaseDifficulty += 3;
    increaseDifficulty = min(increaseDifficulty, maxSpeed);
  }
}

void hideLed()
{ 
  // Once the time interval minus the modifier is reached, the LEDs will dim
  if (millis() - previousMillis >= intervalHide - increaseDifficulty) {
    for (byte i = 8; i <= 11; i++) {
    digitalWrite(i, LOW);
    } 
  }
}

void gameover()
{ 
  // Endless loop of flashing LEDs
  // You have to give Arduino the reset to begin a new game
  for ( ; ; ) {
    for (byte i = 8; i <= 11; i++) {
      digitalWrite(i, HIGH);
    }
    delay(500);
    for (byte i = 8; i <= 11; i++) {
      digitalWrite(i, LOW);
    }
    delay(500);
  }
}

void pollButtons()
{ 
  // https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
  // All the button reading and debounce information is in arrays of 4
  // so they can be looped through, polling all the buttons one by one
  static byte reading[4];
  static byte buttonState[4]; // The current reading from the button pin
  static byte lastButtonState[4] = {HIGH, HIGH, HIGH, HIGH}; // The previous reading from the button pin
  static unsigned long lastDebounceTime[4]; // the last time the button was noted as pressed 

  // This will keep up with the element of the sequence we are looping through
  static int sequenceElement = 0;

  // This loops through all the buttons, polling them one by one
  for (byte i = 4; i <= 7; i++) {
    // read the state of the switch into a local variable:
    reading[i] = digitalRead(i);

    // Check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:

    // If the switch changed, due to noise or pressing:
    if (reading[i] != lastButtonState[i]) {
      // Reset the debouncing timer
      lastDebounceTime[i] = millis();
    }
      
    if (millis() - lastDebounceTime[i] > debounceDelay) {
      // Whatever the reading is at, it's been there for longer than the debounce
      // delay, so take it as the actual current state:

      // If the button state has changed:
      if (reading[i] != buttonState[i]) {
        buttonState[i] = reading[i];

        // Only accept as input if the new button state is LOW
        if (buttonState[i] == LOW) {

          // If the correct button is pressed, increment score and move to the next element in the random sequence:
          if (sequence[sequenceElement] + 3 == i) {
            score++;
            sequenceElement++;
            
            // Once the end of the sequence is reached, it will loop back to [0]
            if (sequenceElement == sequenceLength) {
              sequenceElement = 0;
            }
          }
          else
            // If the wrong button gets pressed, it's game over
            gameover();
        }
      }
    }
    // save the reading. Next time through the loop, it'll be the lastButtonState:
    lastButtonState[i] = reading[i];
  }
}
