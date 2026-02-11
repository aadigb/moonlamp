# Ethereum MoonLamp 🌙

An ESP32-powered LED lamp that changes color based on Ethereum price movements. Green when ETH is up, red when it's down!

**Serial Mode**: This version connects to your laptop via USB. A Python script on your laptop fetches ETH prices and sends them to the ESP32.

## Hardware Requirements

- ESP32 development board
- RGB LED (common cathode)
- 3x 220Ω resistors (one for each LED color)
- Breadboard and jumper wires
- USB cable (for both programming and operation)

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

### 1. Arduino IDE Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add ESP32 board support:
   - Go to File > Preferences
   - Add this URL to "Additional Board Manager URLs":
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to Tools > Board > Boards Manager
   - Search for "esp32" and install "esp32 by Espressif Systems"

### 2. Arduino Libraries

Install via Library Manager (Sketch > Include Library > Manage Libraries):
- **ArduinoJson** by Benoit Blanchon (version 7.x)

### 3. Python Requirements

Install Python 3.7 or newer, then install required packages:

```bash
pip install pyserial requests
```

## Installation

### Step 1: Upload to ESP32

1. Open [moonlamp.ino](moonlamp.ino) in Arduino IDE
2. Edit [config.h](config.h) to adjust pin numbers if needed (default: R=25, G=26, B=27)
3. Select your ESP32 board:
   - Tools > Board > ESP32 Arduino > ESP32 Dev Module (or your specific board)
4. Select the correct COM port:
   - Tools > Port > (select your ESP32's port, e.g., COM3)
5. Click Upload
6. Note which COM port you used

### Step 2: Run Python Script

1. Close Arduino IDE Serial Monitor (if open)
2. Edit [eth_tracker.py](eth_tracker.py) and update the COM port:
   ```python
   SERIAL_PORT = 'COM3'  # Change to match your ESP32's port
   ```
3. Run the script:
   ```bash
   python eth_tracker.py
   ```

## How It Works

1. **Python script on your laptop**:
   - Fetches current Ethereum price from CoinGecko API every 60 seconds
   - Stores price history for comparison
   - Compares current price to 5 minutes ago
   - Sends status to ESP32 via USB serial

2. **ESP32**:
   - Receives price data over serial
   - Updates LED color based on price movement
   - Displays status information back to laptop

## LED Color Meanings

- **Green**: Price is UP compared to 5 minutes ago
- **Red**: Price is DOWN compared to 5 minutes ago
- **White**: No change
- **Blue**: Still gathering price history (first few minutes)
- **Yellow**: Error (connection issue or data problem)

## Configuration

### ESP32 Settings

Edit [config.h](config.h):

| Setting | Default | Description |
|---------|---------|-------------|
| `RED_PIN` | 25 | GPIO pin for red LED |
| `GREEN_PIN` | 26 | GPIO pin for green LED |
| `BLUE_PIN` | 27 | GPIO pin for blue LED |
| `LED_BRIGHTNESS` | 255 | LED brightness (0-255) |

### Python Script Settings

Edit [eth_tracker.py](eth_tracker.py):

| Setting | Default | Description |
|---------|---------|-------------|
| `SERIAL_PORT` | COM3 | ESP32's COM port (Windows: COM3, Linux/Mac: /dev/ttyUSB0) |
| `CHECK_INTERVAL` | 60 | Price check interval in seconds |
| `PRICE_HISTORY_MINUTES` | 5 | Compare to price from X minutes ago |

## Troubleshooting

### LED doesn't light up
- Check your wiring connections
- Verify you're using the correct GPIO pins in [config.h](config.h)
- Make sure you're using appropriate resistors (220Ω recommended)
- Check if you have common cathode or common anode RGB LED

### Python script can't connect
- Make sure ESP32 is connected via USB
- Close Arduino IDE Serial Monitor if it's open (only one program can use the serial port)
- Check the COM port in Device Manager (Windows) or `ls /dev/tty*` (Mac/Linux)
- Update `SERIAL_PORT` in [eth_tracker.py](eth_tracker.py) to match your ESP32's port
- Try unplugging and replugging the USB cable

### "Module not found" errors
Install missing Python packages:
```bash
pip install pyserial requests
```

### Upload fails
- Make sure you selected the correct board and port in Arduino IDE
- Try pressing the BOOT button on ESP32 while uploading
- Check USB cable supports data transfer (not just charging)

## Example Output

When running [eth_tracker.py](eth_tracker.py), you'll see:

```
=== Ethereum MoonLamp Tracker ===
Connecting to ESP32 on COM3...
Connected to ESP32!

[19:45:30] Current ETH Price: $2,456.78
Status: 🔵 BLUE - Building price history...

[19:46:30] Current ETH Price: $2,458.12
Price 5 min ago: $2,450.00
Status: 🟢 GREEN - UP 0.33%

[19:47:30] Current ETH Price: $2,454.50
Price 5 min ago: $2,456.78
Status: 🔴 RED - DOWN -0.09%
```

## API Information

Uses the free CoinGecko API (no API key required):
- Endpoint: `https://api.coingecko.com/api/v3/simple/price?ids=ethereum&vs_currencies=usd`
- Rate limit: ~10-50 calls/minute (free tier)
- No authentication needed

## Running Automatically

### Windows
Create a `.bat` file to run the script:
```batch
@echo off
cd C:\miniapps\moonlamp
python eth_tracker.py
pause
```

### Mac/Linux
Create a shell script:
```bash
#!/bin/bash
cd ~/miniapps/moonlamp
python3 eth_tracker.py
```

Make it executable: `chmod +x run_moonlamp.sh`

## Future Enhancements

Ideas for expansion:
- WiFi mode for standalone operation (no laptop needed)
- Support multiple cryptocurrencies
- Add OLED display for price info
- Web interface for remote monitoring
- Smooth color transitions
- Configurable thresholds for color changes
- Sound alerts for major price movements

## License

MIT License - feel free to modify and use as you wish!

## Credits

Built for tracking Ethereum to the moon! 🚀

Uses:
- CoinGecko API for price data
- Python for data fetching
- ESP32 for LED control
