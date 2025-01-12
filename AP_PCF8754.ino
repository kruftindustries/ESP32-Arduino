#include <WiFi.h>
#include <WiFiClient.h>
#include <Wire.h>
#include <INA226.h>
#include <PCF8574.h>

// Pin Definitions
static const uint8_t PIN_SDA = 4;
static const uint8_t PIN_SCL = 5;

// Timing Constants
static const uint INPUT_LOOP_MS = 100;
static const uint OUTPUT_LOOP_MS = 100;
static const uint SENSOR_LOOP_MS = 3000;

// WiFi Settings
const char *ssid = "yourAP";
const char *password = "yourPassword";

// I2C Device Initialization
PCF8574 pcf21(0x21);  // Input Bank 1
PCF8574 pcf22(0x22);  // Input Bank 2
PCF8574 pcf24(0x24);  // Output Bank 1
PCF8574 pcf25(0x25);  // Output Bank 2
INA226 powerSensor(0x40);

// State Variables
struct IOState {
    uint16_t inputs = 0;
    uint16_t lastInputs = 0;
    uint16_t outputs = 0;
    uint16_t lastOutputs = 0;
} ioState;

struct SensorData {
    float voltage = 0;
    float current = 0;
    float power = 0;
} sensorData;

// Timing Variables
unsigned long lastInputUpdate = 0;
unsigned long lastOutputUpdate = 0;
unsigned long lastSensorUpdate = 0;

WiFiServer server(80);

