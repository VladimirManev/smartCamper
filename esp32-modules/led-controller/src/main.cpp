// LED Controller - Multi-strip support with abstraction
// Поддръжка за множество ленти с абстракция

#include <Arduino.h>
#include <NeoPixelBus.h>

// ============================================================================
// КОНФИГУРАЦИЯ
// ============================================================================

// Брой ленти (променя се лесно)
#define NUM_STRIPS 3

// Настройки за лентите (добавяне на нови ленти тук)
struct StripConfig {
  uint8_t pin;
  uint16_t ledCount;
};

StripConfig stripConfigs[NUM_STRIPS] = {
  {33, 44},  // Strip 0: Pin 33, 44 LEDs - Kitchen (main)
  {18, 53},  // Strip 1: Pin 18, 53 LEDs
  {19, 23}   // Strip 2: Pin 19, 23 LEDs - Kitchen (extension, mirrors strip 0)
};

// Настройки за бутони
#define NUM_BUTTONS 2    // Брой бутони (Strip 2 се управлява автоматично от Strip 0)
#define BUTTON_PIN_1 4   // Бутон за strip 0 (Kitchen - управлява Strip 0 и Strip 2)
#define BUTTON_PIN_2 12  // Бутон за strip 1

// Яркост настройки
#define MIN_BRIGHTNESS 1
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 128  // 50% при първо включване

// Димиране настройки
#define DIMMING_SPEED 50  // единици яркост/секунда (скорост на промяна)
#define HOLD_THRESHOLD 250  // 250ms преди да започне димиране
#define BLINK_DURATION 300  // Продължителност на премигването при макс (ms)
#define BLINK_MIN_FACTOR 0.3  // Минимална яркост при премигване (30% от текущата)

// Транзакции настройки
#define TRANSITION_DURATION 1000  // 1 секунда за транзакции
#define NUM_ON_TRANSITIONS 4   // Брой транзакции за включване
#define NUM_OFF_TRANSITIONS 4  // Брой транзакции за изключване

// ============================================================================
// СТРУКТУРИ И ТИПОВЕ
// ============================================================================

// Тип на лентата: WS2815 RGBW
// За ESP32 използваме RMT методи - всяка лента трябва да използва различен RMT канал
// Strip 0 и Strip 2 (Kitchen) са синхронизирани в софтуера, но използват различни RMT канали
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt0Ws2812xMethod> LedStrip0;
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt1Ws2812xMethod> LedStrip1;
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt2Ws2812xMethod> LedStrip2;

// Общ тип за указатели
typedef NeoPixelBus<NeoRgbwFeature, NeoEsp32Rmt0Ws2812xMethod> LedStrip;

// Състояния на бутона
enum ButtonState {
  BUTTON_IDLE,
  BUTTON_PRESSED,
  BUTTON_HELD
};

// Типове транзакции
enum TransitionType {
  TRANSITION_NONE,
  TRANSITION_ON_CENTER_TO_EDGES,      // 0: Плавно от центъра към краищата
  TRANSITION_ON_RANDOM_LEDS,           // 1: Произволни диоди последователно
  TRANSITION_ON_LEFT_TO_RIGHT,         // 2: От ляво надясно
  TRANSITION_ON_EDGES_TO_CENTER,       // 3: От краищата към центъра
  TRANSITION_OFF_EDGES_TO_CENTER,      // 4: От краищата към центъра
  TRANSITION_OFF_RANDOM_LEDS,          // 5: Произволни диоди последователно
  TRANSITION_OFF_LEFT_TO_RIGHT,        // 6: От ляво надясно
  TRANSITION_OFF_CENTER_TO_EDGES       // 7: От центъра към краищата
};

// Състояние на транзакция
struct TransitionState {
  bool active;
  TransitionType type;
  unsigned long startTime;
  uint8_t targetBrightness;
  uint8_t* randomOrder;
  int randomIndex;
};

// Състояние на лента - използваме void* за да поддържаме различни типове
struct StripState {
  void* strip;  // Указател към LedStrip0 или LedStrip1
  uint8_t stripType;  // 0 = LedStrip0 (RMT0), 1 = LedStrip1 (RMT1), 2 = LedStrip2 (RMT2, синхронизиран с Strip 0)
  bool on;
  uint8_t brightness;
  
