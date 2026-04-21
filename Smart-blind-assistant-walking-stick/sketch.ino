#include <WiFi.h>
#include <HTTPClient.h>

// ---------------- PIN VARIABLES ----------------
int TRIG = 5;
int ECHO = 18;
int LED_PIN = 25;
int BUZZER = 21;
int LDR_PIN = 34;
int POT_PIN = 35;
int SOS_BUTTON = 23;

// ---------------- TELEGRAM SETTINGS ----------------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";

const char* BOT_TOKEN = "8530480384:AAHuloGGdrOCX42S6Hwrh85F37Pgq9o3B_s";  // <-- YOUR TOKEN
const char* CHAT_ID   = "1911766325";                                      // <-- YOUR CHAT ID

// ---------------- SETUP ----------------
void setup() {
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(SOS_BUTTON, INPUT_PULLUP);

  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ---------------- DISTANCE FUNCTION ----------------
long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  return duration * 0.034 / 2;
}

// ---------------- SEND TELEGRAM SOS MESSAGE ----------------
void sendTelegramSOS() {
  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;
    String url = "https://api.telegram.org/bot" + String(BOT_TOKEN) + "/sendMessage";

    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String message =
      "{"
        "\"chat_id\": \"" + String(CHAT_ID) + "\","
        "\"text\": \" *SOS ALERT* \\nRahul needs help!\\n"
        "Location: https://maps.google.com/?q=10.9020,76.9036\","
        "\"parse_mode\": \"Markdown\""
      "}";

    int response = http.POST(message);

    Serial.print("Telegram Response Code: ");
    Serial.println(response);

    if (response > 0) {
      Serial.println("Telegram Response:");
      Serial.println(http.getString());
    }

    http.end();
  }
}

// ---------------- BUZZER PATTERN ----------------
void alertPattern(int speed, int toneFreq) {
  tone(BUZZER, toneFreq);
  delay(speed);
  noTone(BUZZER);
  delay(speed);
}

// ---------------- SOS BUZZER + LED PATTERN ----------------
void sosAlert() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER, 3000);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER);
    delay(200);
  }
}

// ---------------- MAIN LOOP ----------------
void loop() {

  long distance = getDistance();
  int ldrValue = analogRead(LDR_PIN);
  int potValue = analogRead(POT_PIN);

  int threshold = map(potValue, 0, 4095, 20, 100);

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Threshold: ");
  Serial.print(threshold);
  Serial.print(" | LDR: ");
  Serial.println(ldrValue);

  // --------------- LOW LIGHT → TURN LED ON ---------------
  if (ldrValue > 1500)
    digitalWrite(LED_PIN, HIGH);
  else
    digitalWrite(LED_PIN, LOW);

  // --------------- SOS BUTTON CHECK ---------------
  if (digitalRead(SOS_BUTTON) == LOW) {
    delay(50);

    if (digitalRead(SOS_BUTTON) == LOW) {

      Serial.println("\n===== 🚨 SOS BUTTON PRESSED 🚨 =====");
      Serial.println("Sending Telegram Alert...");

      sendTelegramSOS();   // SEND TELEGRAM MESSAGE

      sosAlert();          // MAKE SOUND + FLASH LED

      // Wait until button releases
      while (digitalRead(SOS_BUTTON) == LOW) delay(50);
      delay(200);
    }
  }

  // --------------- BUZZER DISTANCE LOGIC ---------------
  if (distance >= threshold || distance <= 0) {
    noTone(BUZZER);
  }
  else if (distance >= 50) {
    alertPattern(200, 1000);
  }
  else if (distance >= 30) {
    alertPattern(120, 1500);
  }
  else if (distance >= 15) {
    alertPattern(70, 2000);
  }
  else {
    tone(BUZZER, 2500);
  }

  delay(50);
}
