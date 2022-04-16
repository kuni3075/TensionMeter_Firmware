void OLED_Update() {
  char str[20] = {0};
  byte x = 0;
  u8g2.clearBuffer();

  /* RUNモード */
  u8g2.setFont(u8g2_font_t0_11_tf);
  strcat(str, "MODE:");
  if(run_mode == 0)strcat(str, "NORMAL");
  else if(run_mode == 1)strcat(str, "FAST");
  else if(run_mode == 2)strcat(str, "CALI.");
  u8g2.drawStr(0, 8, str);

  /* バッテリー残量 */
  float batt = (float)(analogRead(BATT_PIN)) * 2.0 * 3.3 / 4096.0;
  batt = (batt - 3.0) * 100.0 / 1.2;
  if (batt > 100) {
    batt = 100;
  }
  dtostrf(batt, 3, 0, str);
  strcat(str, "%");
  x = u8g2.getStrWidth(str);
  u8g2.drawStr(128 - x, 8, str);

  /* 現在の張力 */
  u8g2.setFont(u8g2_font_fur17_tf );
  if (weight < 1000) {
    dtostrf(weight, 5, 0, str);
    strcat(str, "  N");
  } else {
    dtostrf(weight / 1000.0, 5, 3, str);
    strcat(str, " kN");
  }

  x = u8g2.getStrWidth(str);
  u8g2.drawStr(128 - x, 35, str);

  /* 最大張力 */
  if (weight_max < 1000) {
    dtostrf(weight_max, 5, 0, str);
    strcat(str, "  N");
  } else {
    dtostrf(weight_max / 1000.0, 5, 3, str);
    strcat(str, " kN");
  }
  x = u8g2.getStrWidth(str);
  u8g2.drawStr(128 - x, 63, str);

  /* 画面更新 */
  u8g2.sendBuffer();
}

void OLED_BootImg() {
  u8g2.clearBuffer();
  u8g2.setBitmapMode(false /* solid */);
  u8g2.setDrawColor(1);
  u8g2.drawXBM(0, 0, 128, 64, bmpdata);
  u8g2.sendBuffer();
}