  // Димиране
  bool dimmingActive;
  bool dimmingDirection;  // true = увеличава, false = намаля
  unsigned long dimmingStartTime;
  uint8_t dimmingStartBrightness;
  unsigned long dimmingDuration;  // време за димиране в милисекунди (изчислява се динамично)
  bool lastDimmingWasIncrease;
  
  // Премигване
  bool blinkActive;
  unsigned long blinkStartTime;
  uint8_t savedBrightnessForBlink;
  
  // Транзакции
  TransitionState transition;
};

// ============================================================================
// ГЛОБАЛНИ ПРОМЕНЛИВИ
// ============================================================================

// Масив от ленти - използваме статични обекти с различни RMT канали
LedStrip0 strip0(stripConfigs[0].ledCount, stripConfigs[0].pin);  // Kitchen main
LedStrip1 strip1(stripConfigs[1].ledCount, stripConfigs[1].pin);
LedStrip2 strip2(stripConfigs[2].ledCount, stripConfigs[2].pin);  // Kitchen extension

// Указатели към лентите (за универсалност)
LedStrip* strips[NUM_STRIPS] = {(LedStrip*)&strip0, (LedStrip*)&strip1, (LedStrip*)&strip2};
StripState stripStates[NUM_STRIPS];

// Бутони
struct ButtonStateMachine {
  ButtonState state;
  unsigned long pressTime;
  uint8_t pin;
  uint8_t stripIndex;  // Коя лента управлява този бутон
  
  // Debounce state
  bool lastRawReading;
  unsigned long lastDebounceTime;
  bool debouncedState;
};

ButtonStateMachine buttons[NUM_BUTTONS] = {
  {BUTTON_IDLE, 0, BUTTON_PIN_1, 0, false, 0, false},  // Бутон 0 -> Strip 0 (Kitchen - управлява Strip 0 и Strip 2)
  {BUTTON_IDLE, 0, BUTTON_PIN_2, 1, false, 0, false}   // Бутон 1 -> Strip 1
};

// ============================================================================
// ПОМОЩНИ ФУНКЦИИ
// ============================================================================

// R и G са разменени в хардуера
RgbwColor fixColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return RgbwColor(g, r, b, w);
}

// Helper функции за работа с различните типове ленти
void setPixelColor(uint8_t stripIndex, int pixelIndex, RgbwColor color) {
  if (stripIndex >= NUM_STRIPS) return;
  StripState& state = stripStates[stripIndex];
  if (state.stripType == 0) {
    ((LedStrip0*)state.strip)->SetPixelColor(pixelIndex, color);
  } else if (state.stripType == 1) {
    ((LedStrip1*)state.strip)->SetPixelColor(pixelIndex, color);
  } else {
    ((LedStrip2*)state.strip)->SetPixelColor(pixelIndex, color);
  }
}

void clearStrip(uint8_t stripIndex, RgbwColor color) {
  if (stripIndex >= NUM_STRIPS) return;
  StripState& state = stripStates[stripIndex];
  if (state.stripType == 0) {
    ((LedStrip0*)state.strip)->ClearTo(color);
  } else if (state.stripType == 1) {
    ((LedStrip1*)state.strip)->ClearTo(color);
  } else {
    ((LedStrip2*)state.strip)->ClearTo(color);
  }
}

void showStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  StripState& state = stripStates[stripIndex];
  if (state.stripType == 0) {
    ((LedStrip0*)state.strip)->Show();
  } else if (state.stripType == 1) {
    ((LedStrip1*)state.strip)->Show();
  } else {
    ((LedStrip2*)state.strip)->Show();
  }
}

// Макрос за по-лесна употреба в транзакциите
#define STRIP_CLEAR(idx, color) clearStrip(idx, color)
#define STRIP_SET_PIXEL(idx, pixel, color) setPixelColor(idx, pixel, color)
#define STRIP_SHOW(idx) showStrip(idx)

