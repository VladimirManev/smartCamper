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
#define MIN_BRIGHTNESS 5
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 128  // 50% при първо включване

// Димиране настройки
#define DIMMING_TIME 4000  // 4 секунди от мин до макс
#define HOLD_THRESHOLD 250  // 250ms преди да започне димиране (за да не се случи при click)

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

// Помощна функция - R и G са разменени в хардуера
RgbwColor fixColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
  return RgbwColor(g, r, b, w);  // Разменяме R и G
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
  if ((dimmingDirection && newBrightness >= MAX_BRIGHTNESS) || 
      (!dimmingDirection && newBrightness <= MIN_BRIGHTNESS)) {
    dimmingActive = false;
    // Запомняме посоката за следващо задържане
    lastDimmingWasIncrease = dimmingDirection;
  }
  
  strip1Brightness = newBrightness;
  updateStrip1();
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n✨ LED Controller Starting...");
  Serial.println("Strip 1 - Pin: " + String(LED_PIN_1) + ", LEDs: " + String(LED_COUNT_1));
  Serial.println("Button 1 - Pin: " + String(BUTTON_PIN_1));
  Serial.println("Dimming: " + String(DIMMING_TIME) + "ms, Hold threshold: " + String(HOLD_THRESHOLD) + "ms");
  
  // Инициализация на лентата
  strip1.Begin();
  delay(100);
  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  strip1.Show();
  
  // Инициализация на бутона
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  
  Serial.println("✅ System ready!");
  Serial.println("Click: Toggle ON/OFF");
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
          // Включваме с последната запаметена яркост
          updateStrip1();
          Serial.println("💡 Strip 1: ON (Brightness: " + String(strip1Brightness) + ")");
        } else {
          // Изключваме - запазваме текущата яркост
          updateStrip1();
          Serial.println("💡 Strip 1: OFF (Saved brightness: " + String(strip1Brightness) + ")");
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
  
  // Обновяване на димиране
  updateDimming();
  
  delay(10);
}

