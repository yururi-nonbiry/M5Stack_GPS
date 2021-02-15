// M5STACK-Cに、GPS(UART接続)を繋いで値を取得する
// 画面サイズ 320*240


// テスト用 テスト時は1を入れる(1は常時読取)
#define test_sw 1

// GPSのピン指定(GLOVE)
//#define GPS_TX 33
//#define GPS_RX 32


// 各種定数
// 緯度経度から距離変換の定数
#define Lat_Coefficient 111263.283 // 緯度(北南)
#define Long_Coefficient 90704.77  // 経度(東西)
#define Geoid_Height 36.7071       //ジオイド高

// インクルードファイル
#include <M5Stack.h>
#include <TinyGPS++.h>
#include "BluetoothSerial.h"
#include <EEPROM.h> // eeprom

static const uint32_t GPSBaud = 9600;

// インスタンス生成
BluetoothSerial SerialBT;
TinyGPSPlus gps;
//HardwareSerial GPSUART(2);
HardwareSerial ss(2);

// EEPROMの構造体宣言
struct set_BT_name {
  char BT_name[21];
};

set_BT_name BT_name_buf ;

// EEPROMの構造体宣言
struct set_position_data { //34
  char title[14]; //15
  short int year_data ; //2
  byte month_data ; //1
  byte day_data ; //1
  byte hour_data ; //1
  byte minute_data ; //1
  byte second_data ; //1
  float lat_data ; //4
  float long_data ; //4
  float alt_data ; //4
};

struct set_position_data set_position_buf[6]; // 構造体宣言

//変数定義
short int read_year = 0;
byte read_month = 0;
byte read_day = 0;
byte read_hour = 0;
byte read_minute = 0;
byte read_second = 0;
float read_lat = 0;
float read_long = 0;
float read_alt = 0;
float read_voltage = 0;
float read_current = 0;
float read_speed = 0;

byte test_count = 0;
byte page_count = 1;


// バッテリー残量表示
void power_supply_draw() {
  if (M5.Power.canControl() == true) {
    byte battery_level = M5.Power.getBatteryLevel();
    M5.Lcd.setTextSize(2);          // 文字のサイズ
    M5.Lcd.setCursor(220, 0);
    M5.Lcd.print(F("BAT:"));
    if (battery_level < 10)   M5.Lcd.print(F(" "));
    if (battery_level < 100)  M5.Lcd.print(F(" "));
    M5.Lcd.print(battery_level);
    M5.Lcd.print(F("%"));
  }
}

// スイッチ表示
void sw_draw() {
  //スイッチの内容表示
  M5.Lcd.setTextSize(2);          // 文字のサイズ
  M5.Lcd.setCursor(60, 220);
  M5.Lcd.print(F("UP"));
  M5.Lcd.setCursor(140, 220);
  M5.Lcd.print(F("DOWN"));
  M5.Lcd.setCursor(240, 220);
  M5.Lcd.print(F("SET"));
}

// EEPROMへ書き込み
void eeprom_write(byte data_no) {
  EEPROM.put<set_BT_name> (0, BT_name_buf);
  EEPROM.put <set_position_data> (data_no * 40 + 30, set_position_buf[data_no]);
  EEPROM.commit();
}

// EEPROMのリセット
void eeprom_reset() {
  //ボタンを離したかの確認
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(2);          // 文字のサイズ
  M5.Lcd.setCursor(90, 30);
  M5.Lcd.print(F("EEPROM Reset"));
  M5.Lcd.setCursor(40, 220);
  M5.Lcd.print(F("reset"));
  M5.Lcd.setCursor(125, 220);
  M5.Lcd.print(F("cansel"));

  while (1) {
    M5.update();
    if (M5.BtnA.wasPressed() == 1)break;
    if (M5.BtnB.wasPressed() == 1)return;
    delay(10);
  }
  //EEPROM reset
  for (byte count = 0; count <= 5; count++) {
    set_position_buf[count].title[0] = '\0'; //11
    set_position_buf[count].year_data = 0; //2
    set_position_buf[count].month_data = 0; //1
    set_position_buf[count].day_data = 0; //1
    set_position_buf[count].hour_data = 0; //1
    set_position_buf[count].minute_data = 0; //1
    set_position_buf[count].second_data = 0; //1
    set_position_buf[count].lat_data = 0; //4
    set_position_buf[count].long_data = 0; //4
    set_position_buf[count].alt_data = 0; //4
    eeprom_write(count);
  }
}

