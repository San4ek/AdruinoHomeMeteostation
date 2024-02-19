#include <avr/io.h>
#include <LiquidCrystal.h>
#include <GyverBME280.h>

GyverBME280 bme;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#define BTN_RIGHT 0
#define BTN_UP 1
#define BTN_DOWN 2

#define BTN_VALUES (short[]){100,200,400}

#define NUMB_OF_FUNCTIONS 3

#define TEMPERATURE_FUN 0
#define PRESSURE_FUN 1
#define HUMIDITY_FUN 2

#define BME_ADDRESS 0x76

#define TEMPERATURE_STR "Temp: "
#define PRESSURE_STR "Press: "
#define HUMIDITY_STR "Humidity: "

#define WELCOME "    Welcome!    "
#define MEASURING "  Measuring...  "
#define ERROR "     Error!     "
#define CHECK "  Check BME280  "
#define EMPTY_STR "                "
#define WAIT "  Please wait! "
#define SECONDS "s"
#define UPDATE "Update after "

#define UNITS (String[]){"C","mm","%"}

#define UPDATE_TIME 60

#define PIN_A0 0

#define BUTTONS_PIN A0

#define INCORRECT_VALUE 0

byte BEFORE_UPDATE = UPDATE_TIME;
bool BUTTON_STATE = 0;
byte CURRENT_FUN = 0;
bool UPDATED=0;
bool isWork=false;

float lastTemp=INCORRECT_VALUE;
float lastHum=INCORRECT_VALUE;
float lastPress=INCORRECT_VALUE;

void setup() {
  if (initAll()) {
    isWork=true;

    printHelloMessage();

    sei();
  } else {
    printErrorMessage();
  }
}

void initValues() {
  lastTemp=getTempMeasurement();
  lastHum=getHumidityMeasurement();
  lastPress=getPressureMeasurement();
}

float getHumidityMeasurement() {
  return bme.readHumidity();
}

float getTempMeasurement() {
  return bme.readTemperature();
}

float getPressureMeasurement() {
  return pressureToMmHg(bme.readPressure());
}

void printTempMeasurement(float temp) {
  updateLine(0, TEMPERATURE_STR + String(temp) + UNITS[TEMPERATURE_FUN]);
}

void printPressureMeasurement(float pressure) {
  updateLine(0, PRESSURE_STR + String(pressure)+UNITS[PRESSURE_FUN]);
}

void printHumidityMeasurement(float humidity) {
  updateLine(0, HUMIDITY_STR + String(humidity) + UNITS[HUMIDITY_FUN]);
}

void printWaitMessage() {
  updateLine(0, MEASURING);
  updateLine(1, WAIT);
}

void printErrorMessage() {
  lcd.setCursor(0, 0);
  lcd.print(ERROR);
  lcd.setCursor(0, 1);
  lcd.print(CHECK);
}

bool initBme() {
  return bme.begin(BME_ADDRESS);
}

bool initAll() {
  Serial.begin(115200);
  lcd.begin(16, 2);

  initTimer1();
  initPCINT1();

  return initBme();
}

void printHelloMessage() {
  updateLine(0, WELCOME);
}

void updateLine(short line, String message) {
  lcd.setCursor(0, line);
  lcd.print(EMPTY_STR);
  lcd.setCursor(0, line);
  lcd.print(message);
}

void printMeasurement() {
    switch (CURRENT_FUN) {
      case TEMPERATURE_FUN:
        printTempMeasurement(lastTemp);
        break;
      case PRESSURE_FUN:
        printPressureMeasurement(lastPress);
        break;
      case HUMIDITY_FUN:
        printHumidityMeasurement(lastHum);
        break;
    }
}

void measure() {
  printWaitMessage();
  initValues();
}

void reverseUpdate() {
  UPDATED^=1;
}

ISR(TIMER1_COMPA_vect) {
  if (BEFORE_UPDATE) {
    updateLine(1, UPDATE + String(BEFORE_UPDATE--) + SECONDS);
  } else {
    reverseUpdate();
    restartTimer1();
  }
}

ISR(PCINT1_vect) {
  reversButtonState();
  if (BUTTON_STATE) {
    detectPosition();
    reverseUpdate();
    restartTimer1();
  }
}

void reversButtonState() {
  BUTTON_STATE ^= 1;
}

void loop() {
  if (Serial.available()) {

    String com=Serial.readString();

    com.trim();

    if (com=="/temperature") {
      waitForWrite();

      Serial.print(String(lastTemp));
    }

    if (com=="/pressure") {
      waitForWrite();

      Serial.print(String(lastPress));
    }

    if (com=="/humidity") {
      waitForWrite();

      Serial.print(String(lastHum));
    }
  }

  if (!UPDATED){
    if (isWork || initBme()) {
      isWork=true;
      reverseUpdate();
      measure();
    }
    if (lastHum==INCORRECT_VALUE || lastPress==INCORRECT_VALUE || lastTemp==INCORRECT_VALUE) {
      isWork=false;
      printErrorMessage();
    } else {
      printMeasurement();
    }
  }
}

void waitForWrite() {
  while (Serial.availableForWrite()==0);
}

void initTimer1() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B |= _BV(WGM12);
  TCCR1B |= _BV(CS12) | _BV(CS10);
  TIMSK1 |= _BV(OCIE1A);
  OCR1AH = 0b00111101;
  OCR1AL = 0b00001001;
}

void initPCINT1() {
  pinMode(BUTTONS_PIN, INPUT_PULLUP);
  PCICR = 0b00000010;
  PCMSK1 = _BV(PIN_A0);
}

void detectPosition() {
  short keyAnalog = analogRead(PIN_A0);
  if (keyAnalog < BTN_VALUES[BTN_RIGHT]) {
    
  } else if (keyAnalog < BTN_VALUES[BTN_UP]) {
    checkUpPosition();
  } else if (keyAnalog < BTN_VALUES[BTN_DOWN]) {
    checkDownPosition();
  }
}

void checkUpPosition() {
  if (CURRENT_FUN != TEMPERATURE_FUN) {
    --CURRENT_FUN;
  } else {
    CURRENT_FUN = HUMIDITY_FUN;
  }
}

void checkDownPosition() {
  if (CURRENT_FUN != HUMIDITY_FUN) {
    ++CURRENT_FUN;
  } else {
    CURRENT_FUN = TEMPERATURE_FUN;
  }
}

void restartTimer1() {
  BEFORE_UPDATE = UPDATE_TIME;
  TCNT1=0;
}