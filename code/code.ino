#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>


#define BLYNK_TEMPLATE_ID "---------------------" // Put your Template ID here
#define BLYNK_TEMPLATE_NAME "--------------------------" // Put your template name here
#define BLYNK_AUTH_TOKEN "-----------------" // Put your Blynk Auth Token here

#define BLYNK_PRINT Serial

char auth[] = BLYNK_AUTH_TOKEN;
BlynkTimer timer;


#define TDS_SENSOR_PIN 27
#define VREF 3.3
#define SCOUNT 30
int analogBuffer[SCOUNT];
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 0;
float phValue = 0;

const char *ssid = "----"; // Enter your WiFi Name
const char *password = "----"; // Enter your WiFi Password

const int potPin = A0;
#define ONE_WIRE_BUS 4
const int relayPin = 18;

#define TDS_THRESHOLD 800
#define TMP_THRESHOLD 25
#define PH_THRESHOLD 7

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void myTimer()
{
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);

    phValue = readPH();

    analogBuffer[analogBufferIndex] = analogRead(TDS_SENSOR_PIN);
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
    {
        analogBufferIndex = 0;
    }

    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
    {
        analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0;

        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
        float compensationVoltage = averageVoltage / compensationCoefficient;

        tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
    }

    Blynk.virtualWrite(V4, temperature);
    Blynk.virtualWrite(V0, phValue);
    Blynk.virtualWrite(V27, tdsValue);
}

void setup()
{
    Serial.begin(115200);
    pinMode(relayPin, OUTPUT);
    pinMode(potPin, INPUT);
    pinMode(TDS_SENSOR_PIN, INPUT);
    delay(1000);
    sensors.begin();
    Blynk.begin(auth, ssid, password);
    timer.setInterval(1000L, myTimer);
}

void loop()
{
    Blynk.run();
    timer.run();
    sensors.requestTemperatures();
    temperature = sensors.getTempCByIndex(0);

    phValue = readPH();

    static unsigned long analogSampleTimepoint = millis();
    if (millis() - analogSampleTimepoint > 40U)
    {
        analogSampleTimepoint = millis();
        analogBuffer[analogBufferIndex] = analogRead(TDS_SENSOR_PIN);
        analogBufferIndex++;
        if (analogBufferIndex == SCOUNT)
        {
            analogBufferIndex = 0;
        }
    }

    static unsigned long printTimepoint = millis();
    if (millis() - printTimepoint > 800U)
    {
        printTimepoint = millis();
        for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
        {
            analogBufferTemp[copyIndex] = analogBuffer[copyIndex];

            averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0;

            float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
            float compensationVoltage = averageVoltage / compensationCoefficient;

            tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage - 255.86 * compensationVoltage * compensationVoltage + 857.39 * compensationVoltage) * 0.5;
        }
    }

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print("°C | ph: ");
    Serial.print(phValue);
    Serial.print(" | TDS:");
    Serial.print(tdsValue);
    Serial.println(" ppm");

    Blynk.run();
    timer.run();

    if (tdsValue > TDS_THRESHOLD || temperature > TMP_THRESHOLD || phValue > PH_THRESHOLD)
    {
        digitalWrite(relayPin, LOW);
        Serial.println("Opened the Valve : Water quality exceeds threshold emptying the container");
        if (tdsValue > TDS_THRESHOLD)
        {
            Blynk.logEvent("alert_water_quality", String("TDS Crossed the Threshold Value! TDS: ") + tdsValue);
        }
        else if (temperature > TMP_THRESHOLD)
        {
            Blynk.logEvent("alert_water_quality", String("Temperature Crossed the Threshold Value! Temp: ") + temperature + String(" °C"));
        }
    }
    else
    {
        digitalWrite(relayPin, HIGH);
    }
}

float readPH()
{
    float Value = analogRead(potPin);
    float voltage = Value * (3.3 / 4095.0);
    float ph = (3.3 * voltage);
    delay(500);
    return ph;
}

int getMedianNum(int bArray[], int iFilterLen)
{
    int bTab[iFilterLen];
    for (byte i = 0; i < iFilterLen; i++)
        bTab[i] = bArray[i];
    int i, j, bTemp;
    for (j = 0; j < iFilterLen - 1; j++)
    {
        for (i = 0; i < iFilterLen - j - 1; i++)
        {
            if (bTab[i] > bTab[i + 1])
            {
                bTemp = bTab[i];
                bTab[i] = bTab[i + 1];
                bTab[i + 1] = bTemp;
            }
        }
    }
    if ((iFilterLen & 1) > 0)
    {
        bTemp = bTab[(iFilterLen - 1) / 2];
    }
    else
    {
        bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
    }
    return bTemp;
}