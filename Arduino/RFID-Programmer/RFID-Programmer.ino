/*
   --------------------------------------------------------
                Jeti RFID-Programmer
   --------------------------------------------------------

    Tero Salminen RC-Thoughts.com 2017 www.rc-thoughts.com

    Easy way to program 13.56Mhz RFID-Tags for Jeti RFID-Sensor

    Includes a 0.96" OLED-screen, only USB-power needed.   

   --------------------------------------------------------
    ALWAYS test functions thoroughly before use!
   --------------------------------------------------------
    Shared under MIT-license by Tero Salminen 2017
   --------------------------------------------------------
*/

#include <SPI.h>
#include <Wire.h>
#include <OneButton.h>
#include <MFRC522.h>
#include <stdlib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// RFID stuff
#define prog_char char PROGMEM
#define SS_PIN 10
#define RST_PIN 5
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

#define block 60
#define block2 61
byte blockcontent[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
byte readbackblock[18];
boolean bReadCard = false;
boolean tagValues = false;
boolean rct = false;

// OLED stuff
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

// control stuff
OneButton actionButton(A1, true); //between A1 and ND
OneButton valueButton(A2, true); //between A2 and ND
int multi = 1;
int screen = 0;

// Values
unsigned int uBatteryID;
unsigned int uCapacity;
unsigned int uCycles;
unsigned int uCells;
unsigned int uCcount;

static const unsigned char PROGMEM RCThoughts[] = {
  0x00, 0x1F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x3F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xFF, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x07, 0xFF, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0F, 0xFF, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0E, 0x03, 0xC0, 0xF0, 0x07, 0xFB, 0x0C, 0x7C, 0x61, 0x8F, 0xC6, 0x1B, 0xFC, 0xFC, 0x07, 0xF0,
  0x1E, 0x01, 0xC0, 0x78, 0x07, 0xFB, 0x0C, 0xFE, 0x61, 0x9F, 0xE6, 0x1B, 0xFD, 0xFC, 0x04, 0x10,
  0x1E, 0x31, 0x8E, 0x78, 0x00, 0xC3, 0x0C, 0xC6, 0x61, 0x98, 0x66, 0x18, 0x61, 0x8E, 0x04, 0x10,
  0x3E, 0x39, 0x8E, 0x78, 0x00, 0xC3, 0x0C, 0xC6, 0x61, 0x98, 0x66, 0x18, 0x61, 0x86, 0x06, 0x30,
  0x3E, 0x39, 0x8E, 0x7C, 0x00, 0xC3, 0x0C, 0xC6, 0x61, 0x98, 0x66, 0x18, 0x61, 0xC0, 0x00, 0x00,
  0x3E, 0x31, 0x8F, 0xFC, 0x00, 0xC3, 0x0C, 0xC6, 0x61, 0x98, 0x06, 0x18, 0x60, 0xE0, 0x07, 0xF0,
  0x3E, 0x03, 0x8F, 0xFC, 0xF8, 0xC3, 0xFC, 0xC6, 0x61, 0x98, 0x07, 0xF8, 0x60, 0x70, 0x04, 0x10,
  0x3E, 0x07, 0x8F, 0xFC, 0xF8, 0xC3, 0xFC, 0xC6, 0x61, 0x99, 0xE7, 0xF8, 0x60, 0x38, 0x04, 0x10,
  0x3E, 0x23, 0x8F, 0xFC, 0x00, 0xC3, 0x0C, 0xC6, 0x61, 0x99, 0xE6, 0x18, 0x60, 0x1C, 0x07, 0xE0,
  0x3E, 0x23, 0x8E, 0x7C, 0x00, 0xC3, 0x0C, 0xC6, 0x61, 0x98, 0x66, 0x18, 0x60, 0x0E, 0x00, 0x00,
  0x3E, 0x33, 0x8E, 0x78, 0x00, 0xC3, 0x0C, 0xC6, 0x61, 0x98, 0x66, 0x18, 0x61, 0x86, 0x07, 0xF0,
  0x3E, 0x31, 0x8E, 0x78, 0x00, 0xC3, 0x0C, 0xC6, 0x61, 0x98, 0x66, 0x18, 0x61, 0xC6, 0x01, 0xC0,
  0x1E, 0x31, 0x8E, 0x78, 0x00, 0xC3, 0x0C, 0xC6, 0x73, 0x98, 0xE6, 0x18, 0x61, 0xC6, 0x07, 0x00,
  0x1E, 0x38, 0xC0, 0x70, 0x00, 0xC3, 0x0C, 0xFE, 0x3F, 0x1F, 0xE6, 0x18, 0x60, 0xFE, 0x61, 0xC0,
  0x0E, 0x38, 0xE0, 0xF0, 0x00, 0xC3, 0x0C, 0x7C, 0x3F, 0x0F, 0x66, 0x18, 0x60, 0x7C, 0x67, 0xF0,
  0x0F, 0xFF, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x07, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x03, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x7F, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x1F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void setup()   {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)

  // Show RC-Thoughts logo in boot
  showScreenLogo();

  // RFID
  SPI.begin();
  mfrc522.PCD_Init();
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  uBatteryID = 0;
  uCapacity = 0;
  uCycles = 0;
  uCells = 0;
  uCcount = 0;

  // Action button
  actionButton.attachClick(clickAction);
  actionButton.attachLongPressStart(longPressStartAction);

  // Value button
  valueButton.attachClick(clickValue);
  valueButton.attachLongPressStart(longPressStartValue);
}

void loop() {

  // RFID Check for tags
  if (! bReadCard) {
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }

    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    // Check if we are using RC-Thoughts tag
    byte piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    if (piccType == MFRC522::PICC_TYPE_MIFARE_1K) {
      rct = true;
    }
  }

  // Monitor buttons if tag is present:
  if (rct) {
    actionButton.tick();
    valueButton.tick();
  }

  if ( bReadCard && rct ) {
    mfrc522.PCD_StopCrypto1();
    mfrc522.PICC_IsNewCardPresent();
    if (! mfrc522.PICC_IsNewCardPresent()) {
      rct = false;
      tagValues = false;
      bReadCard = false;
      uBatteryID = 0;
      uCapacity = 0;
      uCycles = 0;
      uCells = 0;
      uCcount = 0;
      showScreenLogo();
      screen = 0;
      multi = 1;
    }
  }

  // RC-Thoughts Tag Process START
  if (! bReadCard && rct) {
    readBlock(block, readbackblock);
    uBatteryID = ((readbackblock[0] & 0xff) << 8) | readbackblock[1];
    uCapacity = ((readbackblock[2] & 0xff) << 8) | readbackblock[3];
    uCycles = ((readbackblock[4] & 0xff) << 8) | readbackblock[5];
    uCells = ((readbackblock[6] & 0xff) << 8) | readbackblock[7];
    readBlock(block2, readbackblock);
    uCcount = ((readbackblock[0] & 0xff) << 8) | readbackblock[1];
    Serial.println(F("RC-Thoughts Info"));
    Serial.print(F("ID: ")); Serial.println(uBatteryID);
    Serial.print(F("Capacity: ")); Serial.println(uCapacity);
    Serial.print(F("Cycles: ")); Serial.println(uCycles);
    Serial.print(F("Cells: ")); Serial.println(uCells);
    Serial.print(F("C-Value: ")); Serial.println(uCcount);
    Serial.println("");
    bReadCard = true;
    showScreenData();
    screen = 0;

    if (! tagValues)
    {
      tagValues = true;
    }
  } // RC-Thoughts Tag Process END
} // Loop END

