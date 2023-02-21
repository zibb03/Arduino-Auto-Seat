// 뒷자석 시트 코드
#include <Stepper.h> // 젖힘 모터 라이브러리

// 젖힘 모터 관련
// 2048 : 한바퀴(360도), 1024 : 반바퀴(180도)...

// 모터 드라이브에 연결된 핀 IN4, IN2, IN3, IN1
Stepper reclineMotor(2048, 13, 11, 12, 10);

// 버튼 변수
const byte interruptPin1 = 2;
const byte interruptPin2 = 3;

// 젖힘 모터 상태 변수
volatile byte state1 = LOW;
volatile byte state2 = LOW;

// 시리얼 통신 정보 수신 변수
String cmd;

// 시리얼 통신 정보 송신 변수
String message;
int step1 = 0; // 젖힘 모터 회전수 변수
int step2 = 0; // 수평 모터 회전수 변수

void setup() {
  // 버튼 핀 할당 
  pinMode(interruptPin1, INPUT_PULLUP);
  pinMode(interruptPin2, INPUT_PULLUP); 
   
  // 시리얼 통신 시작 (boadrate : 9600)
  Serial.begin(9600);

  // 젖힘 모터 속도
  reclineMotor.setSpeed(2);

  // 젖힘 모터 관련 인터럽트 설정
  attachInterrupt(digitalPinToInterrupt(interruptPin1), seatReclineBack, RISING);   // 버튼의 임피던스가 상승 시 인터럽트 동작
  attachInterrupt(digitalPinToInterrupt(interruptPin2), seatReclineForth, RISING);   // 버튼의 임피던스가 상승 시 인터럽트 동작
}

void loop() {
  // 컴퓨터로부터 시리얼 통신이 전송되면, cmd 변수에 입력
  if(Serial.available()){
    // 시리얼 통신 수신
    cmd = Serial.readString();
    
    // 수신한 문자열 쉼표 기준 분할
    int first = cmd.indexOf(",");// 첫 번째 콤마 위치
    int second = cmd.indexOf(",", first + 1); // 두 번째 콤마 위치
    int length = cmd.length(); // 문자열 길이
   
    // 모터 회전수 변수가 초기 값이 아닐 시 기존 값을 유지하도록 함
    String str1 = cmd.substring(0, first); // 명령 토큰 
    String str2 = cmd.substring(first + 1, second); // 젖힘 모터 회전수(사용자) 토큰 
    String str3 = cmd.substring(second + 1, length); // 젖힘 모터 회전수(현재 좌석) 토큰
    
    // 모터 회전수 변수가 초기 값이 아닐 시 기존 값을 유지하도록 함
    if (step1 == 0)
      step1 = str3.toInt();

    // 송신할 문자열 편집
    message = String(step1) + ',' + String(step2) + ',' + String(0);
    // 시리얼 통신 송신
    Serial.println(message);

    if(str1.toInt() == 1){
      delay(2000);
      Serial.println("user setting");
      
      // 젖힘 모터 제어 부분
      // (str3.toInt() - str2.toInt()) >> (현재 좌석 모터 회전수 - 사용자 좌석 모터 회전수)

      reclineMotor.step(str3.toInt() - str2.toInt()); // 스텝 모터 위치에 따라 부호 바꾸어줄 필요 있음     
      delay(2000);

      step1 += str3.toInt() - str2.toInt(); // 동작한 만큼 모터 회전수 더함, 스텝 모터 위치에 따라 부호 바꾸어 줄 필요 있음
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
  }
}

void seatReclineBack() {
  state1 = !state1;
}

void seatReclineForth(){
  state2 = !state2;
}
