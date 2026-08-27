// Khai báo chân LED
#define LED1 13
#define LED2 12
#define LED3 11
#define LED4 10
#define LED5 9
#define LED6 8
#define LED7 7
#define LED8 6
int leds[] = {LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8};

void setup() {
  // Thiết lập tất cả chân là OUTPUT
  for (int i = 0; i < 8; i++) {
    pinMode(leds[i], OUTPUT);
  }
}

void loop() {
  // Chạy lần lượt từ LED1 đến LED8
  for (int i = 0; i < 8; i++) {
    digitalWrite(leds[i], HIGH);  // bật LED
    delay(200);                   // chờ 0.2 giây
    digitalWrite(leds[i], LOW);   // tắt LED
  }
}