// Helper функция за синхронизация на Kitchen (Strip 0 и Strip 2)
void syncKitchenExtension(uint8_t mainStripIndex) {
  if (mainStripIndex == 0) {
    StripState& mainState = stripStates[0];
    StripState& extState = stripStates[2];
    
    // Копираме състоянието от main към extension
    extState.on = mainState.on;
    extState.brightness = mainState.brightness;
    extState.dimmingActive = mainState.dimmingActive;
    extState.dimmingDirection = mainState.dimmingDirection;
    extState.dimmingStartTime = mainState.dimmingStartTime;
    extState.dimmingStartBrightness = mainState.dimmingStartBrightness;
    extState.dimmingDuration = mainState.dimmingDuration;
    extState.blinkActive = mainState.blinkActive;
    extState.blinkStartTime = mainState.blinkStartTime;
    extState.savedBrightnessForBlink = mainState.savedBrightnessForBlink;
    extState.transition.active = mainState.transition.active;
    extState.transition.type = mainState.transition.type;
    extState.transition.startTime = mainState.transition.startTime;
    extState.transition.targetBrightness = mainState.transition.targetBrightness;
    
    // Обновяваме extension лентата
    if (extState.on) {
      for (int i = 0; i < stripConfigs[2].ledCount; i++) {
        setPixelColor(2, i, RgbwColor(0, 0, 0, extState.brightness));
      }
    } else {
      clearStrip(2, RgbwColor(0, 0, 0, 0));
    }
    showStrip(2);
  }
}

// Обновяване на лента с текущата яркост
void updateStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  
  if (state.on) {
    for (int i = 0; i < stripConfigs[stripIndex].ledCount; i++) {
      setPixelColor(stripIndex, i, RgbwColor(0, 0, 0, state.brightness));
    }
  } else {
    clearStrip(stripIndex, RgbwColor(0, 0, 0, 0));
  }
  showStrip(stripIndex);
  
  // Kitchen: синхронизираме extension лентата
  syncKitchenExtension(stripIndex);
}

// ============================================================================
// ТРАНЗАКЦИИ ЗА ВКЛЮЧВАНЕ
// ============================================================================

void transitionOnCenterToEdges(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  STRIP_CLEAR(stripIndex, RgbwColor(0, 0, 0, 0));
  for (int i = 0; i <= currentDistance; i++) {
    if (center - i >= 0) {
      STRIP_SET_PIXEL(stripIndex, center - i, RgbwColor(0, 0, 0, trans.targetBrightness));
    }
    if (center + i < ledCount) {
      STRIP_SET_PIXEL(stripIndex, center + i, RgbwColor(0, 0, 0, trans.targetBrightness));
    }
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void transitionOnRandomLeds(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  if (trans.randomOrder == nullptr) {
    trans.randomOrder = new uint8_t[ledCount];
    for (int i = 0; i < ledCount; i++) {
      trans.randomOrder[i] = i;
    }
    for (int i = ledCount - 1; i > 0; i--) {
      int j = random(0, i + 1);
      uint8_t temp = trans.randomOrder[i];
      trans.randomOrder[i] = trans.randomOrder[j];
      trans.randomOrder[j] = temp;
    }
    trans.randomIndex = 0;
  }
  
  int targetCount = (int)(ledCount * progress);
  STRIP_CLEAR(stripIndex, RgbwColor(0, 0, 0, 0));
  
  for (int i = 0; i < targetCount && i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, trans.randomOrder[i], 
                    RgbwColor(0, 0, 0, trans.targetBrightness));
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    delete[] trans.randomOrder;
    trans.randomOrder = nullptr;
    trans.active = false;
  }
}

void transitionOnLeftToRight(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int currentEnd = (int)(ledCount * progress);
  
  STRIP_CLEAR(stripIndex, RgbwColor(0, 0, 0, 0));
  for (int i = 0; i < currentEnd; i++) {
    STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, trans.targetBrightness));
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void transitionOnEdgesToCenter(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * (1.0 - progress));
  
  STRIP_CLEAR(stripIndex, RgbwColor(0, 0, 0, 0));
  for (int i = 0; i <= maxDistance - currentDistance; i++) {
    if (center - i >= 0) {
      STRIP_SET_PIXEL(stripIndex, center - i, RgbwColor(0, 0, 0, trans.targetBrightness));
    }
    if (center + i < ledCount) {
      STRIP_SET_PIXEL(stripIndex, center + i, RgbwColor(0, 0, 0, trans.targetBrightness));
    }
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

// ============================================================================
// ТРАНЗАКЦИИ ЗА ИЗКЛЮЧВАНЕ
// ============================================================================

void transitionOffEdgesToCenter(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  for (int i = 0; i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, trans.targetBrightness));
  }
  
  for (int i = 0; i < currentDistance; i++) {
    if (i < ledCount) {
      STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, 0));
    }
    if (ledCount - 1 - i >= 0) {
      STRIP_SET_PIXEL(stripIndex, ledCount - 1 - i, RgbwColor(0, 0, 0, 0));
    }
  }
  
  // При нечетен брой LED-и, изчистваме и средния LED когато достигнем центъра
  if (ledCount % 2 == 1 && currentDistance >= center) {
    STRIP_SET_PIXEL(stripIndex, center, RgbwColor(0, 0, 0, 0));
  }
  
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void transitionOffRandomLeds(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  if (trans.randomOrder == nullptr) {
    trans.randomOrder = new uint8_t[ledCount];
    for (int i = 0; i < ledCount; i++) {
      trans.randomOrder[i] = i;
    }
    for (int i = ledCount - 1; i > 0; i--) {
      int j = random(0, i + 1);
      uint8_t temp = trans.randomOrder[i];
      trans.randomOrder[i] = trans.randomOrder[j];
      trans.randomOrder[j] = temp;
    }
    trans.randomIndex = 0;
  }
  
  int offCount = (int)(ledCount * progress);
  
  for (int i = 0; i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, trans.targetBrightness));
  }
  
  for (int i = 0; i < offCount && i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, trans.randomOrder[i], RgbwColor(0, 0, 0, 0));
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    delete[] trans.randomOrder;
    trans.randomOrder = nullptr;
    trans.active = false;
  }
}

