// Khai báo chân nối LED
#define ledPin 13 

void setup() {
  // Thiết lập chân LED là OUTPUT
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH); // Bật LED
  delay(1000);                // Chờ 1 giây
  digitalWrite(ledPin, LOW);  // Tắt LED
  delay(1000);                // Chờ 1 giây
}