// セッティング用draw
void setting_draw() {

  M5.Lcd.fillScreen(BLACK);  // 画面をクリア
  M5.Lcd.setTextSize(2);          // 文字のサイズ
  M5.Lcd.setCursor(5, 0);
  M5.Lcd.print(F("Set by BT"));
  M5.Lcd.setCursor(5, 20);
  M5.Lcd.print(F("Title :"));
  M5.Lcd.print(set_position_buf[page_count - 1].title);
  M5.Lcd.setCursor(5, 40);
  M5.Lcd.print(F("Date :"));
  M5.Lcd.printf("%4d/%2d/%2d\n", set_position_buf[page_count - 1].year_data,  set_position_buf[page_count - 1].month_data, set_position_buf[page_count - 1].day_data);
  M5.Lcd.setCursor(5, 60);
  M5.Lcd.print(F("Time :"));
  M5.Lcd.printf("%2d:%2d:%2d\n", set_position_buf[page_count - 1].hour_data, set_position_buf[page_count - 1].minute_data, set_position_buf[page_count - 1].second_data);
  M5.Lcd.setCursor(5, 80);
  M5.Lcd.print(F("lat :"));
  M5.Lcd.print(set_position_buf[page_count - 1].lat_data, 12);
  M5.Lcd.setCursor(5, 100);
  M5.Lcd.print(F("long :"));
  M5.Lcd.print(set_position_buf[page_count - 1].long_data, 12);
  M5.Lcd.setCursor(5, 120);
  M5.Lcd.print(F("alt :"));
  M5.Lcd.print(set_position_buf[page_count - 1].alt_data);
}

// セッティング用サブルーチン
void setting() {

  // 変数定義
  String read_serial = "";

  setCpuFrequencyMhz(80); //周波数変更
  delay(100);
  SerialBT.begin("esp_gps"); // bluetoothスタート
  SerialBT.setTimeout(100); // タイムアウトまでの時間を設定

  setting_draw();

  // ここから読取用のルーチン
  while (1) {
    power_supply_draw(); // 電源の状態だけ更新する

    if (SerialBT.available() > 0) { //シリアルに文字が送られてきてるか
      read_serial = SerialBT.readStringUntil(';');
    }
    // スペースと改行を消す
    int search_number = 0; // 文字検索用検索
    while (search_number != -1) { // 消したい文字が見つからなくなるまで繰り返す
      search_number = read_serial.lastIndexOf(" "); // スペースを検索する
      if (search_number = -1)search_number = read_serial.lastIndexOf("\n"); // 改行を検索する
      if (search_number != -1)read_serial.remove(search_number);
    }

    // 受信したときの処置
    if (read_serial != "") { // 文字が入っていたら実行する
      int comma_position = read_serial.indexOf(",");
      if (read_serial.equalsIgnoreCase("help") == true) {
        // ここにヘルプの内容を書き込む
        SerialBT.println(F("help:"));
      } else if (read_serial.equalsIgnoreCase("list") == true) {
        //ここに設定値のリストを書き込む
        SerialBT.println(F("list:"));
      } else if (read_serial.equalsIgnoreCase("save") == true) {
        //ここに設定値のリストを書き込む
        eeprom_write(page_count - 1);
        SerialBT.println(F("save:"));
      } else if (comma_position != -1) { // ","があるか判定する
        String item_str = read_serial.substring(0, comma_position); // 項目読み取り
        String value_str = read_serial.substring(comma_position + 1, read_serial.length()); // 数値読み取り

        // 読み取った結果を表示する
        if (item_str == "title") {
          value_str.toCharArray(set_position_buf[page_count - 1].title, 11);
          SerialBT.print(F("title,")); SerialBT.print(set_position_buf[page_count - 1].title); SerialBT.println(F(";"));
        }
        if (item_str == "year") {
          set_position_buf[page_count - 1].year_data = value_str.toInt();
          SerialBT.print(F("year,")); SerialBT.print(set_position_buf[page_count - 1].year_data); SerialBT.println(F(";"));
        }
        if (item_str == "month") {
          set_position_buf[page_count - 1].month_data = value_str.toInt();
          SerialBT.print(F("month,")); SerialBT.print(set_position_buf[page_count - 1].month_data); SerialBT.println(F(";"));
        }
        if (item_str == "day") {
          set_position_buf[page_count - 1].day_data = value_str.toInt();
          SerialBT.print(F("day,")); SerialBT.print(set_position_buf[page_count - 1].day_data); SerialBT.println(F(";"));
        }
        if (item_str == "hour") {
          set_position_buf[page_count - 1].hour_data = value_str.toInt();
          SerialBT.print(F("hour,")); SerialBT.print(set_position_buf[page_count - 1].hour_data); SerialBT.println(F(";"));
        }
        if (item_str == "minute") {
          set_position_buf[page_count - 1].minute_data = value_str.toInt();
          SerialBT.print(F("minute,")); SerialBT.print(set_position_buf[page_count - 1].minute_data); SerialBT.println(F(";"));
        }
        if (item_str == "second") {
          set_position_buf[page_count - 1].second_data = value_str.toInt();
          SerialBT.print(F("second,")); SerialBT.print(set_position_buf[page_count - 1].second_data); SerialBT.println(F(";"));
        }
        if (item_str == "lat") {
          set_position_buf[page_count - 1].lat_data = value_str.toFloat();
          SerialBT.print(F("lat,")); SerialBT.print(set_position_buf[page_count - 1].lat_data, 12); SerialBT.println(F(";"));
        }
        if (item_str == "long") {
          set_position_buf[page_count - 1].long_data = value_str.toFloat();
          SerialBT.print(F("long,")); SerialBT.print(set_position_buf[page_count - 1].long_data, 12); SerialBT.println(F(";"));
        }
        if (item_str == "alt") {
          set_position_buf[page_count - 1].alt_data = value_str.toFloat();
          SerialBT.print(F("alt,")); SerialBT.print(set_position_buf[page_count - 1].alt_data); SerialBT.println(F(";"));
        }

      }
      setting_draw();
    }
    read_serial = ""; //　クリア
    power_supply_draw();
    M5.update(); // ボタンの状態を更新
    if (M5.BtnC.wasPressed()) { //ボタンを押してたらループから抜ける
      break;
    }
  }
  SerialBT.disconnect(); // bluetooth ストップ
  delay(100);
  setCpuFrequencyMhz(20); //周波数変更
  delay(100);
  M5.Lcd.fillScreen(BLACK); // 画面をクリア
  //menu_lcd_draw(); // メニューのLCD表示

}