void transitionOffLeftToRight(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int currentEnd = (int)(ledCount * progress);
  
  for (int i = 0; i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, trans.targetBrightness));
  }
  
  for (int i = 0; i < currentEnd; i++) {
    STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, 0));
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

void transitionOffCenterToEdges(uint8_t stripIndex) {
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  uint16_t ledCount = stripConfigs[stripIndex].ledCount;
  
  unsigned long elapsed = millis() - trans.startTime;
  float progress = (float)elapsed / TRANSITION_DURATION;
  if (progress > 1.0) progress = 1.0;
  
  int center = ledCount / 2;
  int maxDistance = center;
  int currentDistance = (int)(maxDistance * progress);
  
  for (int i = 0; i < ledCount; i++) {
    STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, trans.targetBrightness));
  }
  
  for (int i = 0; i <= currentDistance; i++) {
    if (center - i >= 0) {
      STRIP_SET_PIXEL(stripIndex, center - i, RgbwColor(0, 0, 0, 0));
    }
    if (center + i < ledCount) {
      STRIP_SET_PIXEL(stripIndex, center + i, RgbwColor(0, 0, 0, 0));
    }
  }
  STRIP_SHOW(stripIndex);
  
  if (progress >= 1.0) {
    trans.active = false;
  }
}

// ============================================================================
// УПРАВЛЕНИЕ НА ТРАНЗАКЦИИТЕ
// ============================================================================

typedef void (*TransitionFunction)(uint8_t);

TransitionFunction onTransitions[NUM_ON_TRANSITIONS] = {
  transitionOnCenterToEdges,
  transitionOnRandomLeds,
  transitionOnLeftToRight,
  transitionOnEdgesToCenter
};

TransitionFunction offTransitions[NUM_OFF_TRANSITIONS] = {
  transitionOffEdgesToCenter,
  transitionOffRandomLeds,
  transitionOffLeftToRight,
  transitionOffCenterToEdges
};

void startTransition(uint8_t stripIndex, bool turningOn) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  
  if (trans.active) return;
  
  trans.active = true;
  trans.startTime = millis();
  trans.targetBrightness = state.brightness;
  trans.randomOrder = nullptr;
  trans.randomIndex = 0;
  
  if (turningOn) {
    int index = random(0, NUM_ON_TRANSITIONS);
    trans.type = (TransitionType)index;
    Serial.println("✨ Strip " + String(stripIndex) + " ON transition " + String(index));
  } else {
    int index = random(0, NUM_OFF_TRANSITIONS);
    trans.type = (TransitionType)(NUM_ON_TRANSITIONS + index);
    Serial.println("✨ Strip " + String(stripIndex) + " OFF transition " + String(index));
  }
}

