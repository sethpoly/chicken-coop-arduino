#include <dht.h>
#include <TimeLib.h>
#include <LiquidCrystal.h>

// Init the LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// For TimeSerial
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_REQUEST  7    // ASCII bell character requests a time sync message 

// For button sensors when reading door open/close
int openDoorPin = 13;  
int closeDoorPin = 10; 
int toggleDoorPin = 30; 
bool doorOpen = true;

// Open door values
int openDoorStateVal;
int openDoorStateVal_2;
int openDoorState;

// Close door values
int closeDoorStateVal;
int closeDoorStateVal_2;
int closeDoorState;

// Toggle button door values
int toggleDoorStateVal;
int toggleDoorStateVal_2;
int toggleDoorState;
bool toggleSwitch = false;

// Debounce values
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay(100);


// Motor inputs
int enA = 9;
int in1 = 8;
int in2 = 7;

unsigned long time_now = 0;
int period = 10000;

// Reads to turn on heat lamp
int relayPin = 22; 

// Temperature & fan variables
dht DHT;
int maxTemp = 70;
float temp;
float humidity;
int fanPin = 24; 
#define DHT11_PIN 26

// Month nested array with sunrise/sunset times
// No leading zeroes allowed
// Military time <12AM = 0>
int sunriseData[12][2] = {
  {810, 1845},  //Jan
  {734, 1935},  //Feb
  {644, 2015},  //Mar
  {558, 2040},  //Apr
  {530, 2110},  //May 
  {530, 2115},  //Jun
  {530, 2100},  //Jul
  {556, 2045},  //Aug
  {627, 2020},  //Sep
  {656, 1910},  //Oct
  {730, 1755},  //Nov
  {85, 1815}    //Dec
};

int month_;
int hour_;
int minute_;
int sunrise;
int sunset;

void setup() {
  Serial.begin(9600);
  
  // Set all motor control pins to outputs
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  // Set Relay output
  pinMode(relayPin, OUTPUT);
  pinMode(fanPin, OUTPUT);

  // Set motor speed
  setMotorSpeed(255);

  // Init the time library to await EPOCH INPUT
  setUpTimeLibrary();

  // Init the lcd
  lcd.begin(16, 2);

  // Init the close/open door button sensors
  pinMode(openDoorPin, INPUT);
  pinMode(closeDoorPin, INPUT);
}

// Write to the LCD
void writeToLCD(int column, int row, String sensor, float value){
  lcd.setCursor(column, row);
  lcd.print(sensor);
  lcd.print(value);
}


// Set up time library
void setUpTimeLibrary (){
  setSyncProvider( requestSync);  //set function to call when sync required
  Serial.println("Input the current time as T??????????");
}

// Realtime clockcounter
void timeClock(){
  if (Serial.available()) {
    processSyncMessage();
  }
  if (timeStatus()!= timeNotSet) {
    getTime();  
  }
}

void getTime() {
  // digital clock display of the time
  Serial.print(hour());
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.print(second());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println();   

  month_ = month() - 1;
  hour_ = hour();
  minute_ = minute();

  // Save sunset/sunrise times for the month
  sunrise = checkSunriseTime();
  sunset = checkSunsetTime();

  handleDoor(); // Check to open/close door
}

// When the time reaches sunrise or sunset - open/close the door
void handleDoor() {
  // Concat the current hour and minute
  char hour_string[50];
  sprintf(hour_string, "%d", hour_);
  char minute_string[50];
  sprintf(minute_string, "%d", minute_);
  strcat(hour_string, minute_string);

  // Convert hour_string back to int
  int current_time = atoi(hour_string);

  // Open and close door at sunset/sunrise
  // #TODO take out the toggle switch condition
  if(current_time == sunrise && !doorOpen && !toggleSwitch) {
    openDoor();
  }
  else if(current_time == sunset && doorOpen && !toggleSwitch) {
    closeDoor();
  }
}

// Handle the door toggle switch
void handleDoorToggle() {
//  if(toggleDoorState != toggleSwitch) { // If the button is pressed and the door is not currently opening
//    if(toggleDoorState == 1) {  // Button is pressed
//      Serial.println("Outside button pressed");
//      toggleSwitch = true;
//    } 
  //}

  // Open door by outside toggle switch <1 complete press>
//  if(toggleSwitch && toggleDoorState == 0) {
////    if(doorOpen) {
////      closeDoor();
////    }
////    else {
////      openDoor();
////    }
//    //Serial.println("Toggle engage");
// // }
//  // If user started toggle switch & is holding the switch, stop the motor
////  else if(toggleSwitch && toggleDoorState == 1) {
////    Serial.println("Outside button is held - pausing motor");
////    stopMotor();
////  }
}


// Recieves the user inputted current time and sets it within internal clock
void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     long int converted_time = convertTimestamp(pctime);
     if(converted_time >= DEFAULT_TIME) {  // Check the integer is a valid time (greater than Jan 1 2013)
       setTime(converted_time); // Sync Arduino clock to the time received on the serial port
     }
  }
}
time_t requestSync() {
  Serial.write(TIME_REQUEST);  
  return 0; // the time will be sent later in response to serial mesg
}

