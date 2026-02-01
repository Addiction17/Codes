#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const String PHONE = "+09xxx";

#define rxPin 10
#define txPin 11
#define BUTTON_PIN 2
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define rxPin 10
#define txPin 11
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SoftwareSerial sim800(rxPin, txPin);

bool lastButtonState = HIGH;

AltSoftSerial neogps;
TinyGPSPlus gps;

String receivedBuffer = "";
String sender_number;
String msg;

void setup() {
  Serial.begin(115200);
  sim800.begin(9600);
  neogps.begin(9600);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  sendSimCommand("AT");
  sendSimCommand("ATE1");
  sendSimCommand("AT+CPIN?");
  sendSimCommand("AT+CMGF=1");
  sendSimCommand("AT+CNMI=2,2,0,0,0");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  Serial.println(F("OLED not found"));
  while (true);
}

display.clearDisplay();
display.setTextSize(1);
display.setTextColor(SSD1306_WHITE);
display.setCursor(0, 0);
display.println("Hello! GPS Tracker Read.");
display.display();
}

void loop() {
  readGpsData();
  updateDisplay();

   bool currentButtonState = digitalRead(BUTTON_PIN);

   if (lastButtonState == HIGH && currentButtonState == LOW) {
  if (gps.location.isValid()) {
    sendSmsGPS();
  }
}
  // Detect button press
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    sendSmsGPS();
    delay(1000);
  }

  lastButtonState = currentButtonState;
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);

  if (gps.location.isValid()) {
    display.println("GPS: LOCKED");
    display.print("Lat: ");
    display.println(gps.location.lat(), 6);
    display.print("Lng: ");
    display.println(gps.location.lng(), 6);
  } else {
    display.println("GPS: SEARCHING...");
  }

  display.display();
}


void sendSimCommand(String command) {
  sim800.println(command);
}

void readGpsData() {
  while (neogps.available()) {
    gps.encode(neogps.read());
  }
}


void sendSmsGPS() {
  if (gps.location.isValid()) {
    sim800.print("AT+CMGF=1\r\n");
    sim800.print("AT+CMGS=\"" + PHONE + "\"\r\n");
    sim800.print("http://maps.google.com/maps?q=?q=");
    sim800.print(gps.location.lat(), 6);
    sim800.print(",");
    sim800.print(gps.location.lng(), 6);
    sim800.write(0x1A);
  } else {
    sendSms("GPS signal not available. Please try again.");
  }
}

void sendSms(String text) {
  sim800.print("AT+CMGF=1\r\n");
  sim800.print("AT+CMGS=\"" + PHONE + "\"\r\n");
  sim800.print(text);
  sim800.write(0x1A);
}