void clickAction() {
  if (multi == 1) {
    multi = 10;
    updateScreen();
  } else if (multi == 10) {
    multi = 100;
    updateScreen();
  } else if (multi == 100) {
    multi = 1000;
    updateScreen();
  } else if (multi == 1000) {
    multi = 1;
    updateScreen();
  }
}

void longPressStartAction() {
  // if long press go to next settings screen
  screen++;
  if (screen == 6) {
    writeCard();
  } else {
    updateScreen();
  }
}

void clickValue() {
  if (screen == 1) {
    uBatteryID = uBatteryID + (1 * multi);
    if (uBatteryID > 9999) {
      uBatteryID = 9999;
    }
    showScreenID();
  }
  if (screen == 2) {
    uCells = uCells + (1 * multi);
    if (uCells > 99) {
      uCells = 99;
    }
    showScreenCells();
  }
  if (screen == 3) {
    uCapacity = uCapacity + (1 * multi);
    if (uCapacity > 65534) {
      uCapacity = 65534;
    }
    showScreenCapacity();
  }
  if (screen == 4) {
    uCycles = uCycles + (1 * multi);
    if (uCycles > 999) {
      uCycles = 999;
    }
    showScreenCycles();
  }
  if (screen == 5) {
    uCcount = uCcount + (1 * multi);
    if (uCcount > 999) {
      uCcount = 999;
    }
    showScreenCValue();
  }
}

