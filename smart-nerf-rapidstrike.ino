#include <Button.h>																											//library to deal with buttons easier
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//pins
#define IR_GATE_PIN 0																											//analog input
#define TOGGLE_FIRE_MODES_BTN_PIN 7 																			//digital inpit
#define TRIGGER_PIN 11              																			//digital input
#define DART_COUNTER_SWITCH_PIN 4   																			//digital input
#define MOTOR_OUTPUT_PIN 3          																			//digital output

//for buttons/switches
#define PULLUP true        																								//internal pullup, so we dont need to wire resistor
#define INVERT true      																									//invert required for proper readings with pullup
#define DEBOUNCE_MS 20 																										//check btn time every 20ms

#define IR_GATE_TRIP 90																										//'trip' value for IR gate					

//code for fire modes. 4 modes total
#define SAFETY 0																													//SAFTEY is mode 0
#define SINGLE_FIRE 1																											//singe fire is mode 1
#define BURST_FIRE 2																											//burst fire is mode 2
#define FULL_AUTO 3																												//full auto is mode 3

#define OLED_RESET 4

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


byte fireMode = 0;   																											//keep track of fire modes. 
byte dartsFired = 0;																											//keep track of how many darts fire
bool isCheckingForDartsFired = false;																			//some modes need to check if a certain number of darts to fire

byte magSizeArr[] = {5, 6, 10, 12, 15, 18, 20, 22, 25, 36, 0};            //keep track of the magazine sizes
byte currentMagSize = 0;                                                  //keep track of the current magazine size
byte currentAmmo = magSizeArr[currentMagSize];                            //keep track of how much ammo there currently is
byte maxAmmo = magSizeArr[currentMagSize];

Button trigger (TRIGGER_PIN, PULLUP, INVERT, DEBOUNCE_MS);														//trigger button, using the library   
Button dartCountingSwitch (DART_COUNTER_SWITCH_PIN, PULLUP, INVERT, DEBOUNCE_MS);			//dart counting button, using the library
Button toggleFireModesBtn (TOGGLE_FIRE_MODES_BTN_PIN, PULLUP, INVERT, DEBOUNCE_MS);		//toggle fire modes button, using the librarys

Adafruit_SSD1306 display(OLED_RESET);

void setup () {   
    pinMode(MOTOR_OUTPUT_PIN, OUTPUT);																		//set motor output pin to an output pin
    digitalWrite(MOTOR_OUTPUT_PIN, LOW);        													//make sure motor is off
    resetDartsFired();																										//reset all dart firing values so they dont get messed up later
}

void loop () {
    toggleFireModes();																										//constantly check for changes in firemodes
    fire();																																//constantly check if dart is fired
    checkForDartsFired();																									//do stuff if dart is fired
    selectFire();																													//do fancy select-fire stuff
}

//switch between the various modes
void toggleFireModes () {
	toggleFireModesBtn.read();																							//read button
	if (toggleFireModesBtn.wasPressed()) {																	//check if it was pressed
		fireMode = ((fireMode == 3) ? 0 : fireMode + 1);    									//increment fireMode
	  resetDartsFired();																										//reset darts fired stuff so it doesn't get messed up later
    updateDisplay();
	}
}

//when dart fired
void fire() {
  dartCountingSwitch.read();																							//read button
  dartsFired += ( (isCheckingForDartsFired && 														//detect and keep track if dart is fired through
  	( (map(analogRead(IR_GATE_PIN), 0, 1023, 0, 100) > IR_GATE_TRIP) ||		//switch or IR gate. 
  	 dartCountingSwitch.wasPressed()) )
  	 ? 1 : 0);        
}

void checkForDartsFired () {						
  if (isCheckingForDartsFired && 																					//if checking for darts being fired. Not all 
   (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE)) {									// modesneed to check if a dart is fired
    byte dartsToFire = (fireMode == SINGLE_FIRE ? 1 : 3);									//determine max amounts of darts to be fired
    if (dartsFired < dartsToFire) {																				//if can still fire (hasn't reached threshold of
      digitalWrite(MOTOR_OUTPUT_PIN, HIGH);																//how many darts can fire), power pusher motor
    } else if (dartCountingSwitch.isPressed() ) {													//if can't fire anymore darts and pusher retracted
      currentAmmo++;
      updateDisplay;
      if (dartsFired >= dartsToFire) {
        resetDartsFired();																								//Reset darts fired stuff so it can happen again
      }
    }
  }
}

//do all the fancy select fire stuff
void selectFire () {
    trigger.read();																												//read trigger
    if (trigger.isPressed()) {      																			//check of trigger is pressed
        if (fireMode == SAFETY) {       																	//if in safety mode, turn off motor
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);													
        } else if (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE) {		//if in burst fire or single shot mode
            isCheckingForDartsFired = true;																//allow for darts to be fired, handled elsewhere
        } else if (fireMode == FULL_AUTO) {     													//if full auto turn on motor
            digitalWrite(MOTOR_OUTPUT_PIN, HIGH);													
        }
    } else if (!trigger.isPressed()) {    																//if trigger isn't pressed
        if (fireMode == FULL_AUTO || fireMode == SAFETY) {								//if firemode is fullauto or safety, turn off motor
            digitalWrite(MOTOR_OUTPUT_PIN, LOW);													
        } else if ( !isCheckingForDartsFired 															//if all darts fired
         && (fireMode == SINGLE_FIRE || fireMode == BURST_FIRE) ) {     	//and in burstfire 
        	resetDartsFired();																							//reset darts fired stuff
        }		
    }
}

void resetDartsFired () {
	digitalWrite(MOTOR_OUTPUT_PIN, LOW);																		//turn of motor
	dartsFired = 0;																													//darts fired set to 0
	isCheckingForDartsFired = false;																				//no longer checking if darts are being fired
}

void updateDisplay () {
  display.clearDisplay();           //clear the display, so the stuff that was here before is no longer here

  display.setTextSize(7);  //set the size of the text
  display.setCursor(30, 0);  //center text
  display.print((currentAmmo < 10 ? "0" : "") + (String)currentAmmo);    //print the text
  
  display.setTextSize(1);
  display.setCursor(85, 55);
  if (fireMode == SAFETY) {
    display.print("SAFETY");
  } else if (fireMode == SINGLE_FIRE) {
    display.print("SINGLE");
  } else if (fireMode == BURST_FIRE) {
    display.print("BURST");
  } else if (fireMode == FULL_AUTO) {
    display.print("AUTO");
  }

  display.display();                //display the text
}