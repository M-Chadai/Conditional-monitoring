#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Keypad.h>

// === LCD Setup ===
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address, 16 columns, 2 rows

// === RFID Setup ===
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);
byte knownUID[] = {0x34, 0x69, 0x14, 0x4C};  // Valid card UID

// === IR Sensor ===
#define IR_SENSOR_PIN 2  // Active LOW when object detected

// === Servo Setup ===
Servo myServo;
#define SERVO_PIN 8

// === Keypad Setup ===
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {3, 4, 5, 6};        // Connect to R1-R4
byte colPins[COLS] = {A0, A1, A2, A3};    // Connect to C1-C4
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// === PIN Code Setup ===
String correctPIN = "12345";
String enteredPIN = "";

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  lcd.init();
  lcd.backlight();

  pinMode(IR_SENSOR_PIN, INPUT);
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Start at closed position

  lcd.setCursor(0, 0);
  lcd.print("System Ready...");
  Serial.println("System initialized and ready.");
}

void loop() {
  if (digitalRead(IR_SENSOR_PIN) == LOW) {  // Object detected
    Serial.println("IR sensor detected object.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan your card");
    Serial.println("Waiting for RFID card...");

    bool cardDetected = false;
    bool cardValid = false;

    unsigned long startTime = millis();
    while (millis() - startTime < 5000) {
      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        Serial.print("Card UID: ");
        for (byte i = 0; i < rfid.uid.size; i++) {
          Serial.print(rfid.uid.uidByte[i], HEX);
          Serial.print(" ");
        }
        Serial.println();

        cardDetected = true;
        cardValid = isKnownUID();
        if (cardValid) {
          Serial.println("Card is valid.");
        } else {
          Serial.println("Card is invalid.");
        }

        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
        break;
      }
    }

    if (cardValid) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Access Granted");
      Serial.println("Access granted by RFID.");
      myServo.write(120);  // Open
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Welcome");
      delay(3000);
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter PIN:");
      Serial.println("Prompting for PIN...");

      enteredPIN = "";
      while (enteredPIN.length() < 5) {
        char key = keypad.getKey();
        if (key) {
          enteredPIN += key;
          lcd.setCursor(enteredPIN.length() - 1, 1);
          lcd.print("*");
          Serial.print("*");
        }
      }

      Serial.println();
      Serial.print("Entered PIN: ");
      Serial.println(enteredPIN);

      if (enteredPIN == correctPIN) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Granted");
        Serial.println("Access granted by PIN.");
        myServo.write(120);  // Open
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Welcome");
        delay(3000);
      } else {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Denied");
        Serial.println("Access denied.");
        delay(2000);
      }
    }

    // === Wait for object to leave ===
    Serial.println("Waiting for object to leave...");
    while (digitalRead(IR_SENSOR_PIN) == LOW) {
      delay(100);
    }

    // === After object leaves, wait 5s then close servo ===
    Serial.println("Object left. Waiting 5 seconds before closing servo.");
    delay(5000);
    myServo.write(0);  // Close
    Serial.println("Servo returned to 0°.");

    // === Reset message ===
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Ready...");
    Serial.println("Ready for next user.");
  }
}

// === Check if scanned UID is known ===
bool isKnownUID() {
  if (rfid.uid.size != sizeof(knownUID)) return false;
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] != knownUID[i]) return false;
  }
  return true;
}
