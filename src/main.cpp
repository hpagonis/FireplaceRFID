#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>

#define BOARD8

#define RST 15
#define SWT 25
#define ID_LENGTH 7

#ifdef BOARD4
  #define MAX_RC522 4
#else
  #define MAX_RC522 8
#endif

MFRC522 *Mfrc522[MAX_RC522];

struct RC522 {
  uint8_t pin;
  byte uids[2][7];
  bool present;
  bool valid;
};

#ifdef BOARD4
  struct RC522 Rc522[] = {
    {.pin = 17, .uids = {{0x53, 0xB0, 0xFC, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xB0, 0xFC, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin =  5, .uids = {{0x53, 0xB1, 0xFC, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xB1, 0xFC, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin =  4, .uids = {{0x53, 0xB2, 0xFC, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xB2, 0xFC, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin = 16, .uids = {{0x53, 0xB7, 0xFC, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xB7, 0xFC, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false}
  };
#else
  struct RC522 Rc522[] = {
    {.pin = 22, .uids = {{0x53, 0xE8, 0xFB, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xE8, 0xFB, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin = 21, .uids = {{0x53, 0xE7, 0xFB, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xE7, 0xFB, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin = 26, .uids = {{0x53, 0xE2, 0xFB, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xE2, 0xFB, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin = 27, .uids = {{0x53, 0xE1, 0xFB, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xE1, 0xFB, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin = 14, .uids = {{0x53, 0xE0, 0xFB, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xE0, 0xFB, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin =  4, .uids = {{0x53, 0xDF, 0xFB, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xDF, 0xFB, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin = 16, .uids = {{0x53, 0xDA, 0xFB, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xDA, 0xFB, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false},
    {.pin = 13, .uids = {{0x53, 0xD9, 0xFB, 0xCC, 0x60, 0x00, 0x01},{0x53, 0xD9, 0xFB, 0xCC, 0x60, 0x00, 0x01}}, .present = false, .valid = false}
  };
#endif

// static uint8_t LED_pin[4] = {26, 27, 14, 12};

void setup() {
  for (int i=0; i< MAX_RC522; i++) {
    pinMode(Rc522[i].pin, OUTPUT);
    digitalWrite(Rc522[i].pin, HIGH);
  }
  pinMode(RST, OUTPUT);
  pinMode(SWT, OUTPUT);
  digitalWrite(RST, HIGH);
  digitalWrite(SWT, LOW);
  // for (int i=0; i<4; i++) {
  //   pinMode(LED_pin[i], OUTPUT);
  //   digitalWrite(LED_pin[i], LOW);
  // }
  Serial.begin(115200);
  SPI.begin( SCK, MISO, MOSI, -1);
  for (uint32_t index = 0; index < MAX_RC522; index++) {
    Mfrc522[index] = new MFRC522(Rc522[index].pin, (index == 0) ? RST : MFRC522::UNUSED_PIN);
    Mfrc522[index]->PCD_Init();
    Mfrc522[index]->PCD_SetAntennaGain(MFRC522::RxGain_48dB);
    uint8_t v = Mfrc522[index]->PCD_ReadRegister(Mfrc522[index]->VersionReg);
    char ver[8] = { 0 };
    switch (v) {
      case 0x92: strcpy_P(ver, PSTR("v2.0")); break;
      case 0x91: strcpy_P(ver, PSTR("v1.0")); break;
      case 0x88: strcpy_P(ver, PSTR("clone")); break;
      case 0x00: case 0xFF: strcpy_P(ver, PSTR("fail")); break;
    }
    Serial.printf("RC522 Rfid Reader %d detected %s\n", index + 1, ver);
  }
}

void loop() {
  for (uint32_t index = 0; index < MAX_RC522; index++) {
    byte atqa_answer[2];
    byte atqa_size = 2;
    if (Mfrc522[index]->PICC_WakeupA(atqa_answer, &atqa_size) == MFRC522::STATUS_OK) {
    // if (Mfrc522[index]->PICC_IsNewCardPresent()) {
      Rc522[index].present = true;
    }
    if (Rc522[index].present) {
      if (Mfrc522[index]->PICC_ReadCardSerial()) {
        Rc522[index].valid = true;
        // for (int i=0; i<7; i++) {
        //   if (Mfrc522[index]->uid.uidByte[i] != Rc522[index].uids[0][i]
        //     && Mfrc522[index]->uid.uidByte[i] != Rc522[index].uids[1][i]) Rc522[index].valid = false;
        //   // Serial.print(Mfrc522[index]->uid.uidByte[i], HEX);
        // }
        if (memcmp(Mfrc522[index]->uid.uidByte, Rc522[index].uids[0], ID_LENGTH) &&
          memcmp(Mfrc522[index]->uid.uidByte, Rc522[index].uids[1], ID_LENGTH))
            Rc522[index].valid = false;
        // if (Rc522[index].valid) {
        //   Serial.printf("Valid card %d\n", index + 1);
        // }
        // digitalWrite(LED_pin[index], HIGH);
      } else {
        Rc522[index].present = false;
        // digitalWrite(LED_pin[index], LOW);
      }
    } else {
      Rc522[index].valid = false;
    }
    Mfrc522[index]->PICC_HaltA();
    delay(50);
  }
  bool all_valid = true;
  for (int i=0; i< MAX_RC522; i++) {
    all_valid = all_valid && Rc522[i].valid;
  }
  if (all_valid) {
    Serial.println("Relay open");
    digitalWrite(SWT, HIGH);
    delay(2000);
    digitalWrite(SWT, LOW);
  }
}