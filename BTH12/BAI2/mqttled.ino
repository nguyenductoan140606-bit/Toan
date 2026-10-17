#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ===== WIFI =====
const char* ssid = "Duy";
const char* password = "20202020";

// ===== MQTT =====
const char* mqtt_server = "192.168.179.27";
const int mqtt_port = 1883;
const char* mqtt_user = "toan";
const char* mqtt_pass = "toan";

const char* command_topic = "iot/lab1/10124274/rgb/set";
const char* state_topic = "iot/lab1/10124274/rgb/state";
const char* status_topic = "iot/lab1/10124274/rgb/status";

// ===== LED RGB TREN BOARD =====
#ifdef RGB_BUILTIN
const int RGB_PIN = RGB_BUILTIN;
#else
const int RGB_PIN = 48;
#endif

WiFiClient espClient;
PubSubClient client(espClient);

String clientId;

bool ledOn = false;
int red = 255;
int green = 0;
int blue = 0;
int brightness = 64;

unsigned long lastReconnect = 0;

// Cap nhat LED theo mau va do sang
void updateLed() {
  if (!ledOn) {
    rgbLedWrite(RGB_PIN, 0, 0, 0);
    return;
  }

  rgbLedWrite(
    RGB_PIN,
    red * brightness / 255,
    green * brightness / 255,
    blue * brightness / 255
  );
}

// Gui trang thai ve Home Assistant
void publishState() {
  StaticJsonDocument<256> doc;

  doc["state"] = ledOn ? "ON" : "OFF";
  doc["brightness"] = brightness;
  doc["color_mode"] = "rgb";

  JsonObject color = doc.createNestedObject("color");
  color["r"] = red;
  color["g"] = green;
  color["b"] = blue;

  char payload[256];
  serializeJson(doc, payload, sizeof(payload));

  if (client.publish(state_topic, payload, true)) {
    Serial.print("Da gui trang thai: ");
    Serial.println(payload);
  } else {
    Serial.println("Gui trang thai that bai");
  }
}

// Xu ly lenh tu Home Assistant
void callback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, command_topic) != 0) {
    return;
  }

  Serial.print("Nhan lenh: ");
  Serial.write(payload, length);
  Serial.println();

  StaticJsonDocument<256> doc;
  DeserializationError error =
    deserializeJson(doc, payload, length);

  if (error) {
    Serial.print("Loi JSON: ");
    Serial.println(error.c_str());
    return;
  }

  if (doc.containsKey("state")) {
    const char* state = doc["state"] | "";

    if (strcmp(state, "ON") == 0) {
      ledOn = true;
    } else if (strcmp(state, "OFF") == 0) {
      ledOn = false;
    }
  }

  if (doc.containsKey("brightness")) {
    brightness = constrain(
      doc["brightness"].as<int>(), 0, 255
    );
  }

  if (doc.containsKey("color")) {
    JsonObject color = doc["color"];

    if (color.containsKey("r")) {
      red = constrain(color["r"].as<int>(), 0, 255);
    }

    if (color.containsKey("g")) {
      green = constrain(color["g"].as<int>(), 0, 255);
    }

    if (color.containsKey("b")) {
      blue = constrain(color["b"].as<int>(), 0, 255);
    }
  }

  updateLed();
  publishState();

  Serial.print("LED: ");
  Serial.println(ledOn ? "BAT" : "TAT");
}

void connectMqtt() {
  Serial.print("Dang ket noi MQTT: ");
  Serial.println(mqtt_server);

  // Broker gui "offline" neu ESP mat ket noi
bool connected = client.connect(
    clientId.c_str(),
    mqtt_user,
    mqtt_pass,
    status_topic,
    1,
    true,
    "offline"
  );

  if (connected) {
    Serial.println("MQTT connected");

    if (client.subscribe(command_topic, 1)) {
      Serial.print("Dang nghe: ");
      Serial.println(command_topic);
    } else {
      Serial.println("Dang ky topic that bai");
    }

    publishState();
    client.publish(status_topic, "online", true);
  } else {
    Serial.print("MQTT failed, rc=");
    Serial.println(client.state());
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Tat LED khi khoi dong
  updateLed();

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);

  Serial.print("Dang ket noi Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi da ket noi");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  clientId = "ESP32S3-RGB-10124274-" + WiFi.macAddress();
  clientId.replace(":", "");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(512);

  lastReconnect = millis();
  connectMqtt();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(10);
    return;
  }

  if (!client.connected()) {
    unsigned long now = millis();

    if (now - lastReconnect >= 5000) {
      lastReconnect = now;
      connectMqtt();
    }
  } else {
    client.loop();
  }

  delay(2);
}
