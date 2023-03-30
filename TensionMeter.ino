#include "main.h"

/* 定数(パラメータ) */
const int TIMER_PERIOD_HX711_FAST = 1000;     /* us */
const int TIMER_PERIOD_HX711_SLOW = 200000;   /* us */
const int TIMER_PERIOD_OLED = 1000000;        /* us */

/* =================================================== */

void IRAM_ATTR TimerFire() {
  timer_flag = 1;
  timer_cnt++;
}

/* =================================================== */

void setup() {
  /* OLED */
  u8g2.begin();
  OLED_BootImg();

  /* 通信 */
  Serial.begin(115200);
  SerialBT.begin("TensionMeter");

  /* GPIO */
  pinMode(CLK_PIN, OUTPUT);
  pinMode(DAT_PIN, INPUT);
  pinMode(BATT_PIN, INPUT);
  pinMode(DEBUG_PIN, OUTPUT);

  /* EEPROM */
  EEPROM.begin(16);

  /* タイマー */
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &TimerFire, true);
  timerAlarmWrite(timer, TIMER_PERIOD_HX711_FAST, true);
  timerAlarmEnable(timer);
  timer_cnt_oled_max = TIMER_PERIOD_OLED / TIMER_PERIOD_HX711_FAST;
  timer_cnt_hx711_max = TIMER_PERIOD_HX711_SLOW / TIMER_PERIOD_HX711_FAST;
}

/* =================================================== */

void loop() {
  /* 受信処理 */
  BT_RxProcess();
  Serial_RxProcess();

  /* 測定開始判定 */
  /* 1.読み込み直後 -> HIGH
     2.読み込み後3.2ms -> LOW
     3.読み込み後14.9ms -> HIGH (測定準備完了)
     4.次の測定までの待ち時間
  */
  if (MeasureState == HX711_HIGH) {
    if (gpio_get_level(DAT_PIN) == 0) {
      MeasureState = HX711_LOW;
    }
  }
  if (MeasureState == HX711_LOW) {
    if (gpio_get_level(DAT_PIN) == 1) {
      MeasureState = HX711_FINISH;
    }
  }

  /* 測定 */
  if (MeasureState == HX711_FINISH) {
    HX711_RX();
    BaseReset();
    MeasureState = HX711_HIGH;
    if (speed_mode == FAST) {
      SerialSendFlag = 1;
    }
  }

  /* タイマー処理 */
  if (timer_flag == 1) {
    /* 測定開始判定 */
    if (speed_mode == SLOW) {
      timer_cnt_hx711++;
      if (timer_cnt_hx711 >= timer_cnt_hx711_max) {
        SerialSendFlag = 1;
      }
    }
    if (SerialSendFlag == 1 &&
        BaseResetMode != HX711_BASE_START &&
        BaseResetMode != HX711_BASE_MEASURE &&
        BaseResetMode != HX711_BASE_END) {
      /* 送信 */
      if (data_mode == NORMAL) {
        /* 換算 */
        EEPROM.get(EEPROM_BASE, weight_base);
        EEPROM.get(EEPROM_PROOFREAD, proofread);
        weight = (double)(weight_raw - weight_base) * (double)(proofread);

        SerialBT.print(timer_cnt); /* ms */
        SerialBT.print(",");
        SerialBT.println(weight); /* N */

        Serial.print(timer_cnt); /* ms */
        Serial.print(",");
        Serial.println(weight); /* N */
      }
      if (data_mode == RAW_DATA) {
        SerialBT.println(weight_raw - weight_base);
        Serial.println(weight_raw - weight_base);
      }
      timer_cnt_hx711 = 0;
      SerialSendFlag = 0;
    }

    /* OLEDの更新 */
    if (timer_cnt_oled >= timer_cnt_oled_max) {
      if (speed_mode == SLOW) {
        OLED_Update();
      }
      timer_cnt_oled = 0;
    } else {
      timer_cnt_oled++;
    }

    /* タイマーフラグ初期化 */
    timer_flag = 0;
  }
}