// Modern and responsive web interface, takes about 5s to load
static const char WEBPAGE[] PROGMEM = R"EOF(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 IO Control</title>
    <style>
        :root {
            --primary: #2196F3;
            --success: #4CAF50;
            --danger: #f44336;
            --gray: #9e9e9e;
        }
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            margin: 0;
            padding: 20px;
            background: #f5f5f5;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .card {
            background: white;
            border-radius: 8px;
            padding: 20px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
            margin-bottom: 20px;
        }
        .grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }
        .io-grid {
            display: grid;
            grid-template-columns: repeat(8, minmax(0, 1fr));
            gap: 10px;
            margin-top: 15px;
            width: 100%;
            max-width: 100%;
            overflow-x: auto;
        }
        .io-item {
            text-align: center;
            padding: 10px;
            border-radius: 4px;
            background: #f5f5f5;
            min-width: 80px;
        }
        .sensor-value {
            font-size: 24px;
            font-weight: bold;
            color: var(--primary);
        }
        .sensor-label {
            color: var(--gray);
            font-size: 14px;
        }
        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        input:checked + .slider {
            background-color: var(--success);
        }
        input:checked + .slider:before {
            transform: translateX(26px);
        }
        .input-indicator {
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background-color: #ccc;
            margin: 0 auto;
        }
        .input-indicator.active {
            background-color: var(--success);
        }
        h2 {
            color: #333;
            margin-top: 0;
        }
        .status {
            margin-top: 20px;
            padding: 15px;
            border-radius: 4px;
            background: #e3f2fd;
            color: var(--primary);
        }
        .io-scroll-container {
            overflow-x: auto;
            padding: 10px 0;
        }
        @media (max-width: 768px) {
            .io-grid {
                grid-template-columns: repeat(4, minmax(0, 1fr));
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="card">
            <h2>System Status</h2>
            <div class="grid">
                <div class="sensor-box">
                    <div class="sensor-value" id="voltage">0.00V</div>
                    <div class="sensor-label">Voltage</div>
                </div>
                <div class="sensor-box">
                    <div class="sensor-value" id="current">0.00A</div>
                    <div class="sensor-label">Current</div>
                </div>
                <div class="sensor-box">
                    <div class="sensor-value" id="power">0.00W</div>
                    <div class="sensor-label">Power</div>
                </div>
            </div>
        </div>

        <div class="card">
            <h2>Digital Inputs</h2>
            <div class="io-scroll-container">
                <div class="io-grid" id="inputs"></div>
            </div>
        </div>

        <div class="card">
            <h2>Digital Outputs</h2>
            <div class="io-scroll-container">
                <div class="io-grid" id="outputs"></div>
            </div>
        </div>

        <div class="status" id="status"></div>
    </div>

    <script>
        // Initialize data arrays for the chart
        const maxDataPoints = 50;
        const timeLabels = [];
        const voltageData = [];
        const currentData = [];
        const powerData = [];

        function createIOPoints() {
            const inputsDiv = document.getElementById('inputs');
            const outputsDiv = document.getElementById('outputs');

            // Create 16 input indicators
            for (let i = 0; i < 16; i++) {
                const input = document.createElement('div');
                input.className = 'io-item';
                input.innerHTML = `
                    <div class="input-indicator" id="in${i}"></div>
                    <div>Input ${i}</div>
                `;
                inputsDiv.appendChild(input);
            }

            // Create 16 output switches
            for (let i = 0; i < 16; i++) {
                const output = document.createElement('div');
                output.className = 'io-item';
                output.innerHTML = `
                    <label class="switch">
                        <input type="checkbox" id="out${i}" onchange="toggleOutput(${i}, this.checked)">
                        <span class="slider"></span>
                    </label>
                    <div>Output ${i}</div>
                `;
                outputsDiv.appendChild(output);
            }
        }

        function toggleOutput(id, state) {
            fetch(`/output?id=out${id}&val=${state ? 1 : 0}`)
                .then(response => response.text())
                .then(updateStatus)
                .catch(error => console.error('Error:', error));
        }

        function updateStatus(data) {
            const [inputs, voltage, current, power] = data.split(',');
            
            // Update inputs
            const inputBits = parseInt(inputs).toString(2).padStart(16, '0');
            for (let i = 0; i < 16; i++) {
                const indicator = document.getElementById(`in${i}`);
                indicator.className = `input-indicator ${inputBits[15-i] === '1' ? 'active' : ''}`;
            }

            // Update sensor values
            const voltageVal = parseFloat(voltage).toFixed(2);
            const currentVal = parseFloat(current).toFixed(3);
            const powerVal = parseFloat(power).toFixed(2);

            document.getElementById('voltage').textContent = voltageVal + 'V';
            document.getElementById('current').textContent = currentVal + 'A';
            document.getElementById('power').textContent = powerVal + 'W';


            // Update status message
            document.getElementById('status').textContent = `Last update: ${new Date().toLocaleTimeString()}`;
        }

        function pollStatus() {
            fetch('/input')
                .then(response => response.text())
                .then(updateStatus)
                .catch(error => console.error('Error:', error));
        }

        // Initialize the interface
        createIOPoints();
        
        // Poll for updates every second
        setInterval(pollStatus, 1000);
        pollStatus(); // Initial poll
    </script>
</body>
</html>
)EOF";

void setupWiFi() {
    if (!WiFi.softAP(ssid, password)) {
        Serial.println("AP creation failed!");
        while (1);
    }
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

void setupI2C() {
    Wire.begin(PIN_SDA, PIN_SCL);
    
    if (!pcf21.begin() || !pcf22.begin() || !pcf24.begin() || !pcf25.begin()) {
        Serial.println("Error initializing PCF8574 devices");
    }

    if (powerSensor.begin()) {
        powerSensor.setMaxCurrentShunt(1.3, 0.100, true);
    } else {
        Serial.println("Error initializing INA226");
    }
}

void updateInputs() {
    uint8_t bank1 = pcf21.read8();
    uint8_t bank2 = pcf22.read8();
    ioState.inputs = (bank2 << 8) | bank1;
    
    if (ioState.lastInputs != ioState.inputs) {
        //Serial.printf("Inputs changed: 0x%04X\n", ioState.inputs);
        ioState.lastInputs = ioState.inputs;
    }
}

void updateOutputs() {
    uint8_t bank1 = ioState.outputs & 0xFF;
    uint8_t bank2 = ioState.outputs >> 8;
    
    if (ioState.lastOutputs != ioState.outputs) {
        //Serial.printf("Outputs changed: 0x%04X\n", ioState.outputs);
        pcf24.write8(bank1);
        pcf25.write8(bank2);
        ioState.lastOutputs = ioState.outputs;
    }
}

void updateSensors() {
    sensorData.voltage = powerSensor.getBusVoltage();
    sensorData.current = powerSensor.getCurrent();
    sensorData.power = powerSensor.getPower();
    
    //Serial.printf("Power: %.2fV, %.3fA, %.2fW\n", sensorData.voltage, sensorData.current, sensorData.power);
}

void handleClient(WiFiClient client) {
    String currentLine = "";
    int requestType = -1;
    
    while (client.connected()) {
        if (client.available()) {
            char c = client.read();
            if (c == '\n') {
                if (currentLine.length() == 0) {
                    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();

                    if (requestType == 0) {
                        // Send web page byte by byte from PROGMEM
                        int webPageLen = strlen_P(WEBPAGE);
                        for (int k = 0; k < webPageLen; k++) {
                            char myChar = pgm_read_byte_near(WEBPAGE + k);
                            client.print(myChar);
                        }
                    } 
                    else if (requestType == 1) {
                        // Handle output control response
                        client.printf("%d,%.2f,%.3f,%.2f", 
                            ioState.inputs, sensorData.voltage, sensorData.current, sensorData.power);
                    }
                    else if (requestType == 2) {
                        // Handle input status response
                        client.printf("%d,%.2f,%.3f,%.2f", 
                            ioState.inputs, sensorData.voltage, sensorData.current, sensorData.power);
                    }

                    client.println();
                    break;
                } else {
                    // Check request type
                    if (currentLine.startsWith("GET / ")) {
                        requestType = 0;  // Homepage request
                    }
                    else if (currentLine.startsWith("GET /output")) {
                        requestType = 1;  // Output control request
                        // Parse output control parameters
                        int ind1 = currentLine.indexOf('=');
                        int ind2 = currentLine.indexOf('&');
                        int ind3 = currentLine.indexOf(' ', ind2);
                        
                        int bitIndex = currentLine.substring(ind1+4, ind2).toInt();
                        int bitValue = currentLine.substring(ind2+5, ind3).toInt();
                        
                        if (bitValue) {
                            ioState.outputs |= (1 << bitIndex);
                        } else {
                            ioState.outputs &= ~(1 << bitIndex);
                        }
                    }
                    else if (currentLine.startsWith("GET /input")) {
                        requestType = 2;  // Input status request
                    }
                    currentLine = "";
                }
            }
            else if (c != '\r') {
                currentLine += c;
            }
        }
    }
    client.stop();
}

void setup() {
    Serial.begin(115200);
    setupWiFi();
    setupI2C();
    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    unsigned long currentMillis = millis();

    // Update inputs
    if (currentMillis - lastInputUpdate >= INPUT_LOOP_MS) {
        lastInputUpdate = currentMillis;
        updateInputs();
    }

    // Update outputs
    if (currentMillis - lastOutputUpdate >= OUTPUT_LOOP_MS) {
        lastOutputUpdate = currentMillis;
        updateOutputs();
    }

    // Update sensors
    if (currentMillis - lastSensorUpdate >= SENSOR_LOOP_MS) {
        lastSensorUpdate = currentMillis;
        updateSensors();
    }

    // Handle client connections
    if (client) {
        handleClient(client);
        client.stop();
    }
}