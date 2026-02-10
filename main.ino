#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>

#define SERVO_PIN 10
#define BUZZER_PIN 11
#define BLUE_RGB_PIN 13
#define RED_RGB_PIN 12
#define TRAFFIC_GREEN_PIN A1
#define TRAFFIC_RED_PIN A0
#define OPEN_BUTTON_PIN A2
#define CLOSE_BUTTON_PIN A3

// 음의 주파수 설정 (도, 레, 미)
#define NOTE_DO  262
#define NOTE_RE  294
#define NOTE_MI  330

SoftwareSerial Bluetooth(1, 0);  // Bluetooth 모듈의 RX, TX 핀

Servo servo;
int unlockAngle = 90;
int lockAngle = 0;
bool isDoorLocked = true;

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {2, 3, 4, 5};
byte colPins[COLS] = {6, 7, 8, 9};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);
String password = "123456";
String inputPassword = "";

void setup() {
  Serial.begin(9600);
  Bluetooth.begin(9600);
  servo.attach(SERVO_PIN);
  servo.write(lockAngle);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLUE_RGB_PIN, OUTPUT);
  pinMode(RED_RGB_PIN, OUTPUT);
  pinMode(TRAFFIC_GREEN_PIN, OUTPUT);
  pinMode(TRAFFIC_RED_PIN, OUTPUT);
  pinMode(OPEN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CLOSE_BUTTON_PIN, INPUT_PULLUP);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  lcd.print("Enter Password:");

  digitalWrite(BLUE_RGB_PIN, LOW);
  digitalWrite(RED_RGB_PIN, LOW);
  digitalWrite(TRAFFIC_GREEN_PIN, LOW);
  digitalWrite(TRAFFIC_RED_PIN, HIGH);
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    tone(BUZZER_PIN, NOTE_DO, 100);
    delay(100);
    noTone(BUZZER_PIN);

    if (key == '#') {
      if (inputPassword == password) {
        unlockDoor();
      } else {
        triggerAlarm();
      }
      inputPassword = "";
    } else if (key == '*') {
      inputPassword = "";
      lcd.clear();
      lcd.print("Enter Password:");
    } else if (key == 'C' && !isDoorLocked) {
      lockDoor();
    } else {
      inputPassword += key;
      lcd.clear();
      lcd.print("Password: ");
      lcd.print(inputPassword);
    }
  }

  if (digitalRead(OPEN_BUTTON_PIN) == LOW && isDoorLocked) {
    unlockDoor();
    delay(500);
  } else if (digitalRead(CLOSE_BUTTON_PIN) == LOW && !isDoorLocked) {
    lockDoor();
    delay(500);
  }
}

void lockDoor() {
  if (!isDoorLocked) {
    servo.write(lockAngle);
    isDoorLocked = true;
    Bluetooth.println("LOCK");  // 슬레이브에 잠금 명령 전송

    digitalWrite(TRAFFIC_GREEN_PIN, LOW);
    digitalWrite(TRAFFIC_RED_PIN, HIGH);
    
    lcd.clear();
    lcd.print("Door Locked");
  }
}

void unlockDoor() {
  if (isDoorLocked) {
    servo.write(unlockAngle);
    isDoorLocked = false;
    Bluetooth.println("OPEN");  // 슬레이브에 열림 명령 전송

    digitalWrite(TRAFFIC_GREEN_PIN, HIGH);
    digitalWrite(TRAFFIC_RED_PIN, LOW);

    lcd.clear();
    lcd.print("Access Granted");

    tone(BUZZER_PIN, NOTE_DO, 200);
    delay(200);
    tone(BUZZER_PIN, NOTE_RE, 200);
    delay(200);
    tone(BUZZER_PIN, NOTE_MI, 200);
    delay(200);
    noTone(BUZZER_PIN);

    digitalWrite(BLUE_RGB_PIN, HIGH);
    digitalWrite(RED_RGB_PIN, HIGH);
    delay(500);
    digitalWrite(BLUE_RGB_PIN, LOW);
    digitalWrite(RED_RGB_PIN, LOW);
  }
}

void triggerAlarm() {
  Bluetooth.println("ALERT");  // 슬레이브에 경고 명령 전송
  for (int i = 0; i < 5; i++) {
    digitalWrite(TRAFFIC_RED_PIN, HIGH);
    digitalWrite(RED_RGB_PIN, HIGH);
    tone(BUZZER_PIN, 1000);
    lcd.clear();
    lcd.print("Access Denied");
    delay(500);

    digitalWrite(TRAFFIC_RED_PIN, LOW);
    digitalWrite(RED_RGB_PIN, LOW);
    noTone(BUZZER_PIN);
    delay(500);
  }
  lockDoor();
  inputPassword = "";
  lcd.clear();
  lcd.print("Enter Password:");
}
