#include <HC595.h>

typedef struct {
  int dumpsterPin;
  int dumpsterIdentifier;
  int ledPosition;
  int startNode;
  int endNode;
} dumpsterType;

unsigned long cycleTime = 0;

const int chipCount = 4;  // Number of serialy connected 74HC595 (8 maximum)
const int latchPin = 11;  // Pin ST_CP (12) of the 74HC595
const int clockPin = 10;  // Pin SH_CP (11) of the 74HC595
const int dataPin = 12;   // Pin DS (14) of the 74HC595

const dumpsterType dumpsters[6] = {
  (dumpsterType) {2, 1, 9, 2, 5},
  (dumpsterType) {3, 2, 6, 2, 3},
  (dumpsterType) {4, 3, 11, 5, 2},
  (dumpsterType) {5, 4, 21, 5, 6},
  (dumpsterType) {6, 5, 25, 7, 5},
  (dumpsterType) {7, 6, 30, 6, 8}
};

int currentNode = 7;

bool workfree = true;

//Constructor 
HC595 ledArray(chipCount,latchPin,clockPin,dataPin);

int delayMs = 300;
int lastPin = ledArray.lastPin();

int roadLedPosition = 0;
int cycles = 0;

void setup() {
  ledArray.reset();
  for(int i = 0; i < 6; i++){
    pinMode(dumpsters[i].dumpsterPin, INPUT);
  }
  Serial.begin(9600);
  while (!Serial) {
    ; 
  }   
  Serial.print("Testing");
  Serial.write("Testing");
  ledArray.setPins(1, 32, ON);
  delay(1000);
  ledArray.setPins(1, 32, OFF);
}

void loop() {

  if(Serial){
    if(millis() >= cycleTime && workfree){
      if(currentNode != 7){
        walkhome();
      }
      ledArray.setPins(1, 32, OFF);
      cycles++;
      Serial.print("CYCLE|");
      Serial.println(cycles);
      cycleTime = cycleTime + 60000;
    }
    else{
      for(int j = 0; j < 6; j++){
        if(digitalRead(dumpsters[j].dumpsterPin) == LOW && workfree){
          /*Serial.print("NONE|");
          Serial.println(dumpsters[j].dumpsterIdentifier);*/
          Serial.println("Busy working...");
          travelToDumpster(dumpsters[j]);
        }
      }
    }
  }
  /*int followers = random(1,5);
  for(int i = 0 ; i < 32; i++) {
    ledArray.setPins(0, 32, OFF);
    ledArray.setPin(i, ON);
    delay(delayMs);
  }*/
}
void travelToDumpster(dumpsterType dumpster){
  workfree = false;
  String command = "TRAVEL|" + String(currentNode) + "|" + dumpster.startNode;
  Serial.println(command);
  //Starting route
  while(true){
    delay(1000);
    String led = Serial.readStringUntil(',');
    if(led == "endleds") break;
    ledArray.setPin(atoi(led.c_str()), ON);
  }
  currentNode = dumpster.startNode;
  command = "TRAVEL|" + String(currentNode) + "|" + dumpster.endNode;
  Serial.println(command);
  //Ending route
  while(true){
    String led = Serial.readStringUntil(',');
    if(dumpster.ledPosition == atoi(led.c_str())){
      int blink = OFF;
      while(digitalRead(dumpster.dumpsterPin) == LOW){
        blink = !blink;
        if(blink){   
          ledArray.setPin(atoi(led.c_str()), ON);
        }
        else{
          ledArray.setPin(atoi(led.c_str()), OFF);
        }
        delay(400);
      }
    }
    if(led == "endleds") break;
    ledArray.setPin(atoi(led.c_str()), ON);
    delay(1000);
  }
  delay(3000);
  ledArray.setPins(0, 31, OFF);
  currentNode = dumpster.endNode;
  workfree = true;
}
void walkhome(){
  String command = "TRAVEL|" + String(currentNode) + "|7";
  Serial.println(command);
  while(true){
    String led = Serial.readStringUntil(',');
    if(led == "endleds") break;
    ledArray.setPin(atoi(led.c_str()), ON);
    delay(600);
  }
  delay(1000);
  ledArray.setPins(0, 31, OFF);
  currentNode = 7;
}