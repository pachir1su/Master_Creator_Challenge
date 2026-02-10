#include <Keypad.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SERVO_PIN 10           // 서보 모터 핀
#define BUZZER_PIN 11          // 버저 핀
#define BLUE_RGB_PIN 13        // RGB LED 파란색 핀
#define RED_RGB_PIN 12         // RGB LED 빨간색 핀
#define TRAFFIC_GREEN_PIN A1   // 신호등 초록 핀 (문 열림 상태 표시)
#define TRAFFIC_RED_PIN A0     // 신호등 빨강 핀 (문 잠금 상태 표시)
#define OPEN_BUTTON_PIN A2     // 문 열림 버튼
#define CLOSE_BUTTON_PIN A3    // 문 잠금 버튼

// 음의 주파수 설정 (도, 레, 미)
#define NOTE_DO  262
#define NOTE_RE  294
#define NOTE_MI  330

// 서보 모터 설정
Servo servo;
int unlockAngle = 90;  // 열림 상태의 각도
int lockAngle = 0;     // 잠금 상태의 각도
bool isDoorLocked = true;  // 초기 상태는 잠금 상태

// 4x4 키패드 설정
const byte ROWS = 4;  // 행 개수
const byte COLS = 4;  // 열 개수
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {2, 3, 4, 5};  // Row4 -> D2, Row3 -> D3, Row2 -> D4, Row1 -> D5
byte colPins[COLS] = {6, 7, 8, 9};  // Col1 -> D6, Col2 -> D7, Col3 -> D8, Col4 -> D9
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C 주소 0x27로 설정
String password = "724070";             // 초기 설정 비밀번호
String inputPassword = "";            // 입력된 비밀번호

void setup() {
  Serial.begin(9600);
  servo.attach(SERVO_PIN);
  servo.write(lockAngle);  // 초기 잠금 상태 각도
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLUE_RGB_PIN, OUTPUT);
  pinMode(RED_RGB_PIN, OUTPUT);
  pinMode(TRAFFIC_GREEN_PIN, OUTPUT);
  pinMode(TRAFFIC_RED_PIN, OUTPUT);
  pinMode(OPEN_BUTTON_PIN, INPUT_PULLUP);  // 문 열림 버튼
  pinMode(CLOSE_BUTTON_PIN, INPUT_PULLUP); // 문 잠금 버튼

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  lcd.print("Enter Password:");

  // 기본 LED 상태: 빨간 신호등(잠금 상태)
  digitalWrite(BLUE_RGB_PIN, LOW);
  digitalWrite(RED_RGB_PIN, LOW);
  digitalWrite(TRAFFIC_GREEN_PIN, LOW);
  digitalWrite(TRAFFIC_RED_PIN, HIGH);
}

void loop() {
  // 키패드 입력 감지
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
    tone(BUZZER_PIN, NOTE_DO, 100);
    delay(100);
    noTone(BUZZER_PIN);

    // 비밀번호 확인 모드
    if (key == '#') {  // '#'을 누르면 비밀번호 확인
      if (inputPassword == password) {
        unlockDoor();
      } else {
        triggerAlarm();
      }
      inputPassword = "";
    } else if (key == '*') {  // '*'를 누르면 입력 초기화
      inputPassword = "";
      lcd.clear();
      lcd.print("Enter Password:");
    } else if (key == 'C' && !isDoorLocked) {  // 문이 열린 상태에서 'C'를 누르면 잠금
      lockDoor();
    } else {
      inputPassword += key;
      lcd.clear();
      lcd.print("Password: ");
      lcd.print(inputPassword);
    }
  }

  // 버튼으로 문 열기/잠그기
  if (digitalRead(OPEN_BUTTON_PIN) == LOW && isDoorLocked) {  // 문 열림 버튼
    unlockDoor();
    delay(500); // 버튼을 안정적으로 인식하기 위한 딜레이
  } else if (digitalRead(CLOSE_BUTTON_PIN) == LOW && !isDoorLocked) {  // 문 잠금 버튼
    lockDoor();
    delay(500); // 버튼을 안정적으로 인식하기 위한 딜레이
  }
}

void lockDoor() {
  if (!isDoorLocked) {
    servo.write(lockAngle);
    isDoorLocked = true;

    // 신호등 LED: 빨간색 켜기
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

    // 신호등 LED: 초록색 켜기
    digitalWrite(TRAFFIC_GREEN_PIN, HIGH);
    digitalWrite(TRAFFIC_RED_PIN, LOW);

    lcd.clear();
    lcd.print("Access Granted");

    // "도 레 미" 음 순으로 부저 울리기
    tone(BUZZER_PIN, NOTE_DO, 200);
    delay(200);
    tone(BUZZER_PIN, NOTE_RE, 200);
    delay(200);
    tone(BUZZER_PIN, NOTE_MI, 200);
    delay(200);
    noTone(BUZZER_PIN);

    // 문이 열릴 때 RGB LED 잠깐 켜기
    digitalWrite(BLUE_RGB_PIN, HIGH);
    digitalWrite(RED_RGB_PIN, HIGH);
    delay(500);  // 0.5초간 켜짐
    digitalWrite(BLUE_RGB_PIN, LOW);
    digitalWrite(RED_RGB_PIN, LOW);
  }
}

void triggerAlarm() {
  for (int i = 0; i < 5; i++) {  // 5회 깜빡임
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