//画面出力ルーチン
void displayInfo()
{

  //GPS シリアル定義
  //static const uint32_t GPSBaud = 9600;
  //TinyGPSPlus gps;
  //HardwareSerial GPSUART(2);


  //電圧・電流を表示する
  power_supply_draw();
  sw_draw();
  M5.Lcd.setTextSize(2); // 文字のサイズ

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print(F("Date :"));
  M5.Lcd.printf("%4d/%2d/%2d\n", read_year, read_month, read_day);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.print(F("Time :"));
  M5.Lcd.printf("%2d:%2d:%2d\n", read_hour, read_minute, read_second);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.print(F("Lat:"));
  M5.Lcd.println(read_lat, 7);
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.print(F("Long:"));
  M5.Lcd.println(read_long, 7);
  M5.Lcd.setCursor(0, 80);
  M5.Lcd.print(F("Alt:"));
  M5.Lcd.println(read_alt, 2);
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.print(F("Speed:"));
  M5.Lcd.println(read_speed, 4);
}



//起動時の表示
void boot_display()
{
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print(F("Booting"));
}

//場所比較
void pozition_comp()
{


  //計算
  //距離計算
  float comparison_lat = (read_lat - set_position_buf[page_count - 1].lat_data) * Lat_Coefficient;   // 緯度
  float comparison_long = (read_long - set_position_buf[page_count - 1].long_data) * Long_Coefficient; // 経度
  float comparison_alt = set_position_buf[page_count - 1].alt_data - read_alt;                       // 高度

  float comparison_distance = pow(pow(comparison_lat, 2) + pow(comparison_long, 2), 0.5);
  float comparison_direction = atan2(comparison_long, comparison_lat) * 180 / PI;
  float comparison_angle = atan2(comparison_alt, comparison_distance) * 180 / PI;
  if (comparison_direction < 0)
  {
    comparison_direction = comparison_direction + 360;
  }
  power_supply_draw();
  sw_draw();

  M5.Lcd.setTextSize(2); // 文字のサイズ
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print(F("Page:"));
  M5.Lcd.print(page_count);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.print(F("Titel:"));
  M5.Lcd.print(set_position_buf[page_count - 1].title);
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.print(F("Set Date :"));
  M5.Lcd.printf("%4d/%2d/%2d\n", set_position_buf[page_count - 1].year_data,  set_position_buf[page_count - 1].month_data, set_position_buf[page_count - 1].day_data);
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.print(F("Set Time :"));
  M5.Lcd.printf("%2d:%2d:%2d\n", set_position_buf[page_count - 1].hour_data, set_position_buf[page_count - 1].minute_data, set_position_buf[page_count - 1].second_data);
  M5.Lcd.setCursor(0, 80);
  M5.Lcd.print(F("Dis:"));
  M5.Lcd.println(comparison_distance, 2);
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.print(F("Dir:"));
  M5.Lcd.println(comparison_direction, 2);
  M5.Lcd.setCursor(0, 120);
  M5.Lcd.print(F("Ang:"));
  M5.Lcd.println(comparison_angle, 2);
}

