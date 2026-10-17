
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// Dat 1 khi mo phong lan dau de ep ghi gio vao DS1307.
// Chay dung roi thi doi ve 0 va bien dich lai.
#define FORCE_SET_TIME 0

#define BTN_MODE 4
#define BTN_UP   5
#define BTN_DOWN 6
#define BTN_SET  7
#define PIN_LED  8
#define PIN_BUZZ 9

#define LED_ON    LOW      // active low
#define LED_OFF   HIGH
#define BUZZ_IDLE HIGH

LiquidCrystal_I2C lcd(0x20, 16, 2);   // 0x20 cho Proteus, 0x27 cho module that
RTC_DS1307 rtc;

byte mode = 0;        // 0 xem gio, 1 cai bao thuc, 2 cai thoi gian
byte step = 0;        // buoc dang chinh
bool ringing = false;

byte alarmH = 6, alarmM = 30;
bool alarmOn = true;

byte tH, tM, tS;      // gia tri tam khi chinh sua

DateTime now;
unsigned long tRead = 0, tDraw = 0, tRing = 0, tHold = 0;
int lastKey = -1;

byte pins[4] = {BTN_MODE, BTN_UP, BTN_DOWN, BTN_SET};
bool prevState[4] = {HIGH, HIGH, HIGH, HIGH};
unsigned long tChange[4] = {0, 0, 0, 0};
bool holdDone = false;

const char *NAME_ALARM[2] = {"Gio ", "Phut"};
const char *NAME_TIME[3]  = {"Gio ", "Phut", "Giay"};

/* -------- kiem tra du lieu doc tu DS1307 co hop le khong -------- */
bool timeValid(DateTime t) {
  return t.year() >= 2020 && t.year() <= 2099 &&
         t.month() >= 1 && t.month() <= 12 &&
         t.day() >= 1 && t.day() <= 31 &&
         t.hour() < 24 && t.minute() < 60 && t.second() < 60;
}

/* -------- EEPROM -------- */
void saveAlarm() {
  EEPROM.update(0, 0x5A);
  EEPROM.update(1, alarmH);
  EEPROM.update(2, alarmM);
  EEPROM.update(3, alarmOn);
}

void loadAlarm() {
  if (EEPROM.read(0) == 0x5A) {
    alarmH  = EEPROM.read(1);
    alarmM  = EEPROM.read(2);
    alarmOn = EEPROM.read(3);
  }
  if (alarmH > 23) alarmH = 6;
  if (alarmM > 59) alarmM = 30;
}

/* -------- doc nut, tra ve true mot lan khi nha nut -------- */
bool released(byte i) {
  bool s = digitalRead(pins[i]);
  if (s == prevState[i]) return false;
  if (millis() - tChange[i] < 30) return false;
  tChange[i] = millis();
  prevState[i] = s;
  return (s == HIGH);
}

/* -------- in mot dong day du 16 ky tu -------- */
void showLine(byte row, const char *s) {
  lcd.setCursor(0, row);
  byte n = 0;
  while (s[n] && n < 16) { lcd.print(s[n]); n++; }
  while (n < 16) { lcd.print(' '); n++; }
}

/* -------- SETUP -------- */
void setup() {
  for (byte i = 0; i < 4; i++) pinMode(pins[i], INPUT_PULLUP);
  pinMode(PIN_LED,  OUTPUT); digitalWrite(PIN_LED,  LED_OFF);
  pinMode(PIN_BUZZ, OUTPUT); digitalWrite(PIN_BUZZ, BUZZ_IDLE);

  lcd.init();
  lcd.backlight();
  rtc.begin();

  now = rtc.now();
  if (FORCE_SET_TIME || !rtc.isrunning() || !timeValid(now)) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    now = rtc.now();
  }

  loadAlarm();
}

