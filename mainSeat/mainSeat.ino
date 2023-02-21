// 운전석 시트 코드
#include <Stepper.h> // 젖힘 모터 라이브러리
#include <AFMotor.h> // 핸들, 수평 방향 모터 라이브러리

// 젖힘 모터 관련
// 2048 : 한바퀴(360도), 1024 : 반바퀴(180도)...
const int stepsPerRevolution = 2048; 
// 모터 드라이브에 연결된 핀 IN4, IN2, IN3, IN1
Stepper reclineMotor(stepsPerRevolution, 17, 15, 16, 14);

// 핸들, 수평 방향 모터 관련
// 1스텝당 회전각 1.8˚ * 200스텝 = 360˚
AF_Stepper steeringWheelMotor(200 , 1);  // 1회전당 필요 스텝수(200), 연결 모터채널(1) 
AF_Stepper horizonalMotor(200 , 2);  // 1회전당 필요 스텝수(200), 연결 모터채널(2) 

// 버튼 변수
const byte interruptPin1 = 2;
const byte interruptPin2 = 3;
const byte interruptPin3 = 18;
const byte interruptPin4 = 19;
const byte interruptPin5 = 20;
const byte interruptPin6 = 21;

// 모터 상태 변수
volatile byte state1 = LOW;
volatile byte state2 = LOW;
volatile byte state3 = LOW;
volatile byte state4 = LOW;
volatile byte state5 = LOW;
volatile byte state6 = LOW;

// 시리얼 통신 정보 수신 변수
String cmd;

// 시리얼 통신 정보 송신 변수
String message;
int step1 = 0; // 젖힘 모터 회전수 변수
int step2 = 0; // 수평 모터 회전수 변수
int step3 = 0; // 핸들 모터 회전수 변수

void setup() {
  //버튼 핀 할당
  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(interruptPin2, INPUT_PULLUP);
  pinMode(interruptPin3, INPUT_PULLUP);
  pinMode(interruptPin4, INPUT_PULLUP);
  pinMode(interruptPin5, INPUT_PULLUP);
  pinMode(interruptPin6, INPUT_PULLUP); 
 
  // 시리얼 통신 시작 (boadrate : 9600)
  Serial.begin(9600);

  // 젖힘 모터 속도
  reclineMotor.setSpeed(2);

  // 핸들, 수평 모터 속도 설정
  steeringWheelMotor.setSpeed(60);  // 200 rpm : 범위(0~255)
  horizonalMotor.setSpeed(60);  // 200 rpm : 범위(0~255)

  //모터 관련 인터럽트 설정
  attachInterrupt(digitalPinToInterrupt(interruptPin1), seatReclineBack, RISING); // 버튼의 임피던스가 상승 시 인터럽트 동작
  attachInterrupt(digitalPinToInterrupt(interruptPin2), seatReclineForth, RISING); // 버튼의 임피던스가 상승 시 인터럽트 동작
  attachInterrupt(digitalPinToInterrupt(interruptPin3), seatBack, RISING); // 버튼의 임피던스가 상승 시 인터럽트 동작
  attachInterrupt(digitalPinToInterrupt(interruptPin4), seatForth, RISING); // 버튼의 임피던스가 상승 시 인터럽트 동작
  attachInterrupt(digitalPinToInterrupt(interruptPin3), handleBack, RISING); // 버튼의 임피던스가 상승 시 인터럽트 동작
  attachInterrupt(digitalPinToInterrupt(interruptPin4), handleForth, RISING); // 버튼의 임피던스가 상승 시 인터럽트 동작
}

