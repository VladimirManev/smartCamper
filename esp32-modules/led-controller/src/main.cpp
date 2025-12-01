// LED Controller - Стъпка 2: Добавяне на димиране
// Една лента, един бутон, включване/изключване + димиране

#include <Arduino.h>
#include <NeoPixelBus.h>

// Настройки за лента 1
#define LED_PIN_1 2
#define LED_COUNT_1 44

// Настройки за бутон 1
#define BUTTON_PIN_1 4

// Яркост настройки
#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 128  // 50% при първо включване

// Димиране настройки
#define DIMMING_TIME 4000  // 4 секунди от мин до макс
#define HOLD_THRESHOLD 250  // 250ms преди да започне димиране (за да не се случи при click)
#define BLINK_DURATION 300  // Продължителност на премигването при мин/макс (ms)
#define BLINK_MIN_FACTOR 0.3  // Минимална яркост при премигване (30% от текущата)

// Транзакции настройки
#define TRANSITION_DURATION 1000  // 1 секунда за транзакции
#define NUM_ON_TRANSITIONS 5   // Брой транзакции за включване
#define NUM_OFF_TRANSITIONS 5  // Брой транзакции за изключване

// Тип на лентата: WS2815 RGBW
NeoPixelBus<NeoRgbwFeature, NeoWs2814Method> strip1(LED_COUNT_1, LED_PIN_1);

// Състояние на лентата
bool strip1On = false;
uint8_t strip1Brightness = DEFAULT_BRIGHTNESS;  // Запазваме последната яркост

// Състояния на бутона
enum ButtonState {
  BUTTON_IDLE,
  BUTTON_PRESSED,
  BUTTON_HELD
};

ButtonState button1State = BUTTON_IDLE;
unsigned long button1PressTime = 0;
bool dimmingActive = false;
bool dimmingDirection = true;  // true = увеличава, false = намаля
unsigned long dimmingStartTime = 0;
uint8_t dimmingStartBrightness = 0;
bool lastDimmingWasIncrease = true;  // Запомняме последната посока

// Премигване при мин/макс
bool blinkActive = false;
unsigned long blinkStartTime = 0;
uint8_t savedBrightnessForBlink = 0;

// Транзакции (transition animations)
enum TransitionType {
  TRANSITION_NONE,
  TRANSITION_ON_CENTER_TO_EDGES,      // 0: Плавно от центъра към краищата
  TRANSITION_ON_RANDOM_LEDS,           // 1: Произволни диоди последователно
  TRANSITION_ON_FADE_BRIGHTNESS,       // 2: Плавно вдигане на яркостта
  TRANSITION_ON_LEFT_TO_RIGHT,         // 3: От ляво надясно
  TRANSITION_ON_EDGES_TO_CENTER,       // 4: От краищата към центъра
  TRANSITION_OFF_EDGES_TO_CENTER,      // 5: От краищата към центъра
  TRANSITION_OFF_FADE_BRIGHTNESS,       // 6: Плавно изгасване на яркостта
  TRANSITION_OFF_RANDOM_LEDS,          // 7: Произволни диоди последователно
  TRANSITION_OFF_LEFT_TO_RIGHT,        // 8: От ляво надясно
  TRANSITION_OFF_CENTER_TO_EDGES       // 9: От центъра към краищата
};

struct TransitionState {
  bool active;
  TransitionType type;
  unsigned long startTime;
  uint8_t targetBrightness;
  uint8_t* randomOrder;  // За произволни диоди
  int randomIndex;
};

TransitionState transitionState = {false, TRANSITION_NONE, 0, 0, nullptr, 0};

// Помощна функция - R и G са разменени в хардуера
RgbwColor fixColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return RgbwColor(g, r, b, w);  // Разменяме R и G
}

// Forward declarations
void updateStrip1();

// ============================================================================
// ТРАНЗАКЦИИ ЗА ВКЛЮЧВАНЕ
// ============================================================================

// 0: Плавно изпълване от средата към краищата
void transitionOnCenterToEdges() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = LED_COUNT_1 / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  for (int i = 0; i <= currentDistance; i++) {
    if (center - i >= 0) {
      strip1.SetPixelColor(center - i, RgbwColor(0, 0, 0, transitionState.targetBrightness));
    }
    if (center + i < LED_COUNT_1) {
      strip1.SetPixelColor(center + i, RgbwColor(0, 0, 0, transitionState.targetBrightness));
    }
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    transitionState.active = false;
  }
}