void longPressStartValue() {
  if (screen == 1) {
    uBatteryID = 0;
    showScreenID();
  }
  if (screen == 2) {
    uCells = 0;
    showScreenCells();
  }
  if (screen == 3) {
    uCapacity = 0;
    showScreenCapacity();
  }
  if (screen == 4) {
    uCycles = 0;
    showScreenCycles();
  }
  if (screen == 5) {
    uCcount = 0;
    showScreenCValue();
  }
}

void updateScreen() {
  if (screen == 0) {
    showScreenData();
  }
  if (screen == 1) {
    showScreenID();
  }
  if (screen == 2) {
    showScreenCells();
  }
  if (screen == 3) {
    showScreenCapacity();
  }
  if (screen == 4) {
    showScreenCycles();
  }
  if (screen == 5) {
    showScreenCValue();
  }
}

void showScreenLogo() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(18, 44);
  display.print(F("RFID-Programmer"));
  display.drawBitmap(0, 8, RCThoughts, 128, 28, 1);
  display.display();
}

// Screen 0
void showScreenData() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.print(F("  RFID Data Display"));
  display.setTextSize(1);
  display.setCursor(3, 14); display.print(F("Battery ID")); display.setCursor(75, 14); display.print(uBatteryID);
  display.setCursor(3, 23); display.print(F("Cells")); display.setCursor(75, 23); display.print(uCells); display.print("S");
  display.setCursor(3, 32); display.print(F("Capacity")); display.setCursor(75, 32); display.print(uCapacity); display.print("mAh");
  display.setCursor(3, 41); display.print(F("Cycles")); display.setCursor(75, 41); display.print(uCycles);
  display.setCursor(3, 50); display.print(F("C-Value")); display.setCursor(75, 50); display.print(uCcount); display.print("C");
  display.display();
  screen = 0;
}

// Screen 1
void showScreenID() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.print(F("  Battery ID number"));
  display.setTextSize(3);
  if (uBatteryID < 10) {
    display.setCursor(55, 20);
  } else {
    if (uBatteryID > 9 && uBatteryID <= 99) {
      display.setCursor(46, 20);
    } else {
      if (uBatteryID > 99 && uBatteryID <= 999) {
        display.setCursor(35, 20);
      } else {
        if (uBatteryID > 999) {
          display.setCursor(26, 20);
        }
      }
    }
  }
  display.print(uBatteryID);
  display.setTextSize(1);
  display.setCursor(0, 56);
  if (multi == 1) {
    display.print(F("x1"));
  }
  if (multi == 10) {
    display.print(F("x10"));
  }
  if (multi == 100) {
    display.print(F("x100"));
  }
  if (multi == 1000) {
    display.print(F("x1000"));
  }
  display.setCursor(59, 56);
  display.print(F("RC-Thoughts"));
  display.display();
}

// Screen 2
void showScreenCells() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.print(F(" Battery Cell Count"));
  display.setTextSize(3);
  if (uCells < 10) {
    display.setCursor(46, 20);
  } else {
    if (uCells > 9 && uCells <= 99) {
      display.setCursor(35, 20);
    }
  }
  display.print(uCells);
  display.print(F("S"));
  display.setTextSize(1);
  display.setCursor(0, 56);
  if (multi == 1) {
    display.print(F("x1"));
  }
  if (multi == 10) {
    display.print(F("x10"));
  }
  if (multi == 100) {
    display.print(F("x100"));
  }
  if (multi == 1000) {
    display.print(F("x1000"));
  }
  display.setCursor(59, 56);
  display.print(F("RC-Thoughts"));
  display.display();
}

// Screen 3
void showScreenCapacity() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.print(F("  Battery Capacity"));
  display.setTextSize(3);
  if (uCapacity < 10) {
    display.setCursor(55, 20);
    display.print(uCapacity);
    display.setTextSize(1);
    display.setCursor(73, 34);
    display.print(F("mAh"));
  } else {
    if (uCapacity > 9 && uCapacity <= 99) {
      display.setCursor(46, 20);
      display.print(uCapacity);
      display.setTextSize(1);
      display.setCursor(82, 34);
      display.print(F("mAh"));
    } else {
      if (uCapacity > 99 && uCapacity <= 999) {
        display.setCursor(37, 20);
        display.print(uCapacity);
        display.setTextSize(1);
        display.setCursor(91, 34);
        display.print(F("mAh"));
      } else {
        if (uCapacity > 999 && uCapacity <= 9999) {
          display.setCursor(28, 20);
          display.print(uCapacity);
          display.setTextSize(1);
          display.setCursor(100, 34);
          display.print(F("mAh"));
        } else {
          if (uCapacity > 9999) {
            display.setCursor(14, 20);
            display.print(uCapacity);
            display.setTextSize(1);
            display.setCursor(105, 34);
            display.print(F("mAh"));
          }
        }
      }
    }
  }
  display.setTextSize(1);
  display.setCursor(0, 56);
  if (multi == 1) {
    display.print(F("x1"));
  }
  if (multi == 10) {
    display.print(F("x10"));
  }
  if (multi == 100) {
    display.print(F("x100"));
  }
  if (multi == 1000) {
    display.print(F("x1000"));
  }
  display.setCursor(59, 56);
  display.print(F("RC-Thoughts"));
  display.display();
}

