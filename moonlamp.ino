#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Price history storage
struct PriceData {
  float price;
  unsigned long timestamp;
};

#define MAX_HISTORY 20  // Store up to 20 price points
PriceData priceHistory[MAX_HISTORY];
int historyCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== Ethereum MoonLamp (WiFi Mode) ===");

  // Initialize LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Set LED to blue while connecting
  setColor(0, 0, LED_BRIGHTNESS);

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Flash green to indicate WiFi connected
    for (int i = 0; i < 3; i++) {
      setColor(0, LED_BRIGHTNESS, 0);
      delay(200);
      setColor(0, 0, 0);
      delay(200);
    }
  } else {
    Serial.println("\nWiFi Connection Failed!");
    Serial.println("Please check your WiFi credentials in config.h");

    // Flash red to indicate error
    while (true) {
      setColor(LED_BRIGHTNESS, 0, 0);
      delay(500);
      setColor(0, 0, 0);
      delay(500);
    }
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Fetch current ETH price
    float currentPrice = fetchEthPrice();

    if (currentPrice > 0) {
      Serial.print("Current ETH Price: $");
      Serial.println(currentPrice, 2);

      // Store price in history
      storePrice(currentPrice);

      // Get historical price for comparison
      float comparePrice = getHistoricalPrice(PRICE_HISTORY_MINUTES);

      if (comparePrice > 0) {
        Serial.print("Price ");
        Serial.print(PRICE_HISTORY_MINUTES);
        Serial.print(" min ago: $");
        Serial.println(comparePrice, 2);

        // Calculate change percentage
        float change = ((currentPrice - comparePrice) / comparePrice) * 100;

        Serial.print("Change: ");
        Serial.print(change, 2);
        Serial.println("%");

        // Update LED based on price movement
        updateLED(change);
      } else {
        // Still building history
        Serial.println("Building price history...");
        setColor(0, 0, LED_BRIGHTNESS); // Blue
      }
    } else {
      Serial.println("Failed to fetch price");
      setColor(255, 255, 0); // Yellow for error
    }
  } else {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.reconnect();
    setColor(LED_BRIGHTNESS, 0, 0); // Red for disconnected
  }

  // Wait before next check
  delay(CHECK_INTERVAL);
}

float fetchEthPrice() {
  HTTPClient http;

  Serial.println("Fetching ETH price from CoinGecko...");

  http.begin("https://api.coingecko.com/api/v3/simple/price?ids=ethereum&vs_currencies=usd");
  http.setTimeout(10000); // 10 second timeout

  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();

    // Parse JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
      float price = doc["ethereum"]["usd"];
      http.end();
      return price;
    } else {
      Serial.print("JSON parse error: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.print("HTTP error: ");
    Serial.println(httpCode);
  }

  http.end();
  return 0;
}

void storePrice(float price) {
  unsigned long currentTime = millis() / 1000; // Convert to seconds

  // Add to history
  if (historyCount < MAX_HISTORY) {
    priceHistory[historyCount].price = price;
    priceHistory[historyCount].timestamp = currentTime;
    historyCount++;
  } else {
    // Shift array and add new price
    for (int i = 0; i < MAX_HISTORY - 1; i++) {
      priceHistory[i] = priceHistory[i + 1];
    }
    priceHistory[MAX_HISTORY - 1].price = price;
    priceHistory[MAX_HISTORY - 1].timestamp = currentTime;
  }

  // Remove old data (older than 20 minutes)
  unsigned long cutoffTime = currentTime - (20 * 60);
  int newCount = 0;

  for (int i = 0; i < historyCount; i++) {
    if (priceHistory[i].timestamp > cutoffTime) {
      if (i != newCount) {
        priceHistory[newCount] = priceHistory[i];
      }
      newCount++;
    }
  }

  historyCount = newCount;
}

float getHistoricalPrice(int minutesAgo) {
  if (historyCount < 2) {
    return 0; // Not enough history
  }

  unsigned long currentTime = millis() / 1000;
  unsigned long targetTime = currentTime - (minutesAgo * 60);

  // Find closest price to target time
  int closestIndex = 0;
  unsigned long minDiff = abs((long)(priceHistory[0].timestamp - targetTime));

  for (int i = 1; i < historyCount; i++) {
    unsigned long diff = abs((long)(priceHistory[i].timestamp - targetTime));
    if (diff < minDiff) {
      minDiff = diff;
      closestIndex = i;
    }
  }

  return priceHistory[closestIndex].price;
}

void updateLED(float changePercent) {
  if (changePercent > 0.01) {
    // Price UP - Green
    Serial.println("Status: GREEN (UP)");
    setColor(0, LED_BRIGHTNESS, 0);
  } else if (changePercent < -0.01) {
    // Price DOWN - Red
    Serial.println("Status: RED (DOWN)");
    setColor(LED_BRIGHTNESS, 0, 0);
  } else {
    // No significant change - White
    Serial.println("Status: NEUTRAL");
    setColor(LED_BRIGHTNESS, LED_BRIGHTNESS, LED_BRIGHTNESS);
  }
}

void setColor(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}