// 1: Появяване като последователно се включват произволни диоди
void transitionOnRandomLeds() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  // Инициализиране на произволен ред при първо извикване
  if (transitionState.randomOrder == nullptr) {
    transitionState.randomOrder = new uint8_t[LED_COUNT_1];
    for (int i = 0; i < LED_COUNT_1; i++) {
      transitionState.randomOrder[i] = i;
    }
    // Shuffle (Fisher-Yates)
    for (int i = LED_COUNT_1 - 1; i > 0; i--) {
      int j = random(0, i + 1);
      uint8_t temp = transitionState.randomOrder[i];
      transitionState.randomOrder[i] = transitionState.randomOrder[j];
      transitionState.randomOrder[j] = temp;
    }
    transitionState.randomIndex = 0;
  }
  
  int targetCount = (int)(LED_COUNT_1 * progress);
  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  
  for (int i = 0; i < targetCount && i < LED_COUNT_1; i++) {
    strip1.SetPixelColor(transitionState.randomOrder[i], 
                         RgbwColor(0, 0, 0, transitionState.targetBrightness));
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    delete[] transitionState.randomOrder;
    transitionState.randomOrder = nullptr;
    transitionState.active = false;
  }
}

// 2: Плавно вдигане на яркостта от 1 до targetBrightness
void transitionOnFadeBrightness() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  // Плавно увеличение от MIN_BRIGHTNESS до targetBrightness
  uint8_t currentBrightness = MIN_BRIGHTNESS + (uint8_t)((transitionState.targetBrightness - MIN_BRIGHTNESS) * progress);
  
  for (int i = 0; i < LED_COUNT_1; i++) {
    strip1.SetPixelColor(i, RgbwColor(0, 0, 0, currentBrightness));
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    transitionState.active = false;
  }
}

// 3: Плавно изпълване от единия край към другия (left-to-right)
void transitionOnLeftToRight() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int currentEnd = (int)(LED_COUNT_1 * progress);
  
  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  for (int i = 0; i < currentEnd; i++) {
    strip1.SetPixelColor(i, RgbwColor(0, 0, 0, transitionState.targetBrightness));
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    transitionState.active = false;
  }
}

// 4: Плавно изпълване от двата края едновременно към центъра
void transitionOnEdgesToCenter() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = LED_COUNT_1 / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * (1.0 - progress));  // Обратно - от краищата към центъра
  
  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  for (int i = 0; i <= maxDistance - currentDistance; i++) {
    if (center - i >= 0) {
      strip1.SetPixelColor(center - i, RgbwColor(0, 0, 0, transitionState.targetBrightness));
    }
    if (center + i < LED_COUNT_1) {
      strip1.SetPixelColor(center + i, RgbwColor(0, 0, 0, transitionState.targetBrightness));
    }
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    transitionState.active = false;
  }
}

// ============================================================================
// ТРАНЗАКЦИИ ЗА ИЗКЛЮЧВАНЕ
// ============================================================================

// 5: Плавно изгасване от краищата към центъра
void transitionOffEdgesToCenter() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = LED_COUNT_1 / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  // Започваме с всички светещи
  for (int i = 0; i < LED_COUNT_1; i++) {
    strip1.SetPixelColor(i, RgbwColor(0, 0, 0, transitionState.targetBrightness));
  }
  
  // Изгасваме от краищата към центъра
  for (int i = 0; i < currentDistance; i++) {
    if (i < LED_COUNT_1) {
      strip1.SetPixelColor(i, RgbwColor(0, 0, 0, 0));
    }
    if (LED_COUNT_1 - 1 - i >= 0) {
      strip1.SetPixelColor(LED_COUNT_1 - 1 - i, RgbwColor(0, 0, 0, 0));
    }
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    transitionState.active = false;
  }
}

// 6: Плавно изгасване на яркостта от targetBrightness до 0
void transitionOffFadeBrightness() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  // Плавно намаляване от targetBrightness до 0
  uint8_t currentBrightness = transitionState.targetBrightness - (uint8_t)(transitionState.targetBrightness * progress);
  
  for (int i = 0; i < LED_COUNT_1; i++) {
    strip1.SetPixelColor(i, RgbwColor(0, 0, 0, currentBrightness));
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    transitionState.active = false;
  }
}

