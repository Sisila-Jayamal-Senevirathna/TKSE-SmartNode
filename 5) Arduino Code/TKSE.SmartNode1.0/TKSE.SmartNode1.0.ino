//ESP32 Dev Module

int statusLed = 2;          // D2
byte sensorInterrupt = 18;  // D18
unsigned long oldTime = 0;
bool pulseCount = 0;  // To store pulse counts
bool SensorStatus = 0;
bool SensorStatusBefore = 0;

void setup() {
  Serial.begin(9600);
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, LOW);
  attachInterrupt(digitalPinToInterrupt(sensorInterrupt), pulseCounter, FALLING);
}

void loop() {
  if ((millis() - oldTime) > 1000) {  // Only process counters once per second
    oldTime = millis();

    if ((pulseCount == 1)) {
      pulseCount = 0;  // Reset the pulse count
      SensorStatus = 1;
    } else if ((pulseCount == 0)) {
      SensorStatus = 0;
    }
  }

  if (SensorStatus == !SensorStatusBefore) {
    if (SensorStatus == 1) {
      Serial.println(SensorStatus);
      digitalWrite(statusLed, HIGH);
    } else {
      Serial.println(SensorStatus);
      digitalWrite(statusLed, LOW);
    }
    SensorStatusBefore = SensorStatus;
  }
}

void pulseCounter() {
  pulseCount = 1;  // Increment pulse count on interrupt
}