void updateTransition(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  TransitionState& trans = state.transition;
  
  if (!trans.active) return;
  
  // Debug - показваме че работим само с тази лента
  static unsigned long lastDebugTime[NUM_STRIPS] = {0, 0};
  unsigned long now = millis();
  if (now - lastDebugTime[stripIndex] > 200) {
    lastDebugTime[stripIndex] = now;
    // Serial.println("Updating transition for strip " + String(stripIndex));
  }
  
  if (trans.type < NUM_ON_TRANSITIONS) {
    onTransitions[trans.type](stripIndex);
  } else {
    int offIndex = trans.type - NUM_ON_TRANSITIONS;
    offTransitions[offIndex](stripIndex);
  }
  
  // Kitchen: синхронизираме extension лентата - изпълняваме същата транзакция
  if (stripIndex == 0 && stripStates[2].transition.active) {
    TransitionState& extTrans = stripStates[2].transition;
    if (extTrans.type < NUM_ON_TRANSITIONS) {
      onTransitions[extTrans.type](2);
    } else {
      int offIndex = extTrans.type - NUM_ON_TRANSITIONS;
      offTransitions[offIndex](2);
    }
  }
  
  if (!trans.active) {
    if (trans.type < NUM_ON_TRANSITIONS) {
      updateStrip(stripIndex);
      Serial.println("✅ Strip " + String(stripIndex) + " ON transition completed");
    } else {
      clearStrip(stripIndex, RgbwColor(0, 0, 0, 0));
      showStrip(stripIndex);
      Serial.println("✅ Strip " + String(stripIndex) + " OFF transition completed");
    }
  }
}

// ============================================================================
// ПРЕМИГВАНЕ ПРИ МИН/МАКС
// ============================================================================

void updateBlink(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  
  if (!state.blinkActive || !state.on) return;
  
  unsigned long elapsed = millis() - state.blinkStartTime;
  
  if (elapsed < BLINK_DURATION) {
    float progress = (float)elapsed / BLINK_DURATION;
    float sineWave = sin(progress * PI);
    float brightnessFactor = 1.0 - (1.0 - BLINK_MIN_FACTOR) * sineWave;
    uint8_t currentBrightness = (uint8_t)(state.savedBrightnessForBlink * brightnessFactor);
    
    for (int i = 0; i < stripConfigs[stripIndex].ledCount; i++) {
      STRIP_SET_PIXEL(stripIndex, i, RgbwColor(0, 0, 0, currentBrightness));
    }
    STRIP_SHOW(stripIndex);
    
    // Kitchen: синхронизираме extension лентата - използваме същия brightnessFactor
    if (stripIndex == 0 && stripStates[2].blinkActive && stripStates[2].on) {
      StripState& extState = stripStates[2];
      uint8_t extBrightness = (uint8_t)(extState.savedBrightnessForBlink * brightnessFactor);
      for (int i = 0; i < stripConfigs[2].ledCount; i++) {
        STRIP_SET_PIXEL(2, i, RgbwColor(0, 0, 0, extBrightness));
      }
      STRIP_SHOW(2);
    }
  } else {
    state.blinkActive = false;
    state.brightness = state.savedBrightnessForBlink;
    updateStrip(stripIndex);
  }
}

// ============================================================================
// ДИМИРАНЕ
// ============================================================================

void updateDimming(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  
  if (!state.dimmingActive || !state.on) return;
  
  unsigned long elapsed = millis() - state.dimmingStartTime;
  float progress = (float)elapsed / state.dimmingDuration;
  
  if (progress >= 1.0) {
    progress = 1.0;
    state.dimmingActive = false;
  }
  
  uint8_t targetBrightness = state.dimmingDirection ? MAX_BRIGHTNESS : MIN_BRIGHTNESS;
  uint8_t newBrightness = state.dimmingStartBrightness + (int)((targetBrightness - state.dimmingStartBrightness) * progress);
  
  if (newBrightness > MAX_BRIGHTNESS) newBrightness = MAX_BRIGHTNESS;
  if (newBrightness < MIN_BRIGHTNESS) newBrightness = MIN_BRIGHTNESS;
  
  bool reachedLimit = false;
  if (state.dimmingDirection && newBrightness >= MAX_BRIGHTNESS) {
    newBrightness = MAX_BRIGHTNESS;
    reachedLimit = true;
  } else if (!state.dimmingDirection && newBrightness <= MIN_BRIGHTNESS) {
    newBrightness = MIN_BRIGHTNESS;
    reachedLimit = true;
  }
  
  // Премигване само при достигане на MAX, не при MIN
  if (reachedLimit && !state.blinkActive) {
    state.dimmingActive = false;
    state.lastDimmingWasIncrease = state.dimmingDirection;
    
    if (state.dimmingDirection) {
      // Само при увеличаване до MAX - премигване
      state.blinkActive = true;
      state.blinkStartTime = millis();
      state.savedBrightnessForBlink = newBrightness;
      Serial.println("✨ Strip " + String(stripIndex) + " reached MAX brightness - blinking");
    } else {
      // При намаляване до MIN - без премигване
      Serial.println("✨ Strip " + String(stripIndex) + " reached MIN brightness");
    }
    
    state.brightness = newBrightness;
    syncKitchenExtension(stripIndex);
  } else if (!reachedLimit) {
    state.brightness = newBrightness;
    updateStrip(stripIndex);
  }
}

