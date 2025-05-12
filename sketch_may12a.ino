// Pin definations
const int redLed = 2;
const int yellowLed =3;
const int greenLed =4;
const int redButton = 7;
const int yellowButton = 8;
const int greenButton =9;

//LED states
bool redState = false;
bool yellowState = false;
bool greenState = false;

void setup() {
  pinMode(redLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  pinMode(redButton, INPUT_PULLUP);
  pinMode(yellowButton, INPUT_PULLUP);
  pinMode(greenButton, INPUT_PULLUP);

  Serial.begin(9600);
  sendStatus();
}

bool lastRedBtn =LOW;
bool lastYellowBtn =LOW;
bool lastGreenBtn =LOW;

void loop() {
  bool currRed = digitalRead(redButton);
  bool currYellow = digitalRead(yellowButton);
  bool currGreen = digitalRead(greenButton);

  if (lastRedBtn == HIGH && currRed == LOW){
    toggleLed('R');
    sendStatus();
  }
  if (lastYellowBtn == HIGH && currYellow == LOW){
    toggleLed('G');
    sendStatus();
  }

  lastRedBtn = currRed;
  lastYellowBtn = currYellow;
  lastGreenBtn = currGreen;

  //Apply LED States
  digitalWrite(redLed, redState ? HIGH : LOW);
  digitalWrite(yellowLed, yellowState ? HIGH : LOW);
  digitalWrite(greenLed, greenState ? HIGH : LOW);

  //Serial input
  if(Serial.available()){
    char cmd = Serial.read();
    toggleLed(cmd);
    sendStatus();
  }
}

void toggleLed(char cmd) {
  if (cmd == 'R') {
    redState = !redState;
    if(redState) { yellowState = false; greenState = false; }
  } else if(cmd =='Y') { 
    yellowState = !yellowState; 
    if (yellowState) { redState = false; greenState = false; }
  } else if (cmd == 'G') {
    greenState =! greenState;
    if (greenState) { redState =false; yellowState =false; }
  }
}

void sendStatus () {
  Serial.print("STATE:R");
  Serial.print(redState ? "1" : "o");
  Serial.print("Y");
  Serial.print(yellowState ? "1" : "0");
  Serial.print("G");
  Serial.println(greenState ? "1" : "0");
}



