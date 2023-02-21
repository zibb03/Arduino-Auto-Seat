// 조수석 시트 코드
#include <Stepper.h> // 젖힘 모터 라이브러리

// 젖힘 모터 관련
// 2048 : 한바퀴(360도), 1024 : 반바퀴(180도)...
// 모터 드라이브에 연결된 핀 IN4, IN2, IN3, IN1
Stepper reclineMotor(2048, 13, 11, 12, 10);

// 수평 방향 모터 관련
const int stepsPerRevolution = 1; // 운전석 시트와 모터 회전수 통일을 위해 설정한 값 
// 수평 방향 모터 변수
const int dirPin = 6;
const int stepPin = 7;

// 버튼 변수
const byte interruptPin1 = 18;
const byte interruptPin2 = 19;
const byte interruptPin3 = 20;
const byte interruptPin4 = 21;

// 젖힘 모터 상태 변수
volatile byte state1 = LOW;
volatile byte state2 = LOW;
volatile byte state3 = LOW;
volatile byte state4 = LOW;

// 시리얼 통신 정보 수신 변수
String cmd;

// 시리얼 통신 정보 송신 변수
String message;
int step1 = 0; // 젖힘 모터 회전수 변수
int step2 = 0; // 수평 모터 회전수 변수

void setup() {
  // 젖힘 모터 속도
  reclineMotor.setSpeed(2);

  //수평 모터 핀 할당
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // 버튼 핀 할당
  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(interruptPin2, INPUT_PULLUP); 
   
  // 시리얼 통신 시작 (boadrate : 9600)
  Serial.begin(9600);

  // 젖힘 모터 관련 인터럽트 설정
  attachInterrupt(digitalPinToInterrupt(interruptPin1), seatReclineBack, RISING);   // 버튼의 임피던스가 상승 시 인터럽트 동작
  attachInterrupt(digitalPinToInterrupt(interruptPin2), seatReclineForth, RISING);   // 버튼의 임피던스가 상승 시 인터럽트 동작
  attachInterrupt(digitalPinToInterrupt(interruptPin3), seatBack, RISING);   // 버튼의 임피던스가 상승 시 인터럽트 동작
  attachInterrupt(digitalPinToInterrupt(interruptPin4), seatForth, RISING);   // 버튼의 임피던스가 상승 시 인터럽트 동작

}

void loop() {
  // 컴퓨터로부터 시리얼 통신이 전송되면, cmd 변수에 입력
  if(Serial.available()){
    // 시리얼 통신 수신
    cmd = Serial.readString();
        
    // 수신한 문자열 쉼표 기준 분할
    int first = cmd.indexOf(",");// 첫 번째 콤마 위치
    int second = cmd.indexOf(",", first + 1); // 두 번째 콤마 위치
    int third = cmd.indexOf(",", second + 1); // 세 번째 콤마 위치
    int forth = cmd.indexOf(",", third + 1); // 네 번째 콤마 위치
    int length = cmd.length(); // 문자열 길이
   
    String str1 = cmd.substring(0, first); // 명령 토큰 
    String str2 = cmd.substring(first + 1, second); // 젖힘 모터 회전수(사용자) 토큰 
    String str3 = cmd.substring(second + 1, third); // 젖힘 모터 회전수(현재 좌석) 토큰
    String str4 = cmd.substring(third + 1, forth); // 수평 모터 회전수(사용자) 토큰
    String str5 = cmd.substring(forth + 1, length); // 수평 모터 회전수(현재 좌석) 토큰
    
    // 모터 회전수 변수가 초기 값이 아닐 시 기존 값을 유지하도록 함
    if (step1 == 0)
      step1 = str3.toInt();
    if (step2 == 0)
      step2 = str5.toInt();  

    // 송신할 문자열 편집
    message = String(step1) + ',' + String(step2) + ',' + String(step3);
    // 시리얼 통신 송신
    Serial.println(message);

    if(str1.toInt() == 1){
      delay(2000);
      Serial.println("user setting");

      // 수평 모터 제어 부분
      // (str5.toInt() - str4.toInt()) : (현재 좌석 모터 회전수 - 사용자 좌석 모터 회전수)

      // 수평 모터를 뒤쪽으로 이동해야 할 때 
      if (str5.toInt() - str4.toInt() > 0) {
        digitalWrite(dirPin, HIGH);
        for(int x = 0; x < (str5.toInt() - str4.toInt()) * stepsPerRevolution; x++)
        {
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(2000); // 모터 회전 속도
          digitalWrite(stepPin, LOW);
          delayMicroseconds(2000); // 모터 회전 속도
        }
        delay(2000);
      }
      //수평 모터를 앞쪽으로 이동해야 할 때
      else {    
        digitalWrite(dirPin, LOW);
        for(int x = 0; x < -(str5.toInt() - str4.toInt()) * stepsPerRevolution; x++)
        {
          digitalWrite(stepPin, HIGH);
          delayMicroseconds(2000); // 모터 회전 속도
          digitalWrite(stepPin, LOW);
          delayMicroseconds(2000); // 모터 회전 속도
        }
        delay(2000);
      }
      step2 += str5.toInt() - str4.toInt()

      // 젖힘 모터 제어 부분
      // (str3.toInt() - str2.toInt()) >> (현재 좌석 모터 회전수 - 사용자 좌석 모터 회전수)
      
      reclineMotor.step(-(str3.toInt() - str2.toInt())); // 모터 배치가 mainSeat.ino와 반대 방향이어서 -부호 붙임     
      delay(2000);

      step1 += -(str3.toInt() - str2.toInt()); // 동작한 만큼 모터 회전수 더함, 모터 배치가 mainSeat.ino와 반대 방향이어서 -부호 붙임
    }
  }
  else {
    // 인터럽트 발생 시 모터를 회전할 수 있도록 함
    if (state1 == HIGH){
      reclineMotor.step(-1); // 모터 배치가 mainSeat.ino와 반대 방향이어서 -부호 붙임 
      step1++;
      }
    if (state2 == HIGH){
      reclineMotor.step(1); // 모터 배치가 mainSeat.ino와 반대 방향이어서 -부호 붙임 
      step1--;
    }
    if (state3 == HIGH){
      digitalWrite(dirPin, HIGH);
      for(int x = 0; x < stepsPerRevolution; x++)
      {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(2000); // 모터 회전 속도
        digitalWrite(stepPin, LOW);
        delayMicroseconds(2000); // 모터 회전 속도
      }
      step2++;
      }
    if (state4 == HIGH){
      digitalWrite(dirPin, LOW);
      for(int x = 0; x < stepsPerRevolution; x++)
      {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(2000); // 모터 회전 속도
        digitalWrite(stepPin, LOW);
        delayMicroseconds(2000); // 모터 회전 속도
      }
      step2--;
    }
  }
}

void seatReclineBack() {
  state1 = !state1;
}

void seatReclineForth(){
  state2 = !state2;
}

void seatBack() {
  state3 = !state3;
}

void seatForth(){
  state4 = !state4;
}