// ============================================================================
// ОСНОВНИ ФУНКЦИИ ЗА УПРАВЛЕНИЕ (извикват се от бутон, MQTT, датчици)
// ============================================================================

void turnOnStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  if (state.on) {
    Serial.println("⚠️ turnOnStrip called for strip " + String(stripIndex) + " but it's already ON");
    return;  // Вече е включена
  }
  
  Serial.println("🔵 turnOnStrip(" + String(stripIndex) + ") - setting state.on = true");
  state.on = true;
  
  // Kitchen: ако управляваме Strip 0, синхронизираме Strip 2 ПРЕДИ да изберем транзакцията
  if (stripIndex == 0) {
    StripState& extState = stripStates[2];
    extState.on = true;
    extState.brightness = state.brightness;
  }
  
  // Избираме транзакцията веднъж и я използваме за двете ленти (ако е Kitchen)
  startTransition(stripIndex, true);
  
  // Kitchen: копираме същата транзакция към extension лентата
  if (stripIndex == 0) {
    StripState& extState = stripStates[2];
    TransitionState& mainTrans = state.transition;
    TransitionState& extTrans = extState.transition;
    
    // Копираме транзакцията от main към extension
    extTrans.active = mainTrans.active;
    extTrans.type = mainTrans.type;
    extTrans.startTime = mainTrans.startTime;
    extTrans.targetBrightness = mainTrans.targetBrightness;
    extTrans.randomOrder = nullptr;  // Extension ще използва същия randomOrder ако е нужно
    extTrans.randomIndex = 0;
    
    Serial.println("💡 Kitchen extension (Strip 2): Turning ON with same transition");
  }
  
  Serial.println("💡 Strip " + String(stripIndex) + ": Turning ON (Brightness: " + String(state.brightness) + ")");
}

void turnOffStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  if (!state.on) return;  // Вече е изключена
  
  state.on = false;
  
  // Kitchen: ако управляваме Strip 0, синхронизираме Strip 2 ПРЕДИ да изберем транзакцията
  if (stripIndex == 0) {
    StripState& extState = stripStates[2];
    extState.on = false;
  }
  
  // Избираме транзакцията веднъж и я използваме за двете ленти (ако е Kitchen)
  startTransition(stripIndex, false);
  
  // Kitchen: копираме същата транзакция към extension лентата
  if (stripIndex == 0) {
    StripState& extState = stripStates[2];
    TransitionState& mainTrans = state.transition;
    TransitionState& extTrans = extState.transition;
    
    // Копираме транзакцията от main към extension
    extTrans.active = mainTrans.active;
    extTrans.type = mainTrans.type;
    extTrans.startTime = mainTrans.startTime;
    extTrans.targetBrightness = mainTrans.targetBrightness;
    extTrans.randomOrder = nullptr;  // Extension ще използва същия randomOrder ако е нужно
    extTrans.randomIndex = 0;
    
    Serial.println("💡 Kitchen extension (Strip 2): Turning OFF with same transition");
  }
  
  Serial.println("💡 Strip " + String(stripIndex) + ": Turning OFF (Saved brightness: " + String(state.brightness) + ")");
}

