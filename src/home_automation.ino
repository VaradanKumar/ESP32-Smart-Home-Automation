/*
 * Smart Home Automation - ESP32
 *
 * Hardware/pin mapping from the project report:
 *   PIR sensor       -> GPIO 27
 *   DHT11 data       -> GPIO 4
 *   LDR digital out  -> GPIO 34
 *   Light indicator  -> GPIO 25
 *   Fan indicator    -> GPIO 26
 *   Door servo       -> GPIO 14
 *
 * Configure your own Wi-Fi credentials and Blynk auth token before use.
 */

#define BLYNK_TEMPLATE_ID   "TMPL3LTZ4jK1X"
#define BLYNK_TEMPLATE_NAME "Home Automation"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "DHT.h"
#include <ESP32Servo.h>

char ssid[] = "WIFI_NAME";
char pass[] = "WIFI_PASSWORD";

#define PIR_PIN    27
#define DHT_PIN    4
#define LDR_PIN    34
#define LIGHT_LED  25
#define FAN_LED    26
#define SERVO_PIN  14
#define DHT_TYPE   DHT11

DHT dht(DHT_PIN, DHT_TYPE);
Servo doorServo;
BlynkTimer timer;

// Blynk override switches
bool lightOverride = false;
bool fanOverride = false;
bool doorManualOpen = false;

// Automatic states
bool fanState = false;
bool lightState = false;
bool doorState = false;

// Timers
unsigned long lastMotionTime = 0;
unsigned long doorOpenTime = 0;

BLYNK_WRITE(V4)
{
    lightOverride = param.asInt();

    if (lightOverride) {
        lightState = true;
        digitalWrite(LIGHT_LED, HIGH);
        Serial.println("Blynk: Light FORCED ON");
    } else {
        Serial.println("Blynk: Light returned to AUTO");
    }
}

BLYNK_WRITE(V5)
{
    fanOverride = param.asInt();

    if (fanOverride) {
        fanState = true;
        digitalWrite(FAN_LED, HIGH);
        Serial.println("Blynk: Fan FORCED ON");
    } else {
        Serial.println("Blynk: Fan returned to AUTO");
    }
}

BLYNK_WRITE(V6)
{
    doorManualOpen = param.asInt();

    if (doorManualOpen) {
        doorState = true;
        doorServo.write(90);
        Serial.println("Blynk: Door OPEN");
    } else {
        doorState = false;
        doorServo.write(0);
        Serial.println("Blynk: Door CLOSED");
    }
}

void sendSensorData()
{
    const int motion = digitalRead(PIR_PIN);
    const int light = digitalRead(LDR_PIN);

    const float temperature = dht.readTemperature();
    const float humidity = dht.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
        Serial.println("DHT11 reading failed!");
        return;
    }

    if (motion == HIGH) {
        lastMotionTime = millis();
    }

    // Automatic light: dark + motion; switch off 30 s after last motion.
    if (!lightOverride) {
        if (motion == HIGH && light == HIGH) {
            lightState = true;
        }

        if (lightState && (millis() - lastMotionTime >= 30000UL)) {
            lightState = false;
        }

        digitalWrite(LIGHT_LED, lightState ? HIGH : LOW);
    }

    // Fan hysteresis: ON at >=30 C, OFF at <=28 C.
    // Between 28 C and 30 C, retain the previous state.
    if (!fanOverride) {
        if (temperature >= 30.0f) {
            fanState = true;
        } else if (temperature <= 28.0f) {
            fanState = false;
        }

        digitalWrite(FAN_LED, fanState ? HIGH : LOW);
    }

    // Automatic door: open on motion, close after 5 seconds.
    if (!doorManualOpen) {
        if (motion == HIGH && !doorState) {
            doorState = true;
            doorOpenTime = millis();
            doorServo.write(90);
            Serial.println("Automatic Door: OPEN");
        }

        if (doorState && (millis() - doorOpenTime >= 5000UL)) {
            doorState = false;
            doorServo.write(0);
            Serial.println("Automatic Door: CLOSED");
        }
    }

    // Blynk dashboard telemetry and state.
    Blynk.virtualWrite(V0, temperature);
    Blynk.virtualWrite(V1, humidity);
    Blynk.virtualWrite(V2, motion);
    Blynk.virtualWrite(V3, light);
    Blynk.virtualWrite(V4, lightState);
    Blynk.virtualWrite(V5, fanState);
    Blynk.virtualWrite(V6, doorState);

    Serial.println();
    Serial.print("Motion: ");
    Serial.println(motion ? "DETECTED" : "NOT DETECTED");
    Serial.print("Room: ");
    Serial.println(light ? "DARK" : "BRIGHT");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.print("Light: ");
    Serial.println(lightState ? "ON" : "OFF");
    Serial.print("Fan: ");
    Serial.println(fanState ? "ON" : "OFF");
    Serial.print("Door: ");
    Serial.println(doorState ? "OPEN" : "CLOSED");
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIR_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(LIGHT_LED, OUTPUT);
    pinMode(FAN_LED, OUTPUT);

    digitalWrite(LIGHT_LED, LOW);
    digitalWrite(FAN_LED, LOW);

    dht.begin();

    doorServo.setPeriodHertz(50);
    doorServo.attach(SERVO_PIN, 500, 2400);
    doorServo.write(0);

    Serial.println();
    Serial.println("================================");
    Serial.println("   HOME AUTOMATION");
    Serial.println("================================");
    Serial.println("PIR warming up...");
    delay(30000);
    Serial.println("PIR Ready");

    Serial.println("Connecting to Blynk...");
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
    Serial.println("Blynk Connected");

    // Read sensors every 2 seconds.
    timer.setInterval(2000L, sendSensorData);
}

void loop()
{
    Blynk.run();
    timer.run();
}
