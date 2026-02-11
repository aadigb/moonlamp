const { SerialPort } = require('serialport');
const axios = require('axios');

// Configuration
const SERIAL_PORT = 'COM3';  // Change this to match your ESP32's COM port (Windows) or /dev/ttyUSB0 (Linux/Mac)
const BAUD_RATE = 115200;
const CHECK_INTERVAL = 60;  // Check price every 60 seconds
const PRICE_HISTORY_MINUTES = 5;  // Compare to price from 5 minutes ago

// CoinGecko API endpoint
const API_URL = "https://api.coingecko.com/api/v3/simple/price?ids=ethereum&vs_currencies=usd";

// Price history storage
let priceHistory = [];

/**
 * Fetch current Ethereum price from CoinGecko API
 */
async function fetchEthPrice() {
    try {
        const response = await axios.get(API_URL, { timeout: 10000 });
        const price = response.data.ethereum.usd;
        return price;
    } catch (error) {
        console.error(`Error fetching price: ${error.message}`);
        return null;
    }
}

/**
 * Store price with timestamp
 */
function storePrice(price) {
    const timestamp = Date.now() / 1000; // Convert to seconds
    priceHistory.push({ price, timestamp });

    // Keep only last 20 minutes of data
    const cutoffTime = timestamp - (20 * 60);
    priceHistory = priceHistory.filter(p => p.timestamp > cutoffTime);
}

/**
 * Get price from X minutes ago
 */
function getHistoricalPrice(minutesAgo) {
    if (priceHistory.length < 2) {
        return null;
    }

    const targetTime = (Date.now() / 1000) - (minutesAgo * 60);

    // Find closest price to target time
    const closest = priceHistory.reduce((prev, curr) => {
        return Math.abs(curr.timestamp - targetTime) < Math.abs(prev.timestamp - targetTime) ? curr : prev;
    });

    return closest.price;
}

/**
 * Send price data to ESP32 via serial
 */
function sendToESP32(port, currentPrice, comparePrice) {
    let status, change;

    if (comparePrice === null) {
        status = 'WAITING';
        change = 0;
    } else {
        change = ((currentPrice - comparePrice) / comparePrice) * 100;
        if (change > 0) {
            status = 'GREEN';
        } else if (change < 0) {
            status = 'RED';
        } else {
            status = 'NEUTRAL';
        }
    }

    // Create JSON message
    const message = {
        price: currentPrice,
        status: status,
        change: Math.round(change * 100) / 100
    };

    // Send to ESP32
    const jsonStr = JSON.stringify(message) + '\n';
    port.write(jsonStr);

    return { status, change };
}

/**
 * Format current time as HH:MM:SS
 */
function getCurrentTime() {
    const now = new Date();
    return now.toLocaleTimeString('en-US', { hour12: false });
}

/**
 * Main function
 */
async function main() {
    console.log("=== Ethereum MoonLamp Tracker ===");
    console.log(`Connecting to ESP32 on ${SERIAL_PORT}...`);

    try {
        // Open serial connection
        const port = new SerialPort({
            path: SERIAL_PORT,
            baudRate: BAUD_RATE
        });

        // Wait for port to open
        await new Promise((resolve, reject) => {
            port.on('open', () => {
                console.log("Connected to ESP32!");
                resolve();
            });
            port.on('error', reject);
        });

        // Read data from ESP32
        port.on('data', (data) => {
            // Display messages from ESP32 (optional)
            // console.log('ESP32:', data.toString().trim());
        });

        // Wait a bit for connection to stabilize
        await new Promise(resolve => setTimeout(resolve, 2000));

        // Main loop
        while (true) {
            // Fetch current price
            const currentPrice = await fetchEthPrice();

            if (currentPrice !== null) {
                console.log(`\n[${getCurrentTime()}] Current ETH Price: $${currentPrice.toLocaleString('en-US', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}`);

                // Store in history
                storePrice(currentPrice);

                // Get historical price for comparison
                const comparePrice = getHistoricalPrice(PRICE_HISTORY_MINUTES);

                if (comparePrice !== null) {
                    console.log(`Price ${PRICE_HISTORY_MINUTES} min ago: $${comparePrice.toLocaleString('en-US', { minimumFractionDigits: 2, maximumFractionDigits: 2 })}`);
                }

                // Send data to ESP32
                const { status, change } = sendToESP32(port, currentPrice, comparePrice);

                if (status === 'WAITING') {
                    console.log("Status: 🔵 BLUE - Building price history...");
                } else if (status === 'GREEN') {
                    console.log(`Status: 🟢 GREEN - UP ${change.toFixed(2)}%`);
                } else if (status === 'RED') {
                    console.log(`Status: 🔴 RED - DOWN ${change.toFixed(2)}%`);
                } else {
                    console.log("Status: ⚪ NEUTRAL - No change");
                }
            } else {
                console.log("Failed to fetch price, retrying...");
            }

            // Wait before next check
            await new Promise(resolve => setTimeout(resolve, CHECK_INTERVAL * 1000));
        }

    } catch (error) {
        if (error.code === 'ENOENT' || error.message.includes('No such file')) {
            console.error(`\nSerial connection error: ${error.message}`);
            console.log(`\nTips:`);
            console.log(`1. Make sure ESP32 is connected via USB`);
            console.log(`2. Check the COM port in Device Manager (Windows) or ls /dev/tty* (Mac/Linux)`);
            console.log(`3. Close Arduino IDE Serial Monitor if it's open`);
            console.log(`4. Update SERIAL_PORT in this script to match your ESP32's port`);
        } else {
            console.error(`\nError: ${error.message}`);
        }
        process.exit(1);
    }
}

// Handle Ctrl+C gracefully
process.on('SIGINT', () => {
    console.log("\n\nShutting down...");
    console.log("Goodbye!");
    process.exit(0);
});

// Run the main function
main();
