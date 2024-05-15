const int potPin = A0;
const int numReadings = 10; // Number of readings to average
float pH;
float sum = 0;

void setup()
{
    Serial.begin(115200);
    pinMode(potPin, INPUT);
    delay(1000);
}

void loop()
{
    sum = 0;
    for (int i = 0; i < numReadings; i++)
    {
        int value = analogRead(potPin);
        float voltage = value * (3.3 / 4095.0);
        pH = 3.3 * voltage;
        sum += pH;
        delay(10);
    }
    float averagepH = sum / numReadings;

    Serial.print("Average pH Value: ");
    Serial.println(averagepH);

    delay(1000);
}