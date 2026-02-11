# Ethereum MoonLamp 🌙

An ESP32-powered LED lamp that changes color based on Ethereum price movements. Green when ETH is up, red when it's down!

**WiFi Mode**: This version connects to your WiFi network and runs completely standalone. The ESP32 fetches ETH prices directly from the internet - no computer needed!

## Hardware Requirements

- ESP32 development board (with WiFi)
- RGB LED (common cathode)
- 3x 220Ω resistors (one for each LED color)
- Breadboard and jumper wires
- USB cable (for programming only)
- USB power adapter or power bank (for standalone operation)

## Wiring Diagram

```
ESP32          RGB LED
GPIO 25  ---[220Ω]---  Red Pin
GPIO 26  ---[220Ω]---  Green Pin
GPIO 27  ---[220Ω]---  Blue Pin
GND      -------------  Common Cathode (-)
```

If you're using a common anode RGB LED, connect the common pin to 3.3V instead of GND, and you'll need to invert the color values in the code.

## Software Requirements

### Arduino IDE Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - Go to File > Preferences
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools > Board > Boards Manager
   - Search for "esp32" and install "esp32 by Espressif Systems"

### Arduino Libraries

Install via Library Manager (Sketch > Include Library > Manage Libraries):
- **ArduinoJson** by Benoit Blanchon (version 7.x)

## Installation

### Step 1: Configure WiFi

1. Open [config.h](config.h) in a text editor or Arduino IDE
2. Update your WiFi credentials:
   ```cpp
   #define WIFI_SSID "YOUR_WIFI_NAME"        // Replace with your WiFi network name
   #define WIFI_PASSWORD "YOUR_WIFI_PASSWORD" // Replace with your WiFi password
   ```
3. Adjust LED pin numbers if needed (default: R=25, G=26, B=27)
4. Adjust check interval if desired (default: 60 seconds)

### Step 2: Upload to ESP32

1. Open [moonlamp.ino](moonlamp.ino) in Arduino IDE
2. Select your ESP32 board:
   - Tools > Board > ESP32 Arduino > ESP32 Dev Module (or your specific board)
3. Select the correct COM port:
   - Tools > Port > (select your ESP32's port, e.g., COM3)
4. Click Upload
5. Open Serial Monitor (Tools > Serial Monitor) to see status messages
   - Set baud rate to 115200

### Step 3: Wire the Circuit

Connect your RGB LED to the ESP32:
```
ESP32          RGB LED
GPIO 25  ---[220Ω]---  Red Pin
GPIO 26  ---[220Ω]---  Green Pin
GPIO 27  ---[220Ω]---  Blue Pin
GND      -------------  Common Cathode (-)
```

### Step 4: Power On

1. Disconnect from computer (optional)
2. Connect ESP32 to USB power adapter or power bank
3. The lamp will:
   - Show **blue** while connecting to WiFi
   - Flash **green** 3 times when WiFi connected
   - Flash **red** continuously if WiFi connection fails
   - Start tracking ETH prices automatically!

## How It Works

1. **ESP32 connects to your WiFi** network on startup
2. **Fetches ETH price** from CoinGecko API every 60 seconds
3. **Stores price history** in memory for comparison
4. **Compares current price** to 5 minutes ago
5. **Updates LED color** based on price movement:
   - Green = Price UP
   - Red = Price DOWN
   - White = No change
   - Blue = Building price history

Everything runs on the ESP32 - no computer needed!

## LED Color Meanings

- **Green**: Price is UP compared to 5 minutes ago
- **Red**: Price is DOWN compared to 5 minutes ago
- **White**: No change
- **Blue**: Still gathering price history (first few minutes)
- **Yellow**: Error (connection issue or data problem)

## Configuration

Edit [config.h](config.h) to customize:

| Setting | Default | Description |
|---------|---------|-------------|
| `WIFI_SSID` | YOUR_WIFI_NAME | Your WiFi network name |
| `WIFI_PASSWORD` | YOUR_WIFI_PASSWORD | Your WiFi password |
| `RED_PIN` | 25 | GPIO pin for red LED |
| `GREEN_PIN` | 26 | GPIO pin for green LED |
| `BLUE_PIN` | 27 | GPIO pin for blue LED |
| `LED_BRIGHTNESS` | 255 | LED brightness (0-255) |
| `CHECK_INTERVAL` | 60000 | Price check interval in milliseconds |
| `PRICE_HISTORY_MINUTES` | 5 | Compare to price from X minutes ago |

## Troubleshooting

### LED doesn't light up
- Check your wiring connections
- Verify you're using the correct GPIO pins in [config.h](config.h)
- Make sure you're using appropriate resistors (220Ω recommended)
- Check if you have common cathode or common anode RGB LED

### LED flashes red continuously
This means WiFi connection failed:
- Check WiFi credentials in [config.h](config.h) are correct
- Make sure your WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Check if your WiFi is working
- Open Serial Monitor (115200 baud) to see detailed error messages

### Price not updating
- Check Serial Monitor for error messages
- Verify internet connection is working
- CoinGecko API might be temporarily down - wait a few minutes
- Check if firewall is blocking the ESP32

### Upload fails
- Make sure you selected the correct board and port in Arduino IDE
- Try pressing the BOOT button on ESP32 while uploading
- Check USB cable supports data transfer (not just charging)

### "Brownout detector was triggered" error
- Your power supply may be insufficient
- Try a different USB power adapter (2A recommended)
- Some ESP32 boards are more sensitive to power issues

## Example Output

Open Serial Monitor (115200 baud) to see:

```
=== Ethereum MoonLamp (WiFi Mode) ===
Connecting to WiFi: YourWiFiName
...
WiFi Connected!
IP Address: 192.168.1.100

Fetching ETH price from CoinGecko...
Current ETH Price: $2456.78
Building price history...
Status: BLUE (Building history)

Fetching ETH price from CoinGecko...
Current ETH Price: $2458.12
Price 5 min ago: $2450.00
Change: 0.33%
Status: GREEN (UP)

Fetching ETH price from CoinGecko...
Current ETH Price: $2454.50
Price 5 min ago: $2456.78
Change: -0.09%
Status: RED (DOWN)
```

## API Information

Uses the free CoinGecko API (no API key required):
- Endpoint: `https://api.coingecko.com/api/v3/simple/price?ids=ethereum&vs_currencies=usd`
- Rate limit: ~10-50 calls/minute (free tier)
- No authentication needed

## Power Options

The ESP32 can be powered by:
- **USB wall adapter** (most common, 5V 2A recommended)
- **Power bank** (for portable operation)
- **USB port on computer** (works but less convenient)
- **Battery pack** with voltage regulator (for truly wireless operation)

## Future Enhancements

Ideas for expansion:
- Support multiple cryptocurrencies (Bitcoin, Solana, etc.)
- Add OLED display for price info
- Web interface for configuration and monitoring
- Smooth color transitions between states
- Configurable thresholds for color changes
- Sound alerts for major price movements
- Historical price graph on web dashboard
- OTA (Over-The-Air) firmware updates

## License

MIT License - feel free to modify and use as you wish!

## Credits

Built for tracking Ethereum to the moon! 🚀

Uses:
- CoinGecko API for price data
- ESP32 WiFi for standalone operation
- ArduinoJson for data parsing
