# Smart Home Automation using ESP32

An ESP32-based home automation prototype that reads temperature/humidity, motion and light-state inputs, and controls light/fan indicator LEDs plus a servo-operated door model. The project also reports telemetry and accepts manual override commands through a Blynk IoT dashboard.

## Features implemented in the project

- DHT11 temperature and humidity monitoring.
- PIR motion detection.
- LDR-based room brightness detection.
- Automatic light logic: turn on when the room is dark and motion is detected; turn off 30 seconds after the last motion.
- Fan control with hysteresis: ON at or above 30 °C, OFF at or below 28 °C, and retain the previous state between those thresholds.
- Servo door model: open to 90° on motion and close after five seconds.
- Blynk dashboard telemetry for sensor readings and device states.
- Blynk manual override inputs for light, fan and door.

## Hardware and pin mapping

| Component | ESP32 pin |
|---|---:|
| PIR output | GPIO 27 |
| DHT11 data | GPIO 4 |
| LDR digital output | GPIO 34 |
| Light LED | GPIO 25 |
| Fan LED | GPIO 26 |
| Servo signal | GPIO 14 |

The light logic assumes the LDR module's digital output is HIGH in darkness. Reverse the condition if your module uses the opposite polarity.

## Software

- Arduino IDE / ESP32 Arduino core
- Libraries: Blynk, DHT sensor library, ESP32Servo
- Source: `src/home_automation.ino`

## Setup

1. Install the ESP32 board support package and the libraries listed above.
2. Create/configure a Blynk template and device for the project.
3. Replace `YOUR_AUTH_TOKEN`, `WIFI_NAME` and `WIFI_PASSWORD` in the sketch with your own values. Do not commit real credentials or tokens to a public repository.
4. Check all pin connections and the servo's power requirements before energizing the setup.
5. Flash the sketch and monitor output at 115200 baud.

The sketch includes a 30-second PIR warm-up delay and sends sensor data to Blynk every two seconds.

## Notes

The LEDs represent the light and fan outputs in the prototype; they are not mains appliance drivers. Use properly rated, isolated driver circuitry for real loads. Never connect an ESP32 GPIO directly to mains equipment.

## Project evidence

Add your own hardware photographs and dashboard screenshots under `docs/images/` if you wish to publish them.
