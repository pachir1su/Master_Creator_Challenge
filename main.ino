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

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C 주소 0x27로 설정

String password = "1234";     // 초기 설정 비밀번호
String inputPassword = "";    // 입력된 비밀번호

bool isDoorLocked = true;     // 문 잠금 상태

void setup() {
  Serial.begin(9600);
  servo.attach(SERVO_PIN);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BLUE_RGB_PIN, OUTPUT);
  pinMode(RED_RGB_PIN, OUTPUT);
  pinMode(TRAFFIC_GREEN_PIN, OUTPUT);
  pinMode(TRAFFIC_RED_PIN, OUTPUT);

  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
  lcd.print("Enter Password:");
}

void loop() {
  // 키패드 입력 감지
  char key = keypad.getKey();
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);  // 시리얼 모니터에 키 입력 표시

    // 비밀번호 확인 모드
    if (key == '#') {  // '#'을 누르면 비밀번호 확인
      if (inputPassword == password) {
        unlockDoor();
      } else {
        triggerAlarm();
      }
      inputPassword = "";  // 입력 초기화
    } else if (key == '*') {  // '*'를 누르면 입력 초기화
      inputPassword = "";
      lcd.clear();
      lcd.print("Enter Password:");
    } else {
      inputPassword += key;  // 입력한 키 추가
      lcd.clear();
      lcd.print("Password: ");
      lcd.print(inputPassword);  // LCD에 입력 중인 비밀번호 표시
    }
  }
}

void lockDoor() {
  servo.write(0);  // 잠금 위치
  isDoorLocked = true;
  digitalWrite(TRAFFIC_GREEN_PIN, LOW);  // 초록 LED 꺼짐
  digitalWrite(TRAFFIC_RED_PIN, HIGH);   // 빨간 LED 켜짐
  digitalWrite(BLUE_RGB_PIN, HIGH);      // 파란색 RGB LED 켜짐 (잠금 표시)
  digitalWrite(RED_RGB_PIN, LOW);        // 빨간색 RGB LED 꺼짐
  lcd.clear();
  lcd.print("Door Locked");
}

void unlockDoor() {
  servo.write(90);  // 잠금 해제 위치
  isDoorLocked = false;
  digitalWrite(TRAFFIC_GREEN_PIN, HIGH); // 초록 LED 켜짐 (열림 상태 표시)
  digitalWrite(TRAFFIC_RED_PIN, LOW);    // 빨간 LED 꺼짐
  digitalWrite(BLUE_RGB_PIN, LOW);       // 파란색 RGB LED 꺼짐
  digitalWrite(RED_RGB_PIN, HIGH);       // 빨간색 RGB LED 켜짐 (열림 표시)
  lcd.clear();
  lcd.print("Access Granted");
  delay(3000);  // 열림 상태 유지 시간
  lockDoor();   // 다시 잠금
}

void triggerAlarm() {
  // 경고: 빨간 신호등 깜빡임, 버저, LCD 경고 메시지
  for (int i = 0; i < 5; i++) {  // 5회 깜빡임
    digitalWrite(TRAFFIC_RED_PIN, HIGH); // 빨간 LED 깜빡임
    tone(BUZZER_PIN, 1000);              // 버저 울림
    lcd.clear();
    lcd.print("Access Denied");
    delay(500);

    digitalWrite(TRAFFIC_RED_PIN, LOW);
    noTone(BUZZER_PIN);                  // 버저 끄기
    delay(500);
  }

  // 경고 후 초기화 및 잠금 상태로 복귀
  lockDoor();                // 잠금 유지
  inputPassword = "";        // 입력 초기화
  lcd.clear();               // LCD 초기화
  lcd.print("Enter Password:"); // 비밀번호 입력 대기 화면
}