// 7: Изгасяне на произволни диоди, докато всички изгаснат
void transitionOffRandomLeds() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  // Инициализиране на произволен ред при първо извикване
  if (transitionState.randomOrder == nullptr) {
    transitionState.randomOrder = new uint8_t[LED_COUNT_1];
    for (int i = 0; i < LED_COUNT_1; i++) {
      transitionState.randomOrder[i] = i;
    }
    // Shuffle (Fisher-Yates)
    for (int i = LED_COUNT_1 - 1; i > 0; i--) {
      int j = random(0, i + 1);
      uint8_t temp = transitionState.randomOrder[i];
      transitionState.randomOrder[i] = transitionState.randomOrder[j];
      transitionState.randomOrder[j] = temp;
    }
    transitionState.randomIndex = 0;
  }
  
  int offCount = (int)(LED_COUNT_1 * progress);
  
  // Започваме с всички светещи
  for (int i = 0; i < LED_COUNT_1; i++) {
    strip1.SetPixelColor(i, RgbwColor(0, 0, 0, transitionState.targetBrightness));
  }
  
  // Изгасваме произволни диоди
  for (int i = 0; i < offCount && i < LED_COUNT_1; i++) {
    strip1.SetPixelColor(transitionState.randomOrder[i], RgbwColor(0, 0, 0, 0));
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    delete[] transitionState.randomOrder;
    transitionState.randomOrder = nullptr;
    transitionState.active = false;
  }
}

// 8: Плавно изгасване от единия край към другия (left-to-right)
void transitionOffLeftToRight() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int currentEnd = (int)(LED_COUNT_1 * progress);
  
  // Започваме с всички светещи
  for (int i = 0; i < LED_COUNT_1; i++) {
    strip1.SetPixelColor(i, RgbwColor(0, 0, 0, transitionState.targetBrightness));
  }
  
  // Изгасваме от ляво надясно
  for (int i = 0; i < currentEnd; i++) {
    strip1.SetPixelColor(i, RgbwColor(0, 0, 0, 0));
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    transitionState.active = false;
  }
}

// 9: Плавно изгасване от центъра към краищата
void transitionOffCenterToEdges() {
  unsigned long elapsed = millis() - transitionState.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = LED_COUNT_1 / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  // Започваме с всички светещи
  for (int i = 0; i < LED_COUNT_1; i++) {
    strip1.SetPixelColor(i, RgbwColor(0, 0, 0, transitionState.targetBrightness));
  }
  
  // Изгасваме от центъра към краищата
  for (int i = 0; i <= currentDistance; i++) {
    if (center - i >= 0) {
      strip1.SetPixelColor(center - i, RgbwColor(0, 0, 0, 0));
    }
    if (center + i < LED_COUNT_1) {
      strip1.SetPixelColor(center + i, RgbwColor(0, 0, 0, 0));
    }
  }
  strip1.Show();
  
  if (progress >= 1.0) {
    transitionState.active = false;
  }
}

// ============================================================================
// УПРАВЛЕНИЕ НА ТРАНЗАКЦИИТЕ
// ============================================================================

// Масиви с указатели към функции за транзакции
typedef void (*TransitionFunction)();

TransitionFunction onTransitions[NUM_ON_TRANSITIONS] = {
  transitionOnCenterToEdges,
  transitionOnRandomLeds,
  transitionOnFadeBrightness,
  transitionOnLeftToRight,
  transitionOnEdgesToCenter
};

TransitionFunction offTransitions[NUM_OFF_TRANSITIONS] = {
  transitionOffEdgesToCenter,
  transitionOffFadeBrightness,
  transitionOffRandomLeds,
  transitionOffLeftToRight,
  transitionOffCenterToEdges
};

// Стартиране на транзакция
void startTransition(bool turningOn) {
  if (transitionState.active) return;  // Вече има активна транзакция
  
  transitionState.active = true;
  transitionState.startTime = millis();
  transitionState.targetBrightness = strip1Brightness;
  transitionState.randomOrder = nullptr;
  transitionState.randomIndex = 0;
  
  if (turningOn) {
    // Произволен избор на транзакция за включване
    int index = random(0, NUM_ON_TRANSITIONS);
    transitionState.type = (TransitionType)index;
    Serial.println("✨ Starting ON transition " + String(index));
  } else {
    // Произволен избор на транзакция за изключване
    int index = random(0, NUM_OFF_TRANSITIONS);
    transitionState.type = (TransitionType)(NUM_ON_TRANSITIONS + index);
    Serial.println("✨ Starting OFF transition " + String(index));
  }
}

