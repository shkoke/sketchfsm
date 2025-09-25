if (pressTime >= 2000) {
        // Долгое нажатие → ночной режим
        processEvent(E_NIGHT_TOGGLE);
      } else {
        if (waitingSecondPress && millis() - lastPressTime < 500) {
          // Двойное нажатие → авария
          if (!emergency) {
            emergency = true;
            processEvent(E_EMERGENCY_ON);
          } else {
            emergency = false;
            processEvent(E_EMERGENCY_OFF);
          }
          waitingSecondPress = false;
        } else {
          // Ждём второе нажатие
          waitingSecondPress = true;
          lastPressTime = millis();
        }
      }
      pressStart = 0;
    }
  }

  if (waitingSecondPress && millis() - lastPressTime > 500) {
    processEvent(E_PED);
    waitingSecondPress = false;
  }
}

// Генерация событий
void processEvent(Event ev) {
  if (ev == E_NONE) return;
  Handler h = fsmTable[currentState][ev];
  if (h) h();
}

// Инициализация
void setup() {
  pinMode(RED,    OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN,  OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  Serial.begin(9600);

  setupTable();
  goToState(S_GREEN, 10000);
}

void loop() {
  readInputs();

  if (millis() - stateStart >= duration) {
    processEvent(E_TIMER);
  }
}