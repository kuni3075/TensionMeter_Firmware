#include <Wire.h>
#include <EEPROM.h>
#include <BluetoothSerial.h>
#include <U8g2lib.h>

const int CLK_PIN = 33;
const int DAT_PIN = 32;
const int EEPROM_PROOFREAD = 0;

double weight = 0.0;
long weight_raw = 0;
long weight_base = 0;
double weight_max = 0.0;
double proofread = 0;
int timer_flag = 0;
int timer_cnt = 0;
char BT_RxBuff[20] = {0};
int BT_RxCnt = 0;
int BT_RxFlag = 0;

BluetoothSerial SerialBT;
hw_timer_t * timer = NULL;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

void IRAM_ATTR TimerFire() {
  timer_flag = 1;
}

void setup() {
  SerialBT.begin("TensionMeter");
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DAT_PIN, INPUT);
  delayMicroseconds(1000000);
  EEPROM.begin(16);

  /* HX711初期設定 */
  BaseReset();

  /* OLED */
  u8g2.begin();

  /* タイマー */
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &TimerFire, true);
  timerAlarmWrite(timer, 200000, true);
  timerAlarmEnable(timer);
}

void loop() {
  BT_RxProcess();
  
  if (timer_flag == 1) {
    HX711_RX();
    weight_raw -= weight_base;
    EEPROM.get(EEPROM_PROOFREAD, proofread);
    weight = (double)(weight_raw * proofread);
    if (weight > weight_max) {
      weight_max = weight;
    }
    SerialBT.println(weight_raw);
    timer_flag = 0;

    timer_cnt++;
  }

  if (timer_cnt == 5) {
    OLED_Update();
    timer_cnt = 0;
  }
}

void BT_RxProcess() {
  while (SerialBT.available() > 0) {
    BT_RxBuff[BT_RxCnt] = (char)SerialBT.read();
    if (BT_RxBuff[BT_RxCnt] == '\n') {
      BT_RxFlag = 1;
    } else {
      BT_RxCnt++;
    }
  }

  if (BT_RxFlag == 1) {
    if (BT_RxBuff[0] == '0') {
      BaseReset();
    } else if (BT_RxBuff[0] == '1') {
      ProofreadUpdate();
    }
    BT_RxCnt = 0;
    BT_RxFlag = 0;
  }
}

void BaseReset() {
  weight_base = 0;
  for (char j = 0; j < 8; j++) {
    HX711_RX();
    weight_base += weight_raw;
    delayMicroseconds(300000);
  }
  weight_base = weight_base >> 3;
  SerialBT.println("finished");
}

void HX711_RX() {
  weight_raw = 0;
  for (char i = 0; i < 24; i++) {
    digitalWrite(CLK_PIN, 1);
    delayMicroseconds(1);
    digitalWrite(CLK_PIN, 0);
    delayMicroseconds(1);
    weight_raw = (weight_raw << 1) | (digitalRead(DAT_PIN));
  }
  weight_raw = weight_raw ^ 0x800000;
}

void OLED_Update() {
  char str[10] = {0};
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_calibration_gothic_nbp_tf);

  dtostrf(weight, 4, 2, str);
  u8g2.drawStr(20, 20, str);
  u8g2.drawStr(100, 20, "N");

  dtostrf(weight_max, 4, 2, str);
  u8g2.drawStr(20, 40, str);
  u8g2.drawStr(100, 40, "N");

  u8g2.sendBuffer();
}

void ProofreadUpdate() {
  double num, num1;
  String str;
  char str1[20] = {0};

  for (char i = 2; i < BT_RxCnt; i++) {
    str.concat((String)(BT_RxBuff[i]));
  }
  num = (double)(str.toFloat());

  EEPROM.put(EEPROM_PROOFREAD, num);
  EEPROM.commit();

  SerialBT.print("Proofread Update:");
  dtostrf(num, 9, 8, str1);
  SerialBT.println(str1);
}
