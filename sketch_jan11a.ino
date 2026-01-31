
 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <esp_now.h>
SoftwareSerial gsm(7, 8); // RX, TX
Adafruit_SSD1306 srituhobby = Adafruit_SSD1306(128, 64, &Wire);

#define sensor A0
#define Highpulse 540
#define BPM_HIGH 120
#define BPM_LOW 50

unsigned long lastAlert = 0;
const unsigned long alertDelay = 300000; // 5 minutes

bool pulseDetected = false; 


int sX = 0;
int sY = 60;
int x = 0;
int Svalue;
int value;
long Stime = 0;
long Ltime = 0;
int count = 0;
int Bpm = 0;

typedef struct struct_message {
  int bpm;
  bool highAlert;
  bool lowAlert;
} struct_message;

struct_message incomingData;

void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy(&incomingData, data, sizeof(incomingData));

  Serial.print("BPM: ");
  Serial.println(incomingData.bpm);

  if (incomingData.highAlert) {
    Serial.println("⚠️ HIGH HEART RATE");
  }

  if (incomingData.lowAlert) {
    Serial.println("⚠️ LOW HEART RATE");
  }
}


void setup() {
  Serial.begin(9600);
  srituhobby.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(1000);
  srituhobby.clearDisplay();

  gsm.begin(9600);
delay(2000);

gsm.println("AT");
delay(1000);

gsm.println("AT+CMGF=1"); // SMS text mode
delay(1000);

WiFi.mode(WIFI_STA);

  Serial.print("Receiver MAC: ");
  Serial.println(WiFi.macAddress());

  esp_now_init();
  esp_now_register_recv_cb(onReceive);

}

void loop() {
  Svalue = analogRead(sensor);
  Serial.println(Svalue);
  value = map(Svalue, 0, 1024, 0, 45);

  int y = 60 - value;

  if (x > 128) {
    x = 0;
    sX = 0;
    srituhobby.clearDisplay();
  }

  srituhobby.drawLine(sX, sY, x, y, WHITE);
  sX = x;
  sY = y;
  x ++;

  BPM();

  srituhobby.setCursor(0, 0);
  srituhobby.setTextSize(2);
  srituhobby.setTextColor(SSD1306_WHITE);
  srituhobby.print("BPM :");
  srituhobby.display();

}

void BPM() {

  if (Svalue > Highpulse && !pulseDetected) {
    pulseDetected = true;
    count++;
  }

  if (Svalue < Highpulse) {
    pulseDetected = false;
  }

  Stime = millis() - Ltime;

  if (Stime >= 60000) {   // 60 seconds
    Ltime = millis();
    Bpm = count;
    count = 0;

    // Display BPM
    srituhobby.setCursor(60, 0);
    srituhobby.setTextSize(2);
    srituhobby.setTextColor(SSD1306_WHITE);
    srituhobby.print(Bpm);
    srituhobby.print("   ");
    srituhobby.display();

    // Alerts
    if (Bpm > BPM_HIGH && millis() - lastAlert > alertDelay) {
      sendSMS("ALERT: Heart rate too HIGH! BPM = " + String(Bpm));
      sendSMS("FN: Isabella Erich Serrano Cresencio
	Age: 15
	Phone no.: +6396 9xxx xxxx
	Emergency Contact: Mother - Ana Cristina Serrano Cresencio 
Emergency Contact no.: +6391 8xxx xxxx
	Chronic Medical Conditions: Scoliosis");
      lastAlert = millis();
    }

    if (Bpm < BPM_LOW && millis() - lastAlert > alertDelay) {
      sendSMS("ALERT: Heart rate too LOW! BPM = " + String(Bpm));
      sendSMS("FN: Isabella Erich Serrano Cresencio
	Age: 15
	Phone no.: +6396 9xxx xxxx
	Emergency Contact: Mother - Ana Cristina Serrano Cresencio 
Emergency Contact no.: +6391 8xxx xxxx
	Chronic Medical Conditions: Scoliosis");
      lastAlert = millis();
    }
  }
}
void sendSMS(String message) {
  gsm.println("AT+CMGS=\"+639XXXXXXXXX\""); // replace with phone number
  delay(1000);
  gsm.print(message);
  delay(500);
  gsm.write(26);
  delay(3000);
}
