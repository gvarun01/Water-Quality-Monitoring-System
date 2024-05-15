const int relay = 27;

void setup()
{
    Serial.begin(115200);
    pinMode(relay, OUTPUT);
}

void loop()
{
    // Normally Open configuration, send LOW signal to let current flow
    digitalWrite(relay, LOW);
    Serial.println("Current Flowing");
    delay(1000);

    // Normally Open configuration, send HIGH signal stop current flow
    digitalWrite(relay, HIGH);
    Serial.println("Current not Flowing");
    delay(5000);
}