#define BUZZER_PIN 25
#define VIBRATOR_PIN 26

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(VIBRATOR_PIN, OUTPUT);
}

void loop() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);

  digitalWrite(VIBRATOR_PIN, HIGH);
  delay(500);
  digitalWrite(VIBRATOR_PIN, LOW);

  delay(1000);
}
