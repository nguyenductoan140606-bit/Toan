#define LED 2
 void setup() {
pinMode (LED, OUTPUT); 3
 void loop() {
digitalWrite(LED, HIGH);
delay(1000);
digitalWrite(LED, LOW);
 delay(1000);
 }