// Обновяване на активна транзакция
void updateTransition() {
  if (!transitionState.active) return;
  
  if (transitionState.type < NUM_ON_TRANSITIONS) {
    // Транзакция за включване
    onTransitions[transitionState.type]();
  } else {
    // Транзакция за изключване
    int offIndex = transitionState.type - NUM_ON_TRANSITIONS;
    offTransitions[offIndex]();
  }
  
  // Ако транзакцията приключи, финализираме
  if (!transitionState.active) {
    if (transitionState.type < NUM_ON_TRANSITIONS) {
      // Включването приключи - задаваме финалната яркост
      updateStrip1();
      Serial.println("✅ ON transition completed");
    } else {
      // Изключването приключи - изчистваме лентата
      strip1.ClearTo(RgbwColor(0, 0, 0, 0));
      strip1.Show();
      Serial.println("✅ OFF transition completed");
    }
  }
}

// Обновяване на лентата с текущата яркост
void updateStrip1() {
  if (strip1On) {
    for (int i = 0; i < LED_COUNT_1; i++) {
      strip1.SetPixelColor(i, RgbwColor(0, 0, 0, strip1Brightness));
    }
  } else {
    strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  }
  strip1.Show();
}

// Обновяване на премигване - плавно намаляване и обратно увеличение
void updateBlink() {
  if (!blinkActive || !strip1On) return;
  
  unsigned long elapsed = millis() - blinkStartTime;
  
  if (elapsed < BLINK_DURATION) {
    // Използваме синусоида за плавно премигване
    // Синусът върви от 0 до PI, което дава плавно намаляване и обратно увеличение
    float progress = (float)elapsed / BLINK_DURATION;
    float sineWave = sin(progress * PI);  // От 0 до 1 и обратно до 0
    
    // Превръщаме синусоидата в диапазон от BLINK_MIN_FACTOR до 1.0
    // Когато sineWave = 0 (начало/край), яркостта е пълна (1.0)
    // Когато sineWave = 1 (средата), яркостта е минимална (BLINK_MIN_FACTOR)
    float brightnessFactor = 1.0 - (1.0 - BLINK_MIN_FACTOR) * sineWave;
    
    uint8_t currentBrightness = (uint8_t)(savedBrightnessForBlink * brightnessFactor);
    
    // Прилагаме яркостта
    for (int i = 0; i < LED_COUNT_1; i++) {
      strip1.SetPixelColor(i, RgbwColor(0, 0, 0, currentBrightness));
    }
    strip1.Show();
  } else {
    // Премигването приключи - връщаме се на пълна яркост
    blinkActive = false;
    strip1Brightness = savedBrightnessForBlink;
    updateStrip1();
  }
}