void loop() {
  // 컴퓨터로부터 시리얼 통신이 전송되면, cmd 변수에 입력
  if(Serial.available()){
    // 시리얼 통신 수신
    cmd = Serial.readString();

    // 수신한 문자열 쉼표 기준 분할
    int first = cmd.indexOf(",");// 첫 번째 콤마 위치
    int second = cmd.indexOf(",",first + 1); // 두 번째 콤마 위치
    int third = cmd.indexOf(",",second + 1); // 세 번째 콤마 위치
    int forth = cmd.indexOf(",",third + 1); // 네 번째 콤마 위치
    int fifth = cmd.indexOf(",",forth + 1); // 다섯 번째 콤마 위치
    int sixth = cmd.indexOf(",",fifth + 1); // 여섯 번째 콤마 위치
    int length = cmd.length(); // 문자열 길이
   
    String str1 = cmd.substring(0, first); // 명령 토큰
    String str2 = cmd.substring(first + 1, second); // 젖힘 모터 회전수(사용자) 토큰 
    String str3 = cmd.substring(second + 1,third); // 젖힘 모터 회전수(현재 좌석) 토큰
    String str4 = cmd.substring(third + 1,forth); // 수평 모터 회전수(사용자) 토큰
    String str5 = cmd.substring(forth + 1,fifth); // 수평 모터 회전수(현재 좌석) 토큰
    String str6 = cmd.substring(fifth + 1,sixth); // 핸들 모터 회전수(사용자) 토큰     
    String str7 = cmd.substring(sixth + 1,length); // 핸들 모터 회전수(현재 좌석) 토큰
    
    // 모터 회전수 변수가 초기 값이 아닐 시 기존 값을 유지하도록 함
    if (step1 == 0)
      step1 = str3.toInt();
    if (step2 == 0)
      step2 = str5.toInt();
    if (step3 == 0)
      step3 = str5.toInt();

    // 송신할 문자열 편집
    message = String(step1) + ',' + String(step2) + ',' + String(step3);
    // 시리얼 통신 송신
    Serial.println(message);

    if(str1.toInt() == 1){
      delay(2000);
      Serial.println("user setting");      
      
      // 수평 모터 제어 부분
      // (str5.toInt() - str4.toInt()): (현재 좌석 모터 회전수 - 사용자 좌석 모터 회전수)

      // 수평 모터를 뒤쪽으로 이동해야 할 때 
      if (str5.toInt() - str4.toInt() > 0) {
        horizonalMotor.step(str5.toInt() - str4.toInt(), BACKWARD, SINGLE); 
        delay(2000);
      }
      // 수평 모터를 앞쪽으로 이동해야 할 때
      else {    
        horizonalMotor.step(-(str5.toInt() - str4.toInt()), FORWARD, SINGLE); 
        delay(2000); 
      }
      step2 += str5.toInt() - str4.toInt();

      // 젖힘 모터 제어 부분
      // (str3.toInt() - str2.toInt()) >> (현재 좌석 모터 회전수 - 사용자 좌석 모터 회전수)

      reclineMotor.step(str3.toInt() - str2.toInt());      
      delay(2000);

      step1 += str3.toInt() - str2.toInt(); // 동작한 만큼 모터 회전수 더함

      // 핸들 모터 제어 부분
      // (str7.toInt() - str6.toInt()): (현재 좌석 모터 회전수 - 사용자 좌석 모터 회전수)

      // 핸들 모터를 뒤쪽으로 이동해야 할 때       
      if (str7.toInt() - str6.toInt() > 0) {
        steeringWheelMotor.step(str7.toInt() - str6.toInt(), BACKWARD, SINGLE); // 2048 : 1회전에 필요한 스텝수
        delay(2000);
      }
      // 핸들 모터를 앞쪽으로 이동해야 할 때
      else {    
        steeringWheelMotor.step(-(str7.toInt() - str6.toInt()), FORWARD, SINGLE); 
        delay(2000); 
      }
      step3 += str7.toInt() - str6.toInt();     
    }
  }
  else {
    // 인터럽트 발생 시 모터를 회전할 수 있도록 함
    if (state1 == HIGH){
      reclineMotor.step(1);
      step1++;
      }
    if (state2 == HIGH){
      reclineMotor.step(-1);
      step1--;
    }
    if (state3 == HIGH){
      horizonalMotor.step(1, BACKWARD, SINGLE);
      step2++;
      }
    if (state4 == HIGH){
      horizonalMotor.step(-1, FORWARD, SINGLE);
      step2--;
    }
    if (state5 == HIGH){
      steeringWheelMotor.step(1, BACKWARD, SINGLE);
      step3++;
      }
    if (state6 == HIGH){
      steeringWheelMotor.step(-1, FORWARD, SINGLE);
      step3--;
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

void handleBack() {
  state5 = !state5;
}

void handleForth(){
  state6 = !state6;
}