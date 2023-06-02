#include <SPI.h>

#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode card_status;

enum RESPONSES {
  SUCCESS,
  ERROR
};

const int SUCCESS_PIN = 7;
const int ERROR_PIN = 8;
const int BUZZER_PIN = 6;

const byte MONEY_BLOCK = 8;
const byte POINT_BLOCK = 9;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(SUCCESS_PIN, OUTPUT);
  pinMode(ERROR_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}
void loop() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Get Money and Block Data
  String money = readBytesFromBlock(MONEY_BLOCK);
  String points = readBytesFromBlock(POINT_BLOCK);

  if (!is_number(money))
    money = "0";
  if (!is_number(points))
    points = "0";

  Serial.print(F("[card_data]"));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.print(";");
  Serial.print(money);
  Serial.print(";");
  Serial.print(points);
  Serial.println(";");

  byte buffr[16];
  byte block = 4;
  byte len;

  while (!Serial.available())
    ;

  String res = Serial.readString();
  res.trim();
  len = res.length();
  res = res.substring(0, len - 1);
  len = res.length();
  res.getBytes(buffr, 16);

  if (len >= 1) {
    // Serial.print("GOT[");
    // Serial.print(len);
    // Serial.print("]: |");
    // Serial.print(res);
    // Serial.println("|");

    if (is_number(res)) {
      if (res.toInt() == ERROR) {
        Serial.println("FAILED");
        access_denied();
      } else if (res.toInt() == SUCCESS) {
        access_granted();
      }
    } else {
      String strs[2] = { "0", "0" };
      byte StringCount = 0;
      while (res.length() > 0 && StringCount < 2) {
        int index = res.indexOf(';');
        if (index == -1)  // no ; found
        {
          strs[StringCount++] = res;
          break;
        } else {
          strs[StringCount++] = res.substring(0, index);
          res = res.substring(index + 1);
        }
      }

      for (int i = 0; i < StringCount; i++) {
        Serial.print(i);
        Serial.print(": \"");
        if (!is_number(strs[i]))
          strs[i] = "0";
        Serial.print(strs[i]);
        Serial.println("\"");
      }

      // write the new data to the card
      writeBytesToBlock(MONEY_BLOCK, strs[0]);
      writeBytesToBlock(POINT_BLOCK, strs[1]);
      Serial.println("DONE\n=======================");
      access_granted();
    }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
void printHex(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void access_granted() {
  digitalWrite(SUCCESS_PIN, HIGH);
  digitalWrite(ERROR_PIN, LOW);
  buzz("twice");
  delay(1820);
  digitalWrite(SUCCESS_PIN, LOW);
}

void access_denied() {
  digitalWrite(ERROR_PIN, HIGH);
  digitalWrite(SUCCESS_PIN, LOW);
  buzz("long");
  digitalWrite(ERROR_PIN, LOW);
}

void buzz(String type) {
  if (type.equals("once")) {
    analogWrite(BUZZER_PIN, 60);
    delay(60);
    analogWrite(BUZZER_PIN, 0);
  } else if (type.equals("twice")) {
    analogWrite(BUZZER_PIN, 60);
    delay(60);
    analogWrite(BUZZER_PIN, 0);
    delay(60);
    analogWrite(BUZZER_PIN, 60);
    delay(60);
    analogWrite(BUZZER_PIN, 0);
  } else if (type.equals("long")) {
    analogWrite(BUZZER_PIN, 60);
    delay(2000);
    analogWrite(BUZZER_PIN, 0);
  } else {
    analogWrite(BUZZER_PIN, 60);
    delay(60);
    analogWrite(BUZZER_PIN, 0);
  }
}

String readBytesFromBlock(byte blockNumber) {
  card_status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNumber, &key, &(rfid.uid));
  if (card_status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(rfid.GetStatusCodeName(card_status));
    return "";
  }
  byte arrayAddress[18];
  byte buffersize = sizeof(arrayAddress);
  card_status = rfid.MIFARE_Read(blockNumber, arrayAddress, &buffersize);
  if (card_status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(rfid.GetStatusCodeName(card_status));
    return;
  }

  String value = "";
  for (uint8_t i = 0; i < 16; i++) {
    value += (char)arrayAddress[i];
  }
  value.trim();
  return value;
}

void writeBytesToBlock(byte block, String content) {
  byte buff[16];
  content.getBytes(buff, 16);
  if (content.length() > 0) {
    for (byte i = content.length(); i < 16; i++) {
      buff[i] = ' ';  //We have to pad array items with spaces.
    }
  }
  card_status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid));

  if (card_status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(rfid.GetStatusCodeName(card_status));
    return;
  }
  // Write block
  card_status = rfid.MIFARE_Write(block, buff, 16);

  if (card_status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(rfid.GetStatusCodeName(card_status));
    return;
  }
}

boolean is_number(String str) {
  for (byte i = 0; i < str.length(); i++) {
    if (!isDigit(str.charAt(i))) return false;
  }
  return true;
}