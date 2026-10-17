
#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "Duy";
const char* password = "20202020";

IPAddress serverIP(192, 168, 179, 33);
const uint16_t serverPort = 5683;

WiFiUDP udp;
uint16_t messageId = 1;
unsigned long lastSend = 0;

void sendCoAP() {
  const char* payload = "temp=30.5";
  const char* path1 = "sensor";
  const char* path2 = "temp";

  uint8_t packet[128];
  int pos = 0;

  // CoAP version 1, NON, token length 0
  packet[pos++] = 0x50;

  // POST
  packet[pos++] = 0x02;

  // Message ID
  packet[pos++] = messageId >> 8;
  packet[pos++] = messageId & 0xFF;
  messageId++;

  // Uri-Path: sensor (option 11)
  packet[pos++] = 0xB6;
  memcpy(packet + pos, path1, 6);
  pos += 6;

  // Uri-Path: temp (option 11, delta 0)
  packet[pos++] = 0x04;
  memcpy(packet + pos, path2, 4);
  pos += 4;

  // Content-Format: text/plain (option 12)
  packet[pos++] = 0x11;
  packet[pos++] = 0x00;

  // Payload marker
  packet[pos++] = 0xFF;

  memcpy(packet + pos, payload, strlen(payload));
  pos += strlen(payload);

  udp.beginPacket(serverIP, serverPort);
  udp.write(packet, pos);
  int result = udp.endPacket();

  Serial.print("Gui CoAP NON POST: ");
  Serial.println(payload);
  Serial.print("UDP send result: ");
  Serial.println(result);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Dang ket noi WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("ESP32-S3 IP: ");
  Serial.println(WiFi.localIP());

  udp.begin(5684);

  Serial.println("CoAP Client Started!");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED &&
      millis() - lastSend >= 5000) {
    lastSend = millis();
    sendCoAP();
  }
}