// @return today's sunrise time
int checkSunriseTime() {
  return sunriseData[month_][0];
}

// @return today's sunset time
int checkSunsetTime() {
  return sunriseData[month_][1];
}

// Stops the motor
void stopMotor(){
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);  
}

// @param speed , 0-255
void setMotorSpeed(int speed) {
  analogWrite(enA, speed);   
}

// Close the door 
void closeDoor() {
    // Turn on the motor
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    Serial.println("Closing Door");
    if(closeDoorState == 1) {
      stopMotor();
      doorOpen = false;
      toggleSwitch = false;
      Serial.println("DOOR HAS CLOSED - STOPPING MOTOR");
    }
}

// Open the door
void openDoor() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    Serial.println("Opening Door");
    if(openDoorState == 1) {
      stopMotor();
      doorOpen = true;
      toggleSwitch = false;
      Serial.println("DOOR HAS OPENED - STOPPING MOTOR");
    }
}

// Close the door 
void testCloseDoor() {
    // Turn on the motor
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    Serial.println("Closing Door");
    if(closeDoorState == 1) {
      stopMotor();
      doorOpen = false;
      Serial.println("DOOR HAS CLOSED - STOPPING MOTOR");
      delay(3000);
    }
}

// Open the door
void testOpenDoor() {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    Serial.println("Opening Door");
    if(openDoorState == 1) {
      stopMotor();
      doorOpen = true;
      Serial.println("DOOR HAS OPENED - STOPPING MOTOR");
      delay(3000);
    }
}

void testDoor() {
  if(!doorOpen) {
    testOpenDoor();
  }
  else {
    testCloseDoor();
  }
}

void debounceOpenDoorState(){
  openDoorStateVal = digitalRead(openDoorPin);

  if ((unsigned long)(millis() - lastDebounceTime) > debounceDelay) {
    openDoorStateVal_2 = digitalRead(openDoorPin);

    if(openDoorStateVal == openDoorStateVal_2) {
      if(openDoorStateVal != openDoorState) {
        openDoorState = openDoorStateVal;
      }
    }
  }
}

void debounceCloseDoorState(){
  closeDoorStateVal = digitalRead(closeDoorPin);

  if ((unsigned long)(millis() - lastDebounceTime) > debounceDelay) {
    closeDoorStateVal_2 = digitalRead(closeDoorPin);

    if(closeDoorStateVal == closeDoorStateVal_2) {
      if(closeDoorStateVal != closeDoorState) {
        closeDoorState = closeDoorStateVal;
      }
    }
  }
}


void debounceToggleDoorState(){
  toggleDoorStateVal = digitalRead(toggleDoorPin);

  if ((unsigned long)(millis() - lastDebounceTime) > debounceDelay) {
    toggleDoorStateVal_2 = digitalRead(toggleDoorPin);

    if(toggleDoorStateVal == toggleDoorStateVal_2) {
      if(toggleDoorStateVal != toggleDoorState) {
        toggleDoorState = toggleDoorStateVal;
      }
      Serial.println(toggleDoorState);
    }
  }
}

void readTempHumid() {
  int chk = DHT.read11(DHT11_PIN);
  temp = (DHT.temperature * 9/5) + 32;
//  Serial.print("Temperature = ");
//  Serial.println(temp);
  humidity = DHT.humidity;
//  Serial.print("Humidity = ");
//  Serial.print(DHT.humidity);
//  Serial.println("%");
  delay(1000);  
}

// Toggle light using temperature
void toggleLight() {
  if(temp <= maxTemp) {
    Serial.println("Light on");
    digitalWrite(relayPin, LOW);
  }
  else {
    Serial.println("Light off");
    digitalWrite(relayPin, HIGH);
  }
}

// Toggle the fan based on temp/humidity
void toggleFan(){
  if(humidity >= 40 && temp < 80) {
     Serial.println("Fan on");
     digitalWrite(fanPin, LOW);
  }
  else if(temp >= 80 && humidity <= 40) {
     Serial.println("Fan on");
     digitalWrite(fanPin, LOW);
  }
  else if(temp >= 80 || humidity >= 40){
     Serial.println("Fan on");
     digitalWrite(fanPin, LOW);
  }
  else if(temp < 80 && humidity < 40){  // Turn fan off
    Serial.println("Fan off");
    digitalWrite(fanPin, HIGH);
  }
}

// Convert GMT to our timezone and add T and leading zero
long int convertTimestamp(long int GMT_time){
  long int converted_time = GMT_time - 14400;
  int digitCount = floor(log10(abs(converted_time))) + 1;

  if(digitCount < 10) {
    converted_time *= pow(10, digitCount);
  }
  Serial.print("Newly converted local time: ");
  Serial.println(converted_time);
  return converted_time;
}

void loop() {
  debounceCloseDoorState();
  debounceOpenDoorState();
  //debounceToggleDoorState();
  
  readTempHumid();
  toggleLight();
  toggleFan();
  timeClock();

  writeToLCD(0,0,"Temp: ", temp);
  writeToLCD(0,1,"Humidity: ", humidity);
  //testDoor();
  //handleDoorToggle();
}
