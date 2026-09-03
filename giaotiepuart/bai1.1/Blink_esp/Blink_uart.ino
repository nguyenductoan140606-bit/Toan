// Khai báo chân UART kết nối với Arduino Uno
#define RX_PIN 4  // Nối với chân 9 (TX) của Arduino Uno
#define TX_PIN 5  // Nối với chân 10 (RX) của Arduino Uno

// Khai báo 2 chân LED theo yêu cầu
#define LED1_PIN 12
#define LED2_PIN 13

void setup() {
  Serial.begin(115200); // Cổng Debug trên máy tính
  
  // Khởi tạo cổng Hardware Serial1 giao tiếp với Arduino Uno
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Cấu hình chân xuất tín hiệu cho 2 LED
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);

  // Mặc định ban đầu tắt cả 2 LED
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);

  Serial.println("ESP32-S3 San sang dieu khien LED 12 va 13!");
}

void loop() {
  // Nếu có dữ liệu gửi tới từ Arduino Uno qua Serial1
  if (Serial1.available()) {
    char c = Serial1.read();
    Serial.print("ESP32-S3 Nhan duoc: ");
    Serial.println(c);

    // Bật/tắt LED1 (GPIO 12) và LED2 (GPIO 13)
    if (c == '0') {
      digitalWrite(LED1_PIN, HIGH);
      Serial1.println("LED 12: ON");
      Serial.println("-> Bat LED chân 12");
    }
    else if (c == '1') {
      digitalWrite(LED1_PIN, LOW);
      Serial1.println("LED 12: OFF");
      Serial.println("-> Tat LED chân 12");
    }
    else if (c == '2') {
      digitalWrite(LED2_PIN, HIGH);
      Serial1.println("LED 13: ON");
      Serial.println("-> Bat LED chân 13");
    }
    else if (c == '3') {
      digitalWrite(LED2_PIN, LOW);
      Serial1.println("LED 13: OFF");
      Serial.println("-> Tat LED chân 13");
    }
    else {
      Serial1.println("Command khong hop le!");
    }
  }
}