/* -------- LOOP -------- */
void loop() {
  char buf[17];

  // 1. doc thoi gian moi 500 ms
  if (millis() - tRead >= 500) {
    tRead = millis();
    DateTime t = rtc.now();
    if (timeValid(t)) now = t;
  }

  // 2. den gio bao thuc chua
  int key = now.hour() * 60 + now.minute();
  if (key != lastKey) {
    lastKey = key;
    if (alarmOn && !ringing && now.hour() == alarmH && now.minute() == alarmM) {
      ringing = true;
      tRing = millis();
      tone(PIN_BUZZ, 2500);
      digitalWrite(PIN_LED, LED_ON);
    }
  }

  // 3. dang keu: nhan nut bat ky de tat, hoac tu tat sau 30 giay
  if (ringing) {
    bool any = false;
    for (byte i = 0; i < 4; i++) if (released(i)) any = true;

    if (any || millis() - tRing >= 30000) {
      ringing = false;
      noTone(PIN_BUZZ);
      digitalWrite(PIN_BUZZ, BUZZ_IDLE);
      digitalWrite(PIN_LED, LED_OFF);
    }
  }

  else {
    // 4. nut MODE
    if (digitalRead(BTN_MODE) == LOW) {
      if (prevState[0] == HIGH && millis() - tChange[0] > 30) {
        tChange[0] = millis();
        prevState[0] = LOW;
        tHold = millis();
        holdDone = false;
      }
      if (!holdDone && millis() - tHold >= 2000) {   // giu 2 giay
        holdDone = true;
        alarmOn = !alarmOn;
        saveAlarm();
        mode = 0;
      }
    }
    else if (released(0) && !holdDone) {             // nhan ngan: doi che do
      mode = (mode + 1) % 3;
      step = 0;
      if (mode == 1) { tH = alarmH; tM = alarmM; }
      if (mode == 2) { tH = now.hour(); tM = now.minute(); tS = now.second(); }
    }

    // 5. UP / DOWN / SET
    int d = 0;
    if (released(1)) d = 1;
    if (released(2)) d = -1;
    bool set = released(3);

    if (mode == 1) {                                 // cai bao thuc
      if (d) {
        if (step == 0) tH = (tH + 24 + d) % 24;
        else           tM = (tM + 60 + d) % 60;
      }
      if (set) {
        if (step == 0) step = 1;
        else {
          alarmH = tH;
          alarmM = tM;
          saveAlarm();
          lastKey = -1;
          mode = 0;
        }
      }
    }
    else if (mode == 2) {                            // cai thoi gian
      if (d) {
        if (step == 0)      tH = (tH + 24 + d) % 24;
        else if (step == 1) tM = (tM + 60 + d) % 60;
        else                tS = (tS + 60 + d) % 60;
      }
      if (set) {
        if (step < 2) step++;
        else {
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), tH, tM, tS));
          lastKey = -1;
          mode = 0;
        }
      }
    }
  }

  // 6. ve man hinh 5 lan moi giay
  if (millis() - tDraw >= 200) {
    tDraw = millis();

    if (ringing) {
      showLine(0, "*** BAO THUC ***");
      showLine(1, "Nhan nut de tat");
    }
    else if (mode == 0) {
      sprintf(buf, "%02d:%02d:%02d  %s",
              now.hour(), now.minute(), now.second(),
              alarmOn ? "AL ON" : "AL OFF");
      showLine(0, buf);
      sprintf(buf, "%02d/%02d/%04d %02d:%02d",
              now.day(), now.month(), now.year(), alarmH, alarmM);
      showLine(1, buf);
    }
    else if (mode == 1) {
      showLine(0, "CAI BAO THUC");
      sprintf(buf, "%s   %02d:%02d", NAME_ALARM[step], tH, tM);
      showLine(1, buf);
    }
    else {
      showLine(0, "CAI THOI GIAN");
      sprintf(buf, "%s %02d:%02d:%02d", NAME_TIME[step], tH, tM, tS);
      showLine(1, buf);
    }
  }
}