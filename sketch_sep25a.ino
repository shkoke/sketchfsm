//Пины
const int RED = 13;
const int YELLOW = 12;
const int GREEN = 11;
const int BUTTON = 2;

//Состояния
enum State { S_GREEN, S_YELLOW, S_RED, S_WARNING, S_NIGHT };
State currentState = S_GREEN;

//Переменные таймера
unsigned long stateStart = 0;
unsigned long duration = 0;

//Флаги
bool pedRequest = false;
bool emergency = false;
bool nightMode = false;

//Антидребезг
unsigned long lastButtonChange = 0;
bool lastButtonState = HIGH;

//Для длинного и двойного нажатия
unsigned long pressStart = 0;
unsigned long lastPressTime = 0;
bool waitingSecondPress = false;

//Функции
void setOutputsForState(State s) {
  //Сначала всё выключим
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);

  switch (s) {
    case S_GREEN:
      digitalWrite(GREEN, HIGH);
      Serial.println("State: GREEN");
      break;
    case S_YELLOW:
      digitalWrite(YELLOW, HIGH);
      Serial.println("State: YELLOW");
      break;
    case S_RED:
      digitalWrite(RED, HIGH);
      Serial.println("State: RED");
      break;
    case S_WARNING:
      Serial.println("State: WARNING (blinking yellow)");
      break;
    case S_NIGHT:
      Serial.println("State: NIGHT (blinking yellow 1s)");
      break;
  }
}

void goToState(State s, unsigned long dur) {
  currentState = s;
  stateStart = millis();
  duration = dur;
  setOutputsForState(s);
}

void setup() {
  pinMode(RED, OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  Serial.begin(9600);

  goToState(S_GREEN, 10000);
}

void readInputs() {
  bool btn = digitalRead(BUTTON);

  if (btn != lastButtonState && millis() - lastButtonChange > 50) {
    lastButtonChange = millis();
    lastButtonState = btn;

    //Нажатие
    if (btn == LOW) {
      pressStart = millis();
    }

    //Отпускание
    if (btn == HIGH && pressStart > 0) {
      unsigned long pressTime = millis() - pressStart;

      //Длинное нажатие → переключение ночного режима
      if (pressTime >= 2000) {
        nightMode = !nightMode;
        if (nightMode) goToState(S_NIGHT, 1000);
        else goToState(S_GREEN, 10000);
        Serial.println("Night mode toggled");
      }
      //Короткое нажатие
      else {
        if (waitingSecondPress && millis() - lastPressTime < 500) {
          //двойное нажатие → аварийный режим
          emergency = !emergency;
          if (emergency) {
            goToState(S_WARNING, 500);
          } else {
            goToState(S_RED, 10000); // возврат в цикл
          }
          Serial.println("Emergency toggled");
          waitingSecondPress = false;
        } else {
          // ждём второе нажатие
          waitingSecondPress = true;
          lastPressTime = millis();
        }
      }
      pressStart = 0;
    }
  }

  //Если ждали второе нажатие, но не дождались → считаем как пешеходный запрос
  if (waitingSecondPress && millis() - lastPressTime > 500) {
    pedRequest = true;
    Serial.println("Pedestrian request!");
    waitingSecondPress = false;
  }
}

void loop() {
  readInputs();

  switch (currentState) {
    case S_GREEN:
      if (millis() - stateStart >= duration) {
        goToState(S_YELLOW, 3000);
      }
      break;

    case S_YELLOW:
      if (millis() - stateStart >= duration) {
        if (pedRequest) {
          pedRequest = false;
          goToState(S_RED, 15000); // удлинённый красный
        } else {
          goToState(S_RED, 10000);
        }
      }
      break;

    case S_RED:
      if (millis() - stateStart >= duration) {
        goToState(S_GREEN, 10000);
      }
      break;

    case S_WARNING:
      if (millis() - stateStart >= duration) {
        digitalWrite(YELLOW, !digitalRead(YELLOW));
        stateStart = millis();
      }
      break;

    case S_NIGHT:
      if (millis() - stateStart >= duration) {
        digitalWrite(YELLOW, !digitalRead(YELLOW));
        stateStart = millis();
      }
      break;
  }
}