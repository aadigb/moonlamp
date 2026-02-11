#include <ArduinoJson.h>
#include "config.h"

String inputBuffer = "";

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== Ethereum MoonLamp (Serial Mode) ===");
  Serial.println("Waiting for data from laptop...");

  // Initialize LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Set LED to blue while waiting for data
  setColor(0, 0, LED_BRIGHTNESS);
}

void loop() {
  // Read data from serial
  while (Serial.available() > 0) {
    char c = Serial.read();

    if (c == '\n') {
      // Complete message received, process it
      processMessage(inputBuffer);
      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }

  delay(10);
}

void processMessage(String message) {
  // Parse JSON message from Python script
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.c_str());
    setColor(255, 255, 0); // Yellow for error
    return;
  }

  // Extract data
  float price = doc["price"];
  String status = doc["status"].as<String>();
  float change = doc["change"];

  // Display info
  Serial.print("ETH: $");
  Serial.print(price, 2);
  Serial.print(" | Change: ");
  Serial.print(change, 2);
  Serial.print("% | Status: ");
  Serial.println(status);

  // Update LED based on status
  if (status == "GREEN") {
    setColor(0, LED_BRIGHTNESS, 0); // Green - price up
  } else if (status == "RED") {
    setColor(LED_BRIGHTNESS, 0, 0); // Red - price down
  } else if (status == "NEUTRAL") {
    setColor(LED_BRIGHTNESS, LED_BRIGHTNESS, LED_BRIGHTNESS); // White - no change
  } else if (status == "WAITING") {
    setColor(0, 0, LED_BRIGHTNESS); // Blue - building history
  } else {
    setColor(255, 255, 0); // Yellow - unknown status
  }
}

void setColor(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}
