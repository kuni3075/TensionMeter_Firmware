#include "main.h"

/* 定数(パラメータ) */
const int TIMER_PERIOD_HX711_FAST = 200000;     /* us */
const int TIMER_PERIOD_HX711_SLOW = 200000;   /* us */
const int TIMER_PERIOD_OLED = 1000000;        /* us */

/* =================================================== */

void IRAM_ATTR TimerFire() {
  timer_flag = 1;
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
}

/* =================================================== */

void loop() {
  /* 受信処理 */
  BT_RxProcess();
  Serial_RxProcess();

  /* Fastモード設定 */
  if (FastModeFlag == 1) {
    timer_cnt_hx711_max = 0;
  } else {
    timer_cnt_hx711_max = (TIMER_PERIOD_HX711_SLOW / TIMER_PERIOD_HX711_FAST) - 1;
  }

  /* タイマー処理 */
  if (timer_flag == 1) {
    /* A/Dコンバータ */
    if (timer_cnt_hx711 >= timer_cnt_hx711_max) {
      HX711_RX();
      EEPROM.get(EEPROM_BASE, weight_base);
      weight_raw -= weight_base;
      EEPROM.get(EEPROM_PROOFREAD, proofread);
      weight = (double)(weight_raw * proofread);

      if (weight <= 50000 && weight >= -50000) {
        if (weight > weight_max) {
          weight_max = weight;
        }
        if (data_mode == NORMAL) {
          SerialBT.println(weight);
          Serial.println(weight);
        }
        if (data_mode == RAW_DATA) {
          SerialBT.println(weight_raw);
          Serial.println(weight_raw);
        }
      } else {
        SerialBT.println("error");
        Serial.println("error");
      }
      timer_cnt_hx711 = 0;
    } else {
      timer_cnt_hx711++;
    }

    /* OLEDの更新 */
    if (timer_cnt_oled >= timer_cnt_oled_max) {
      if (FastModeFlag == 0) {
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