void toggleStrip(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) {
    Serial.println("ERROR: toggleStrip called with invalid stripIndex: " + String(stripIndex));
    return;
  }
  
  StripState& state = stripStates[stripIndex];
  Serial.println("🔄 toggleStrip(" + String(stripIndex) + ") - current state: " + String(state.on ? "ON" : "OFF"));
  Serial.println("   Strip 0 state: " + String(stripStates[0].on ? "ON" : "OFF") + ", Strip 1 state: " + String(stripStates[1].on ? "ON" : "OFF"));
  Serial.flush();
  
  if (state.on) {
    turnOffStrip(stripIndex);
  } else {
    turnOnStrip(stripIndex);
  }
  
  Serial.println("   After toggle - Strip 0 state: " + String(stripStates[0].on ? "ON" : "OFF") + ", Strip 1 state: " + String(stripStates[1].on ? "ON" : "OFF"));
  Serial.flush();
}

void startDimming(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  if (!state.on || state.dimmingActive) return;
  
  state.dimmingActive = true;
  state.dimmingStartTime = millis();
  state.dimmingStartBrightness = state.brightness;
  state.dimmingDirection = !state.lastDimmingWasIncrease;
  state.lastDimmingWasIncrease = state.dimmingDirection;
  
  // Изчисляваме целевата яркост и времето според разстоянието
  uint8_t targetBrightness = state.dimmingDirection ? MAX_BRIGHTNESS : MIN_BRIGHTNESS;
  uint8_t distance = abs((int)targetBrightness - (int)state.dimmingStartBrightness);
  state.dimmingDuration = (distance * 1000) / DIMMING_SPEED;  // време в милисекунди
  
  Serial.println("🔆 Strip " + String(stripIndex) + " dimming: " + String(state.dimmingDirection ? "Increasing" : "Decreasing") + 
                 " (distance: " + String(distance) + ", time: " + String(state.dimmingDuration) + "ms)");
  
  // Kitchen: синхронизираме extension лентата
  syncKitchenExtension(stripIndex);
}

void stopDimming(uint8_t stripIndex) {
  if (stripIndex >= NUM_STRIPS) return;
  
  StripState& state = stripStates[stripIndex];
  state.dimmingActive = false;
  Serial.println("🔆 Strip " + String(stripIndex) + " dimming stopped (Brightness: " + String(state.brightness) + ")");
  
  // Kitchen: синхронизираме extension лентата
  syncKitchenExtension(stripIndex);
}

