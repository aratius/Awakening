#include <M5StickCPlus.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRac.h>
#include <IRutils.h>

#define IR_RECV_PIN  33
//#define IR_SEND_PIN   GPIO_NUM_9
#define IR_SEND_PIN   32

IRrecv irrecv(IR_RECV_PIN, 1024, 50, true);
decode_results results;
IRsend irsend(IR_SEND_PIN);

#define IR_MAX_SEND_DATA 300
uint16_t send_data[IR_MAX_SEND_DATA];
uint16_t send_len = 0;

unsigned long recvStartTime;
unsigned long recvDuration = 0;

void print_screen(String message, int font_size = 2){
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(font_size);

  M5.Lcd.println(message);
}

void print_screen_next(String message){
  M5.Lcd.println(message);
}

void setup() {
  M5.begin();
  M5.IMU.Init();

  irsend.begin();

  // Start Serial
  Serial.begin(9600);

  M5.Lcd.fillScreen(WHITE);                   // 「分」が変わったら画面全体を書き換え Rewrite the entire screen when the "minute" changes.

  ir_start(1000 * 60);  // 60sec
}

void loop() {
  M5.update();

  if ( M5.BtnA.wasReleased() ) {
    // M5Stick-Cのボタンが押下されたら、直近のリモコン信号を送信
    Serial.println("BtnA.released");
    print_screen("BtnA.released start", 2);
    if( send_len > 0 )
      irsend.sendRaw((uint16_t*)send_data, send_len, 38);
    print_screen_next("BtnA.released end");
  }

  if (irrecv.decode(&results)) {
    // リモコン信号の受信が受信された
    irrecv.resume(); // Receive the next value

    if( results.rawlen <= IR_MAX_SEND_DATA ){
      ir_stop();

      // 受信したリモコン信号をバッファに格納
      uint16_t * result = resultToRawArray(&results);
      send_len = getCorrectedRawLength(&results);
      for( int i = 0 ; i < send_len ; i++ )
        send_data[i] = result[i];
      delete[] result;

      Serial.println("IR received");
      print_screen_next("IR received");
    }else{
      Serial.println("IR size over");
      print_screen_next("IR size over");
    }

    Serial.print(resultToHumanReadableBasic(&results));
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length())
      Serial.println("Mesg Desc.: " + description);
//    Serial.println(resultToTimingInfo(&results));
    Serial.println(resultToSourceCode(&results));

    delay(100);
  }

  if( recvDuration > 0 ){
    // リモコン受信待ち時間タイムアウト
    unsigned long elapsed = millis() - recvStartTime;
    if( elapsed >= recvDuration ){
      Serial.println("Expired");
      ir_stop();
    }
  }

}


void ir_start(unsigned long duration){
  if( duration == 0 ){
    ir_stop();
  }else{
    if( recvDuration == 0 ){
      irrecv.enableIRIn();
    }

    send_len = 0;
    recvStartTime = millis();
    recvDuration = duration;

    print_screen("IR scan start", 2);
  }
}

void ir_stop(void){
  if( recvDuration > 0 ){
    pinMode(IR_RECV_PIN, OUTPUT);
    recvDuration = 0;

    print_screen_next("IR scan stopped");
  }
}
