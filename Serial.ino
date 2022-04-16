void Serial_RxProcess() {
  while (Serial.available() > 0) {
    RxBuff[RxCnt] = (char)Serial.read();
    if (RxBuff[RxCnt] == '\n')RxFlag = 1;
    else RxCnt++;
  }
  if (RxFlag == 1)FunctionRun();
}

void BT_RxProcess() {
  while (SerialBT.available() > 0) {
    RxBuff[RxCnt] = (char)SerialBT.read();
    if (RxBuff[RxCnt] == '\n')RxFlag = 1;
    else RxCnt++;
  }
  if (RxFlag == 1)FunctionRun();
}

void FunctionRun() {
  if (RxBuff[0] == '0')BaseReset();
  else if (RxBuff[0] == '1')ProofreadUpdate();
  else if (RxBuff[0] == '2')ModeChange();
  RxCnt = 0;
  RxFlag = 0;
}

void ModeChange() {
  if (RxBuff[2] == '0') {   /* Normalモード(OLEDで測定) */
    run_mode = 0;
    data_mode = NORMAL;
    FastModeFlag = 0;
    
    SerialBT.println("Normal Mode");
    Serial.println("Normal Mode");
  } else if (RxBuff[2] == '1') {  /* Fastモード(衝撃試験用) */
    run_mode = 1;
    data_mode = NORMAL;
    FastModeFlag = 1;
    
    SerialBT.println("Fast Mode");
    Serial.println("Fast Mode");

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_11_tf);
    u8g2.drawStr(0, 8, "MODE:FAST");
    u8g2.sendBuffer();
  } else if (RxBuff[2] == '2') {  /* キャリブレーション用 */
    run_mode = 2;
    data_mode = RAW_DATA;
    FastModeFlag = 0;
    
    SerialBT.println("Calibration Mode");
    Serial.println("Calibration Mode");
  } else {
    data_mode = NORMAL;
    FastModeFlag = 0;
  }
}