// Screen 4
void showScreenCycles() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.print(F("   Battery Cycles"));
  display.setTextSize(3);
  if (uCycles < 10) {
    display.setCursor(55, 20);
  } else {
    if (uCycles > 9 && uCycles <= 99) {
      display.setCursor(46, 20);
    } else {
      if (uCycles > 99 && uCycles <= 999) {
        display.setCursor(35, 20);
      } else {
        if (uCycles > 999) {
          display.setCursor(26, 20);
        }
      }
    }
  }
  display.print(uCycles);
  display.setTextSize(1);
  display.setCursor(0, 56);
  if (multi == 1) {
    display.print(F("x1"));
  }
  if (multi == 10) {
    display.print(F("x10"));
  }
  if (multi == 100) {
    display.print(F("x100"));
  }
  if (multi == 1000) {
    display.print(F("x1000"));
  }
  display.setCursor(59, 56);
  display.print(F("RC-Thoughts"));
  display.display();
}

// Screen 5
void showScreenCValue() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.print(F("   Battery C-Value"));
  display.setTextSize(3);
  if (uCcount < 10) {
    display.setCursor(46, 20);
  } else {
    if (uCcount > 9 && uCcount <= 99) {
      display.setCursor(35, 20);
    } else {
      if (uCcount > 99) {
        display.setCursor(26, 20);
      }
    }
  }
  display.print(uCcount);
  display.print(F("C"));
  display.setTextSize(1);
  display.setCursor(0, 56);
  if (multi == 1) {
    display.print(F("x1"));
  }
  if (multi == 10) {
    display.print(F("x10"));
  }
  if (multi == 100) {
    display.print(F("x100"));
  }
  if (multi == 1000) {
    display.print(F("x1000"));
  }
  display.setCursor(59, 56);
  display.print(F("RC-Thoughts"));
  display.display();
}

// Screen 6
void showScreenWritten() {
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(25, 22);
  display.print(F("DONE!"));
  display.display();
  delay(750);
  screen = 0;
  showScreenData();
}

void showScreenWriting() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 27);
  display.print(F("Writing!"));
  display.display();
}

void showScreenError() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(26, 27);
  display.print(F("ERROR!"));
  display.display();
  delay(2000);
  screen = 0;
  showScreenLogo();
}

void writeCard() {
  if (rct) {
  showScreenWriting();
  // write tag
  mfrc522.PICC_ReadCardSerial();
  unsigned char high = (byte)(uBatteryID >> 8);
  unsigned char low  = (byte)uBatteryID;
  blockcontent[0] = high; blockcontent[1] = low;
  high = (byte)(uCapacity >> 8);
  low  = (byte)uCapacity ;
  blockcontent[2] = high; blockcontent[3] = low;
  high = (byte)(uCycles >> 8);
  low  = (byte)uCycles;
  blockcontent[4] = high; blockcontent[5] = low;
  high = (byte)(uCells >> 8);
  low  = (byte)uCells;
  blockcontent[6] = high; blockcontent[7] = low;
  writeBlock(block, blockcontent);
  high = (byte)(uCcount >> 8);
  low  = (byte)uCcount;
  blockcontent[0] = high; blockcontent[1] = low;
  writeBlock(block2, blockcontent);
  tagValues = false;
  delay(500);
  readBlock(block, readbackblock);
  uBatteryID = ((readbackblock[0] & 0xff) << 8) | readbackblock[1];
  uCapacity = ((readbackblock[2] & 0xff) << 8) | readbackblock[3];
  uCycles = ((readbackblock[4] & 0xff) << 8) | readbackblock[5];
  uCells = ((readbackblock[6] & 0xff) << 8) | readbackblock[7];
  readBlock(block2, readbackblock);
  uCcount = ((readbackblock[0] & 0xff) << 8) | readbackblock[1];
  bReadCard = true;
  if (! tagValues)
  {
    tagValues = true;
  }
  showScreenWritten();
  } else {
    showScreenError();
  }
}