//セットアップ
void setup()
{
  //画面初期化
  M5.begin();
  M5.Power.begin();
  ss.begin(GPSBaud);
  M5.Lcd.setBrightness(200);     // バックライトの明るさ(0-255)
  M5.Lcd.fillScreen(TFT_BLACK);   //LCD背景色
  M5.Lcd.setTextSize(1);          // 文字のサイズ
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK); // 文字の色
  M5.Lcd.setCursor(0, 0);

  EEPROM.begin(1024); //EEPROM開始(サイズ指定)
  EEPROM.get <set_BT_name>(0, BT_name_buf );
  for (int count = 0; count <= 5; count++ ) {
    EEPROM.get <set_position_data>(count * 40 + 30, set_position_buf[count]); // EEPROMを読み込む
  }
  //CPU周波数変更
  setCpuFrequencyMhz(20);

  //GPSのピン指定
  //GPSUART.begin(9600, SERIAL_8N1, GPS_TX, GPS_RX);
  //GPSUART.begin(GPSBaud);
  //GPSUART.begin(9600);
  //SerialBT.begin("ESP32");
}

//メインルーチン
void loop()
{
  //起動確認用の表示
  boot_display();

  //起動時、ボタンCを押しっぱなしだったらEEPROMをリセットする
  M5.update(); //ボタン操作
  if (M5.BtnC.isPressed() == 1)eeprom_reset();
  delay(1000);
  M5.Lcd.fillScreen(TFT_BLACK);

  //ここからループ
  while (1)
  {
    //テストモードとの分岐
    if (test_sw == 0) //通常ルート
    {
      //GPSがスタート出来るかの判定
      while (ss.available())
      {
        if (gps.encode(ss.read()))
        {
          break;
        }
      }
      if (gps.location.isValid())
      {
        //GPS読み取り
        read_year = gps.date.year();
        read_month = gps.date.month();
        read_day = gps.date.day();
        read_hour = 9 + gps.time.hour();
        if (read_hour >= 24) read_hour -= 24;
        read_minute = gps.time.minute();
        read_second = gps.time.second();
        read_lat = gps.location.lat();
        read_long = gps.location.lng();
        read_alt = gps.altitude.meters();
        read_speed = gps.speed.kmph();

        if (page_count == 0)
        {
          displayInfo();
        }
        else
        {
          pozition_comp();
        }
      }
    }
    else //テストモードとの分岐(常時読取)
    {
      //GPS読み取り
      //gps.encode(ss.read());

      while (ss.available() > 0) {
        gps.encode(ss.read());
      }

      read_year = gps.date.year();
      read_month = gps.date.month();
      read_day = gps.date.day();
      read_hour = 9 + gps.time.hour();
      if (read_hour >= 24) read_hour -= 24;
      read_minute = gps.time.minute();
      read_second = gps.time.second();
      read_lat = gps.location.lat();
      read_long = gps.location.lng();
      read_alt = gps.altitude.meters();
      read_speed = gps.speed.kmph();
    }

    M5.update(); //ボタン操作
    if (M5.BtnA.wasPressed() == 1) {
      page_count++;
      M5.Lcd.fillScreen(BLACK);
    }
    if (M5.BtnB.wasPressed() == 1) {
      page_count--;
      M5.Lcd.fillScreen(TFT_BLACK);
    }
    //M5.Lcd.fillScreen(TFT_BLACK);
    if (page_count > 7)page_count = 7;
    if (page_count < 1)page_count = 1;
    if (M5.BtnC.wasPressed() == 1) setting();

    //画面変更ルーチン
    if (page_count == 7) {
      displayInfo();
    }
    else {
      pozition_comp();
    }

    delay(100);
    //test_count += test_sw;
  }
}