// ============================================================================
// SETUP И LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n✨ LED Controller Starting...");
  Serial.println("Number of strips: " + String(NUM_STRIPS));
  
  // Инициализация на лентите - използваме различни RMT канали за всяка лента
  Serial.println("Initializing strip 0 on pin " + String(stripConfigs[0].pin) + " with RMT0...");
  Serial.flush();
  strip0.Begin();
  delay(100);
  strip0.ClearTo(RgbwColor(0, 0, 0, 0));
  strip0.Show();
  stripStates[0].strip = (void*)&strip0;
  stripStates[0].stripType = 0;  // LedStrip0 (RMT0)
  stripStates[0].on = false;
  stripStates[0].brightness = DEFAULT_BRIGHTNESS;
  stripStates[0].dimmingActive = false;
  stripStates[0].dimmingDirection = true;
  stripStates[0].lastDimmingWasIncrease = true;
  stripStates[0].blinkActive = false;
  stripStates[0].transition.active = false;
  stripStates[0].transition.randomOrder = nullptr;
  Serial.println("Strip 0 - Pin: " + String(stripConfigs[0].pin) + ", LEDs: " + String(stripConfigs[0].ledCount) + " - OK (RMT0)");
  
  Serial.println("Initializing strip 1 on pin " + String(stripConfigs[1].pin) + " with RMT1...");
  Serial.flush();
  strip1.Begin();
  delay(100);
  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  strip1.Show();
  stripStates[1].strip = (void*)&strip1;
  stripStates[1].stripType = 1;  // LedStrip1 (RMT1)
  stripStates[1].on = false;
  stripStates[1].brightness = DEFAULT_BRIGHTNESS;
  stripStates[1].dimmingActive = false;
  stripStates[1].dimmingDirection = true;
  stripStates[1].lastDimmingWasIncrease = true;
  stripStates[1].blinkActive = false;
  stripStates[1].transition.active = false;
  stripStates[1].transition.randomOrder = nullptr;
  Serial.println("Strip 1 - Pin: " + String(stripConfigs[1].pin) + ", LEDs: " + String(stripConfigs[1].ledCount) + " - OK (RMT1)");
  
  Serial.println("Initializing strip 2 on pin " + String(stripConfigs[2].pin) + " with RMT2 (Kitchen extension, synced with Strip 0)...");
  Serial.flush();
  strip2.Begin();
  delay(100);
  strip2.ClearTo(RgbwColor(0, 0, 0, 0));
  strip2.Show();
  stripStates[2].strip = (void*)&strip2;
  stripStates[2].stripType = 2;  // LedStrip2 (RMT2), синхронизиран с Strip 0 в софтуера
  stripStates[2].on = false;
  stripStates[2].brightness = DEFAULT_BRIGHTNESS;
  stripStates[2].dimmingActive = false;
  stripStates[2].dimmingDirection = true;
  stripStates[2].lastDimmingWasIncrease = true;
  stripStates[2].blinkActive = false;
  stripStates[2].transition.active = false;
  stripStates[2].transition.randomOrder = nullptr;
  Serial.println("Strip 2 - Pin: " + String(stripConfigs[2].pin) + ", LEDs: " + String(stripConfigs[2].ledCount) + " - OK (RMT2) Kitchen extension");
  
  Serial.println("Dimming speed: " + String(DIMMING_SPEED) + " units/sec, Hold threshold: " + String(HOLD_THRESHOLD) + "ms");
  Serial.println("Transitions: " + String(TRANSITION_DURATION) + "ms");
  
  // Инициализация на бутоните
  Serial.println("Initializing buttons...");
  Serial.flush();
  
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
    Serial.println("Button " + String(i) + " - Pin: " + String(buttons[i].pin) + " -> Strip " + String(buttons[i].stripIndex));
    Serial.flush();
  }
  
  randomSeed(analogRead(0));
  
  Serial.println("✅ System ready!");
  Serial.println("Click: Toggle strip ON/OFF (with random transitions)");
  Serial.println("Hold: Dim/Increase brightness\n");
}

void loop() {
  unsigned long currentTime = millis();
  
  
  // Обработка на всички бутони
  for (int btnIndex = 0; btnIndex < NUM_BUTTONS; btnIndex++) {
    ButtonStateMachine& btn = buttons[btnIndex];
    uint8_t stripIndex = btn.stripIndex;
    
    // Четем състоянието на бутона
    bool rawButtonReading = (digitalRead(btn.pin) == LOW);
    
    // Debounce логика (отделна за всеки бутон)
    const unsigned long DEBOUNCE_DELAY = 50;
    
    if (rawButtonReading != btn.lastRawReading) {
      btn.lastDebounceTime = currentTime;
    }
    
    if (currentTime - btn.lastDebounceTime > DEBOUNCE_DELAY) {
      btn.debouncedState = rawButtonReading;
    }
    
    btn.lastRawReading = rawButtonReading;
    bool debouncedButtonState = btn.debouncedState;
    
    // State machine за бутона
    switch (btn.state) {
      case BUTTON_IDLE:
        if (debouncedButtonState) {
          btn.state = BUTTON_PRESSED;
          btn.pressTime = currentTime;
          Serial.println("🔘 Button " + String(btnIndex) + " pressed (IDLE -> PRESSED)");
        }
        break;
        
      case BUTTON_PRESSED:
        if (debouncedButtonState) {
          if (currentTime - btn.pressTime >= HOLD_THRESHOLD) {
            btn.state = BUTTON_HELD;
            if (stripStates[stripIndex].on) {
              startDimming(stripIndex);
            }
          }
        } else {
          btn.state = BUTTON_IDLE;
          Serial.println("🔘 Button " + String(btnIndex) + " released - toggling strip " + String(stripIndex));
          Serial.flush();
          toggleStrip(stripIndex);
        }
        break;
        
      case BUTTON_HELD:
        if (!debouncedButtonState) {
          btn.state = BUTTON_IDLE;
          stopDimming(stripIndex);
        }
        break;
    }
  }
  
  // Обновяване на всички ленти
  for (int i = 0; i < NUM_STRIPS; i++) {
    if (stripStates[i].transition.active) {
      updateTransition(i);
    } else {
      updateDimming(i);
      updateBlink(i);
    }
  }
  
  delay(10);
}
