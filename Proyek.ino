#include <NewPing.h>
#include <SoftwareSerial.h>

// --- Konfigurasi Ultrasonic ---
#define MAX_DISTANCE 200
#define TRIG_PIN 

// Definisi Pin Echo
#define ECHO_FRONT 
#define ECHO_LEFT  
#define ECHO_RIGHT 

NewPing sonarFront(TRIG_PIN, ECHO_FRONT, MAX_DISTANCE);
NewPing sonarLeft(TRIG_PIN, ECHO_LEFT, MAX_DISTANCE);
NewPing sonarRight(TRIG_PIN, ECHO_RIGHT, MAX_DISTANCE);

int distFront, distLeft, distRight;

// --- Konfigurasi IR (Line Follower) 
#define irRight 
#define irLeft 

// --- Konfigurasi Motor L298N ---
#define in1L298N 
#define in2L298N 
#define in3L298N 
#define in4L298N 
// Pin PWM (Kecepatan)
#define speed1L298N 
#define speed2L298N 

// --- LED & Button ---
#define led1     // HANYA 1 LED
#define button    // Tombol ganti mode

// --- Bluetooth HC-05 ---
#define unoRX 
#define unoTX 
SoftwareSerial HC05(unoRX, unoTX);

// --- Variabel Global ---
int rightStatus, leftStatus;
byte buttonValue = 0;
byte prevButtonValue = 0;
byte mode = 0; // 0: Idle, 1: WF, 2: LF, 3: BT

int speedNormal = 150; 
// Jika ingin kecepatan berbeda saat belok, ubah ini:
int speedTurn = 180;   
String voice;

// Variabel untuk Blinking LED
unsigned long previousMillis = 0;
boolean ledState = LOW;
long blinkInterval = 0; 

void setup() {
  Serial.begin(9600);
  HC05.begin(9600);

  pinMode(irRight, INPUT);
  pinMode(irLeft, INPUT);
  pinMode(button, INPUT_PULLUP);

  pinMode(in1L298N, OUTPUT);
  pinMode(in2L298N, OUTPUT);
  pinMode(in3L298N, OUTPUT);
  pinMode(in4L298N, OUTPUT);
  pinMode(speed1L298N, OUTPUT);
  pinMode(speed2L298N, OUTPUT);

  pinMode(led1, OUTPUT);

  // Set kecepatan awal default
  analogWrite(speed1L298N, speedNormal);
  analogWrite(speed2L298N, speedNormal);
}

void loop() {
  // Cek Tombol Mode
  buttonValue = digitalRead(button);
  if ((buttonValue == LOW) && (prevButtonValue == 0)) {
    mode++;
    prevButtonValue = 1;
    motorControl(LOW, LOW, LOW, LOW); 
    delay(200); 
  } else if (buttonValue == HIGH) {
    prevButtonValue = 0;
  }
  
  if (mode > 3) { 
    mode = 0;
  }

  // Atur Indikator 1 LED 
  handleSingleLed(mode);

  // Eksekusi Mode 
  switch (mode) {
    case 0: // Idle
      motorControl(LOW, LOW, LOW, LOW);
      break;

    case 1: 
      WFMode();
      break;

    case 2: 
      LFMode();
      break;

    case 3: 
      BTMode();
      break;
  }
}

// Fungsi Indikator 1 LED 
void handleSingleLed(int currentMode) {
  unsigned long currentMillis = millis();

  if (currentMode == 0) {
    digitalWrite(led1, LOW);
    ledState = LOW; 
  }
  else if (currentMode == 3) {
    digitalWrite(led1, HIGH);
    ledState = HIGH;
  }
  else {
    if (currentMode == 1) blinkInterval = 800; // WF lambat
    else if (currentMode == 2) blinkInterval = 150; // LF cepat

    if (currentMillis - previousMillis >= blinkInterval) {
      previousMillis = currentMillis;
      ledState = !ledState;
      digitalWrite(led1, ledState);
    }
  }
}


void motorControl (int in1Value, int in2Value, int in3Value, int in4Value) {
  digitalWrite(in1L298N, in1Value);
  digitalWrite(in2L298N, in2Value);
  digitalWrite(in3L298N, in3Value);
  digitalWrite(in4L298N, in4Value);
  analogWrite(speed1L298N, speedNormal); 
  analogWrite(speed2L298N, speedNormal);
}

// --- WF Mode 
void WFMode() {
  distFront = sonarFront.ping_cm(); delay(10);
  distLeft  = sonarLeft.ping_cm();  delay(10);
  distRight = sonarRight.ping_cm(); delay(10);
  
  if (distFront == 0) distFront = 200;
  if (distLeft == 0) distLeft = 200;

  if (distFront < 15) {
    motorControl(LOW, HIGH, HIGH, LOW); // Kanan
    delay(100);
  } 
  else if (distLeft < 10) { 
    motorControl(HIGH, LOW, LOW, LOW); // Menjauh
  }
  else if (distLeft > 25) { 
    motorControl(LOW, LOW, HIGH, LOW); // Mendekat
  }
  else {
    motorControl(HIGH, LOW, HIGH, LOW); // Lurus
  }
}

// --- LF Mode 
void LFMode() {
  rightStatus = digitalRead(irRight);
  leftStatus = digitalRead(irLeft);

  if (leftStatus == 0 && rightStatus == 0) {
    motorControl(HIGH, LOW, HIGH, LOW);
  } else if (leftStatus == 0 && rightStatus == 1) {
    motorControl(LOW, HIGH, HIGH, LOW);
  } else if (leftStatus == 1 && rightStatus == 0) {
    motorControl(HIGH, LOW, LOW, HIGH);
  } else if (leftStatus == 1 && rightStatus == 1) {
    motorControl(LOW, LOW, LOW, LOW);
  }
}

// --- BT Mode ---
void BTMode() {
  // Metode 1: String Voice (Aplikasi Voice Control)
  while (HC05.available()) {
    delay(10);
    char c = HC05.read();
    voice += c;
  }

  if (voice.length() > 0) {
    Serial.println(voice); // Debugging
    if (voice == "maju") 
    motorControl(HIGH, LOW, HIGH, LOW);
    else if (voice == "mundur") 
    motorControl(LOW, HIGH, LOW, HIGH);
    else if (voice == "kanan") 
    motorControl(LOW, HIGH, HIGH, LOW);
    else if (voice == "kiri") 
    motorControl(HIGH, LOW, LOW, HIGH);
    else motorControl(LOW, LOW, LOW, LOW);
    
    voice = ""; 
  }
  
}
