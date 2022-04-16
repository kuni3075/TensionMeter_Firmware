void HX711_RX() {
  weight_raw = 0;
  for (char i = 0; i < 24; i++) {
    gpio_set_level(CLK_PIN, 1);
    delayMicroseconds(1);
    gpio_set_level(CLK_PIN, 0);
    delayMicroseconds(1);
    weight_raw = (weight_raw << 1) | (gpio_get_level(DAT_PIN));
  }
  weight_raw = weight_raw ^ 0x800000;
}

/* =================================================== */

void BaseReset() {
  SerialBT.println("Base Update");
  Serial.println("Base Update");
  
  weight_base = 0;
  for (char j = 0; j < 8; j++) {
    HX711_RX();
    weight_base += weight_raw;
    delayMicroseconds(300000);
  }
  weight_base = weight_base >> 3;
  
  EEPROM.put(EEPROM_BASE, weight_base);
  EEPROM.commit();
  SerialBT.println("Finished");
  Serial.println("Finished");
}

/* =================================================== */

void ProofreadUpdate() {
  double num, num1;
  String str;
  char str1[20] = {0};

  for (char i = 2; i < RxCnt; i++) {
    str.concat((String)(RxBuff[i]));
  }
  num = (double)(str.toFloat());

  EEPROM.put(EEPROM_PROOFREAD, num);
  EEPROM.commit();

  SerialBT.print("Proofread Update:");
  Serial.print("Proofread Update:");
  dtostrf(num, 9, 8, str1);
  SerialBT.println(str1);
  Serial.println(str1);
}