// Обновяване на димиране
void updateDimming() {
  if (!dimmingActive || !strip1On) return;
  
  unsigned long elapsed = millis() - dimmingStartTime;
  float progress = (float)elapsed / DIMMING_TIME;
  
  if (progress >= 1.0) {
    progress = 1.0;
    dimmingActive = false;
  }
  
  uint8_t targetBrightness = dimmingDirection ? MAX_BRIGHTNESS : MIN_BRIGHTNESS;
  uint8_t newBrightness = dimmingStartBrightness + (int)((targetBrightness - dimmingStartBrightness) * progress);
  
  // Ограничаваме в границите
  if (newBrightness > MAX_BRIGHTNESS) newBrightness = MAX_BRIGHTNESS;
  if (newBrightness < MIN_BRIGHTNESS) newBrightness = MIN_BRIGHTNESS;
  
  // Проверяваме дали сме достигнали целта
  bool reachedLimit = false;
  if (dimmingDirection && newBrightness >= MAX_BRIGHTNESS) {
    newBrightness = MAX_BRIGHTNESS;
    reachedLimit = true;
  } else if (!dimmingDirection && newBrightness <= MIN_BRIGHTNESS) {
    newBrightness = MIN_BRIGHTNESS;
    reachedLimit = true;
  }
  
  if (reachedLimit && !blinkActive) {
    // Започваме премигване
    dimmingActive = false;
    lastDimmingWasIncrease = dimmingDirection;
    blinkActive = true;
    blinkStartTime = millis();
    savedBrightnessForBlink = newBrightness;
    strip1Brightness = newBrightness;
    Serial.println("✨ Reached " + String(dimmingDirection ? "MAX" : "MIN") + " brightness - blinking");
  } else if (!reachedLimit) {
    strip1Brightness = newBrightness;
    updateStrip1();
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n✨ LED Controller Starting...");
  Serial.println("Strip 1 - Pin: " + String(LED_PIN_1) + ", LEDs: " + String(LED_COUNT_1));
  Serial.println("Button 1 - Pin: " + String(BUTTON_PIN_1));
  Serial.println("Dimming: " + String(DIMMING_TIME) + "ms, Hold threshold: " + String(HOLD_THRESHOLD) + "ms");
  Serial.println("Transitions: " + String(TRANSITION_DURATION) + "ms");
  
  // Инициализация на лентата
  strip1.Begin();
  delay(100);
  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  strip1.Show();
  
  // Инициализация на бутона
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  
  // Инициализация на random seed
  randomSeed(analogRead(0));
  
  Serial.println("✅ System ready!");
  Serial.println("Click: Toggle ON/OFF (with random transitions)");
  Serial.println("Hold: Dim/Increase brightness\n");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Четем състоянието на бутона (LOW = натиснат, HIGH = не натиснат при INPUT_PULLUP)
  bool rawButtonReading = (digitalRead(BUTTON_PIN_1) == LOW);
  
  // Debounce логика - правилна имплементация
  static bool lastRawReading = false;
  static unsigned long lastDebounceTime = 0;
  static bool debouncedButtonState = false;
  const unsigned long DEBOUNCE_DELAY = 50;
  
  // Ако се е променило състоянието, рестартираме debounce таймера
  if (rawButtonReading != lastRawReading) {
    lastDebounceTime = currentTime;
  }
  
  // Ако е минало достатъчно време без промяна, приемаме новото състояние като стабилно
  if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) {
    debouncedButtonState = rawButtonReading;
  }
  
  lastRawReading = rawButtonReading;
  
  // State machine за бутона - използваме debouncedButtonState
  switch (button1State) {
    case BUTTON_IDLE:
      if (debouncedButtonState) {  // Бутонът е натиснат (LOW = true)
        button1State = BUTTON_PRESSED;
        button1PressTime = currentTime;
        Serial.println("🔘 Button pressed (IDLE -> PRESSED)");
      }
      break;
      
    case BUTTON_PRESSED:
      if (debouncedButtonState) {  // Все още натиснат
        // Проверяваме дали е задържан достатъчно дълго за димиране
        if (currentTime - button1PressTime >= HOLD_THRESHOLD) {
          button1State = BUTTON_HELD;
          // Започваме димиране
          if (strip1On) {
            dimmingActive = true;
            dimmingStartTime = currentTime;
            dimmingStartBrightness = strip1Brightness;
            // Редуваме посоката при всяко ново задържане
            dimmingDirection = !lastDimmingWasIncrease;
            lastDimmingWasIncrease = dimmingDirection;
            Serial.println("🔆 Dimming: " + String(dimmingDirection ? "Increasing" : "Decreasing"));
          }
        }
      } else {  // Отпускане преди HOLD_THRESHOLD - това е click
        button1State = BUTTON_IDLE;
        strip1On = !strip1On;
        
        if (strip1On) {
          // Включваме с транзакция
          startTransition(true);
          Serial.println("💡 Strip 1: Turning ON (Brightness: " + String(strip1Brightness) + ")");
        } else {
          // Изключваме с транзакция - запазваме текущата яркост преди транзакцията
          startTransition(false);
          Serial.println("💡 Strip 1: Turning OFF (Saved brightness: " + String(strip1Brightness) + ")");
        }
      }
      break;
      
    case BUTTON_HELD:
      if (!debouncedButtonState) {  // Бутонът е отпущен (HIGH = false)
        button1State = BUTTON_IDLE;
        dimmingActive = false;
        Serial.println("🔆 Dimming stopped (Brightness: " + String(strip1Brightness) + ")");
      } else {
        // Все още задържаме - димирането се обновява в updateDimming()
        // Ако димирането е приключило, спираме
        if (!dimmingActive) {
          // Димирането е приключило, но бутонът все още е натиснат
          // Нищо не правим, чакаме отпускане
        }
      }
      break;
  }
  
  // Обновяване на транзакции (приоритет над димиране)
  if (transitionState.active) {
    updateTransition();
  } else {
    // Обновяване на димиране (само ако няма активна транзакция)
    updateDimming();
    
    // Обновяване на премигване
    updateBlink();
  }
  
  delay(10);
}

