// #include <M5Stack.h>
#include <M5StickCPlus.h>
#include <WiFi.h>
#include "time.h"

const char* ssid       = "HUMAX-BD2EB";
const char* password   = "MjdjMmNxMEgaX";

const char* ntpServer = "ntp.nict.jp";
const long  gmtOffset_sec = 3600 * 9;
const int   daylightOffset_sec = 0;

int smin = 0;  // 画面書き換え判定用min
bool isActive = false;
bool hasReachedTime = false;
int mode = 0;  // 0: normal, 1:hour, 2:minute
int alarmHour = 0;
int alarmMinute = 0;

bool checkHasReached() {
  struct tm timeinfo;

  // timeinfo.tm_year + 1900
  // timeinfo.tm_mon + 1
  // timeinfo.tm_mday
  // timeinfo.tm_hour
  // timeinfo.tm_min
  // timeinfo.tm_sec

  if(!getLocalTime(&timeinfo)){
    M5.Lcd.println("Failed to obtain time");
    return false;
  }
  if(
    !hasReachedTime && 
    timeinfo.tm_hour == alarmHour &&
    timeinfo.tm_min == alarmMinute
  ) return true;
  return false;
}

void printLocalTime()
{
  struct tm timeinfo;

  // timeinfo.tm_year + 1900
  // timeinfo.tm_mon + 1
  // timeinfo.tm_mday
  // timeinfo.tm_hour
  // timeinfo.tm_min
  // timeinfo.tm_sec

  if(!getLocalTime(&timeinfo)){
    M5.Lcd.println("Failed to obtain time");
    return;
  }
  
  // 画面書き換え処理　Screen rewriting process.
  if (smin == timeinfo.tm_min) {         // 分単位の変更がかかったかどうか確認
    M5.Lcd.fillRect(140, 8, 20, 10, BLACK);    // 「秒」だけが変わった場合、秒表示エリアだけ書き換え Rewrite only the display area of seconds.
  } else {
    M5.Lcd.fillScreen(BLACK);                   // 「分」が変わったら画面全体を書き換え Rewrite the entire screen when the "minute" changes.
    printIsActive();
  }
  smin = timeinfo.tm_min;

  // 日付表示
  M5.Lcd.setTextFont(1);                      // 1:Adafruit 8ピクセルASCIIフォント
  M5.Lcd.setTextColor(WHITE);                 //日付表示文字だけ白色の文字色にする
  M5.Lcd.setCursor(10, 10, 1);                //x,y,font 1:Adafruit 8ピクセルASCIIフォント
  M5.Lcd.printf("Date:%04d.%02d.%02d.", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
  M5.Lcd.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min); 
  M5.Lcd.printf(":%02d\n", timeinfo.tm_sec); // 秒を表示
  
}

void printIsActive() {
  // アクティブ状態
  M5.Lcd.setTextColor(WHITE);                 //日付表示文字だけ白色の文字色にする
  M5.Lcd.setCursor(10, 110, 1);                //x,y,font 1:Adafruit 8ピクセルASCIIフォント
  M5.Lcd.printf("active");
  M5.Lcd.drawCircle(60, 113, 6, WHITE);
  if(isActive) M5.Lcd.fillCircle(60, 113, 3, WHITE);
  else M5.Lcd.fillCircle(60, 113, 3, BLACK);
}

void printAlarmTime(bool blind) {
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(10, 40, 7);
  M5.Lcd.setTextColor(GREEN);
  if(blind && mode == 1) {
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.printf("%02d", 88);   
  } else {
    M5.Lcd.printf("%02d", alarmHour);   
  }
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.printf(":");
  if(blind && mode == 2) {
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.printf("%02d", 88);
  } else {
    M5.Lcd.printf("%02d", alarmMinute);
  }
}

void turnLight(bool isOn) {
  M5.Lcd.drawCircle(80, 113, 6, WHITE);
  if(isOn) M5.Lcd.fillCircle(80, 113, 3, WHITE);
  else M5.Lcd.fillCircle(80, 113, 3, BLACK);
}

void setup()
{
  M5.begin();

  M5.Lcd.setRotation(3);
  M5.Axp.ScreenBreath(12);            // 液晶バックライト電圧設定 LCD backlight voltage setting.
  M5.Lcd.fillScreen(BLACK); 

  //connect to WiFi
  int cnt=0;
  M5.Lcd.print("Connecting to ");
  M5.Lcd.print(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    cnt++;
    delay(500);
    M5.Lcd.print(".");
  }
  M5.Lcd.println(" CONNECTED");

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  printIsActive();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

}

void loop()
{
  // 1sec10分割のループ
  for(int i = 0; i < 10; i++) {
    M5.update();
    
    // モード切り替え
    if(M5.BtnA.wasReleased()) {
      mode = (mode + 1) % 3;
    }

    // i=5（中間）で点滅用出力
    if((mode == 1 || mode == 2) && i == 5) {
      printAlarmTime(true);
    }

    // Bボタン押して各種操作（モードによって動作が変わる）
    if(mode == 0 && M5.BtnB.wasReleased()) {
      // タイマーアクティブ変更
      isActive = !isActive;
      printIsActive();
    } else if(M5.BtnB.wasReleased() || M5.BtnB.pressedFor(1000)) {
      if (mode == 1) {
        alarmHour = (alarmHour + 1) % 24;
        printAlarmTime(true);
        printAlarmTime(false); 
        i = 0;
      } else if (mode == 2) {
        alarmMinute = (alarmMinute + 1) % 60;
        printAlarmTime(true);
        printAlarmTime(false);
        i = 0;
      }
    }

    if(isActive && hasReachedTime && i == 5) {
      turnLight(false);
    }

    delay(100);
  }

  // 時間到達確認
  if(isActive && !hasReachedTime) {
    bool res = checkHasReached();
    if(res) hasReachedTime = true;
  } else if(!isActive) {
    hasReachedTime = false;
    turnLight(true);  // 最後タイマー切ったあとデフォルトはON!起きたいので
  }

  if(isActive && hasReachedTime) {
    turnLight(true);
  }

  printLocalTime();
  printAlarmTime(false);
}