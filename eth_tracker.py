import serial
import requests
import time
import json
from datetime import datetime

# Configuration
SERIAL_PORT = 'COM3'  # Change this to match your ESP32's COM port (Windows) or /dev/ttyUSB0 (Linux/Mac)
BAUD_RATE = 115200
CHECK_INTERVAL = 60  # Check price every 60 seconds
PRICE_HISTORY_MINUTES = 5  # Compare to price from 5 minutes ago

# CoinGecko API endpoint
API_URL = "https://api.coingecko.com/api/v3/simple/price?ids=ethereum&vs_currencies=usd"

# Price history storage
price_history = []

def fetch_eth_price():
    """Fetch current Ethereum price from CoinGecko API"""
    try:
        response = requests.get(API_URL, timeout=10)
        response.raise_for_status()
        data = response.json()
        price = data['ethereum']['usd']
        return price
    except Exception as e:
        print(f"Error fetching price: {e}")
        return None

def store_price(price):
    """Store price with timestamp"""
    timestamp = time.time()
    price_history.append({'price': price, 'timestamp': timestamp})

    # Keep only last 20 minutes of data
    cutoff_time = timestamp - (20 * 60)
    global price_history
    price_history = [p for p in price_history if p['timestamp'] > cutoff_time]

def get_historical_price(minutes_ago):
    """Get price from X minutes ago"""
    if len(price_history) < 2:
        return None

    target_time = time.time() - (minutes_ago * 60)

    # Find closest price to target time
    closest = min(price_history, key=lambda p: abs(p['timestamp'] - target_time))
    return closest['price']

def send_to_esp32(ser, current_price, compare_price):
    """Send price data to ESP32 via serial"""
    if compare_price is None:
        status = 'WAITING'
        change = 0
    else:
        change = ((current_price - compare_price) / compare_price) * 100
        if change > 0:
            status = 'GREEN'
        elif change < 0:
            status = 'RED'
        else:
            status = 'NEUTRAL'

    # Create JSON message
    message = {
        'price': current_price,
        'status': status,
        'change': round(change, 2)
    }

    # Send to ESP32
    json_str = json.dumps(message) + '\n'
    ser.write(json_str.encode())

    return status, change

def main():
    print("=== Ethereum MoonLamp Tracker ===")
    print(f"Connecting to ESP32 on {SERIAL_PORT}...")

    try:
        # Open serial connection
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        time.sleep(2)  # Wait for connection to stabilize
        print("Connected to ESP32!")

        while True:
            # Fetch current price
            current_price = fetch_eth_price()

            if current_price is not None:
                print(f"\n[{datetime.now().strftime('%H:%M:%S')}] Current ETH Price: ${current_price:,.2f}")

                # Store in history
                store_price(current_price)

                # Get historical price for comparison
                compare_price = get_historical_price(PRICE_HISTORY_MINUTES)

                if compare_price is not None:
                    print(f"Price {PRICE_HISTORY_MINUTES} min ago: ${compare_price:,.2f}")

                # Send data to ESP32
                status, change = send_to_esp32(ser, current_price, compare_price)

                if status == 'WAITING':
                    print("Status: 🔵 BLUE - Building price history...")
                elif status == 'GREEN':
                    print(f"Status: 🟢 GREEN - UP {change:.2f}%")
                elif status == 'RED':
                    print(f"Status: 🔴 RED - DOWN {change:.2f}%")
                else:
                    print("Status: ⚪ NEUTRAL - No change")
            else:
                print("Failed to fetch price, retrying...")

            # Wait before next check
            time.sleep(CHECK_INTERVAL)

    except serial.SerialException as e:
        print(f"\nSerial connection error: {e}")
        print(f"\nTips:")
        print(f"1. Make sure ESP32 is connected via USB")
        print(f"2. Check the COM port in Device Manager (Windows) or ls /dev/tty* (Mac/Linux)")
        print(f"3. Close Arduino IDE Serial Monitor if it's open")
        print(f"4. Update SERIAL_PORT in this script to match your ESP32's port")
    except KeyboardInterrupt:
        print("\n\nShutting down...")
        if 'ser' in locals():
            ser.close()
        print("Goodbye!")

if __name__ == "__main__":
    main()
