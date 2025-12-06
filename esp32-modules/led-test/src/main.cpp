// LED Strip Control with Button
// Пълна функционалност за управление на множество LED ленти чрез бутон

#include <Arduino.h>
#include <NeoPixelBus.h>

// Параметри
#define LED_PIN_1 2
#define LED_COUNT_1 44
#define LED_PIN_2 5
#define LED_COUNT_2 54
#define BUTTON_PIN 4 // Променен от 35 (не поддържа pull-up) на 4

#define BUTTON_DEBOUNCE_TIME 50
#define BUTTON_SHORT_PRESS_MAX 200
#define BUTTON_LONG_PRESS_TIME 1000
#define BUTTON_IDLE_TIMEOUT 3000
#define VISUAL_INDICATOR_INTERVAL 500
#define VISUAL_INDICATOR_STEP 10
#define DIMMING_TIME 5000
#define DIMMING_UPDATE_INTERVAL 20
#define MIN_BRIGHTNESS 10
#define MAX_BRIGHTNESS 255
#define DEFAULT_BRIGHTNESS 150
#define INACTIVE_STRIP_BRIGHTNESS 128
#define WIPE_EFFECT_TIME 1000
#define WIPE_HOLD_TIME 2000

// Състояния на системата
enum SystemMode
{
  NORMAL_MODE,
  CONTROL_MODE
};

// Състояния на бутона
enum ButtonState
{
  BUTTON_IDLE,
  BUTTON_PRESSED,
  BUTTON_HELD,
  BUTTON_RELEASED
};

// Структура за лента
struct StripData
{
  NeoPixelBus<NeoRgbwFeature, NeoWs2814Method> *strip;
  int ledCount;
  bool isOn;
  bool isMarked;
  uint8_t currentBrightness;
  uint8_t targetBrightness;
  unsigned long lastVisualUpdate;
  int visualColorIndex;
  bool lastDimDirection; // true = увеличаване, false = димиране
};

// Глобални променливи
NeoPixelBus<NeoRgbwFeature, NeoWs2814Method> strip1(LED_COUNT_1, LED_PIN_1);
NeoPixelBus<NeoRgbwFeature, NeoWs2814Method> strip2(LED_COUNT_2, LED_PIN_2);

StripData strips[] = {
    {&strip1, LED_COUNT_1, false, false, DEFAULT_BRIGHTNESS, DEFAULT_BRIGHTNESS, 0, 0, false},
    {&strip2, LED_COUNT_2, false, false, DEFAULT_BRIGHTNESS, DEFAULT_BRIGHTNESS, 0, 0, false}};

const int STRIP_COUNT = 2;
int markedStripIndex = -1;
SystemMode currentMode = NORMAL_MODE;
unsigned long controlModeActivatedTime = 0;

// Бутон променливи
ButtonState buttonState = BUTTON_IDLE;
unsigned long buttonPressTime = 0;
unsigned long buttonReleaseTime = 0;
unsigned long lastButtonActivity = 0;

// Глобални променливи за димиране
unsigned long dimmingStartTime = 0;
uint8_t dimmingStartBrightness = 0;
bool dimmingActive = false;

// Помощна функция - R и G са разменени в хардуера
RgbwColor fixColor(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
  return RgbwColor(g, r, b, w);
}

// Функция за получаване на цвят според позицията
RgbwColor getColorForPosition(int position, uint8_t brightness)
{
  // Връщаме бяло на зададената яркост
  return RgbwColor(0, 0, 0, brightness);
}

// Функция за получаване на RGB цвят за визуален индикатор
RgbwColor getVisualIndicatorColor(int colorIndex, uint8_t brightness)
{
  switch (colorIndex % 3)
  {
  case 0: // Червено
    return fixColor(brightness, 0, 0, 0);
  case 1: // Зелено
    return fixColor(0, brightness, 0, 0);
  case 2: // Синьо
    return RgbwColor(0, 0, brightness, 0);
  default:
    return RgbwColor(0, 0, 0, 0);
  }
}

// Проверка дали има светещи ленти
bool hasAnyStripOn()
{
  for (int i = 0; i < STRIP_COUNT; i++)
  {
    if (strips[i].isOn)
      return true;
  }
  return false;
}

// Намиране на първата светеща лента
int findFirstOnStrip()
{
  for (int i = 0; i < STRIP_COUNT; i++)
  {
    if (strips[i].isOn)
      return i;
  }
  return -1;
}

// Маркиране на лента
void markStrip(int index)
{
  // Премахваме маркировката от всички ленти
  for (int i = 0; i < STRIP_COUNT; i++)
  {
    strips[i].isMarked = false;
    strips[i].visualColorIndex = 0;
  }

  // Маркираме новата лента
  if (index >= 0 && index < STRIP_COUNT)
  {
    strips[index].isMarked = true;
    strips[index].visualColorIndex = 0;
    strips[index].lastVisualUpdate = millis();

    // Ако лентата не свети, светваме я на 50% бяло
    if (!strips[index].isOn)
    {
      strips[index].isOn = true;
      strips[index].currentBrightness = INACTIVE_STRIP_BRIGHTNESS;
      strips[index].targetBrightness = INACTIVE_STRIP_BRIGHTNESS;
    }

    markedStripIndex = index;
  }
}

// Прилагане на визуален индикатор
void applyVisualIndicator(StripData &strip)
{
  if (!strip.isMarked)
    return;

  unsigned long currentTime = millis();
  if (currentTime - strip.lastVisualUpdate >= VISUAL_INDICATOR_INTERVAL)
  {
    strip.visualColorIndex = (strip.visualColorIndex + 1) % 3;
    strip.lastVisualUpdate = currentTime;
  }

  // Прилагаме визуалния индикатор на всеки 10-ти диод
  for (int i = 0; i < strip.ledCount; i++)
  {
    if (i % VISUAL_INDICATOR_STEP == 0)
    {
      // Визуален индикатор (RGB)
      strip.strip->SetPixelColor(i, getVisualIndicatorColor(strip.visualColorIndex, strip.currentBrightness));
    }
    else
    {
      // Нормални цветове
      if (strip.isOn)
      {
        if (strip.currentBrightness == INACTIVE_STRIP_BRIGHTNESS)
        {
          // Бяло за неактивни ленти
          strip.strip->SetPixelColor(i, RgbwColor(0, 0, 0, strip.currentBrightness));
        }
        else
        {
          // Нормални цветове
          strip.strip->SetPixelColor(i, getColorForPosition(i, strip.currentBrightness));
        }
      }
      else
      {
        strip.strip->SetPixelColor(i, RgbwColor(0, 0, 0, 0));
      }
    }
  }
}

// Прилагане на нормални цветове (без визуален индикатор)
void applyNormalColors(StripData &strip)
{
  for (int i = 0; i < strip.ledCount; i++)
  {
    if (strip.isOn)
    {
      if (strip.currentBrightness == INACTIVE_STRIP_BRIGHTNESS)
      {
        // Бяло за неактивни ленти
        strip.strip->SetPixelColor(i, RgbwColor(0, 0, 0, strip.currentBrightness));
      }
      else
      {
        // Нормални цветове
        strip.strip->SetPixelColor(i, getColorForPosition(i, strip.currentBrightness));
      }
    }
    else
    {
      strip.strip->SetPixelColor(i, RgbwColor(0, 0, 0, 0));
    }
  }
}

// Обновяване на лентите
void updateStrips()
{
  for (int i = 0; i < STRIP_COUNT; i++)
  {
    if (strips[i].isMarked && currentMode == CONTROL_MODE && !dimmingActive)
    {
      applyVisualIndicator(strips[i]);
    }
    else
    {
      applyNormalColors(strips[i]);
    }
    strips[i].strip->Show();
  }
}

// Wipe ефект от центъра
void applyWipeEffect(StripData &strip, bool expanding)
{
  int center = strip.ledCount / 2;
  int maxDistance = center;

  unsigned long startTime = millis();
  while (millis() - startTime < WIPE_EFFECT_TIME)
  {
    float progress = (float)(millis() - startTime) / WIPE_EFFECT_TIME;
    int currentDistance;

    if (expanding)
    {
      currentDistance = (int)(maxDistance * progress);
    }
    else
    {
      currentDistance = (int)(maxDistance * (1.0 - progress));
    }

    strip.strip->ClearTo(RgbwColor(0, 0, 0, 0));

    for (int i = 0; i <= currentDistance; i++)
    {
      if (center - i >= 0)
      {
        strip.strip->SetPixelColor(center - i, getColorForPosition(center - i, strip.currentBrightness));
      }
      if (center + i < strip.ledCount)
      {
        strip.strip->SetPixelColor(center + i, getColorForPosition(center + i, strip.currentBrightness));
      }
    }

    strip.strip->Show();
    delay(20);
  }
}

// Включване на всички ленти
void turnOnAllStrips()
{
  for (int i = 0; i < STRIP_COUNT; i++)
  {
    strips[i].isOn = true;
    strips[i].currentBrightness = INACTIVE_STRIP_BRIGHTNESS; // 50% бяло
    strips[i].targetBrightness = INACTIVE_STRIP_BRIGHTNESS;
    applyWipeEffect(strips[i], true);
  }

  for (int i = 0; i < STRIP_COUNT; i++)
  {
    applyNormalColors(strips[i]);
    strips[i].strip->Show();
  }
}

// Изключване на всички ленти
void turnOffAllStrips()
{
  // Първо прилагаме wipe ефект (събиране към центъра)
  for (int i = 0; i < STRIP_COUNT; i++)
  {
    applyWipeEffect(strips[i], false); // false = събиране към центъра
  }
  
  // След това изключваме лентите
  for (int i = 0; i < STRIP_COUNT; i++)
  {
    strips[i].isOn = false;
    strips[i].isMarked = false;
    applyNormalColors(strips[i]);
    strips[i].strip->Show();
  }
  markedStripIndex = -1;
}

// Превключване на маркираната лента
void switchMarkedStrip()
{
  if (markedStripIndex < 0)
  {
    // Ако няма маркирана, маркираме първата
    markStrip(0);
  }
  else
  {
    // Превключваме към следващата
    int nextIndex = (markedStripIndex + 1) % STRIP_COUNT;
    markStrip(nextIndex);
  }
}

// Димиране/увеличаване на маркираната лента (неблокиращо)
void startDimming(bool increase)
{
  if (markedStripIndex < 0)
    return;

  StripData &strip = strips[markedStripIndex];
  strip.lastDimDirection = increase;
  dimmingStartTime = millis();
  dimmingStartBrightness = strip.currentBrightness;
  dimmingActive = true;
}

// Обновяване на димиране (извиква се в loop)
void updateDimming()
{
  if (!dimmingActive || markedStripIndex < 0)
    return;
  if (buttonState != BUTTON_HELD)
  {
    dimmingActive = false;
    return;
  }

  StripData &strip = strips[markedStripIndex];
  uint8_t endBrightness = strip.lastDimDirection ? MAX_BRIGHTNESS : MIN_BRIGHTNESS;

  unsigned long elapsed = millis() - dimmingStartTime;
  float progress = (float)elapsed / DIMMING_TIME;

  if (progress >= 1.0)
  {
    progress = 1.0;
    dimmingActive = false;
  }

  uint8_t newBrightness = dimmingStartBrightness + (int)((endBrightness - dimmingStartBrightness) * progress);

  if (strip.lastDimDirection && newBrightness >= MAX_BRIGHTNESS)
  {
    newBrightness = MAX_BRIGHTNESS;
    dimmingActive = false;
  }
  else if (!strip.lastDimDirection && newBrightness <= MIN_BRIGHTNESS)
  {
    newBrightness = MIN_BRIGHTNESS;
    dimmingActive = false;
  }

  strip.currentBrightness = newBrightness;
  strip.targetBrightness = newBrightness;
}

// Обработка на бутона
void handleButton()
{
  bool currentReading = digitalRead(BUTTON_PIN);
  unsigned long currentTime = millis();

  // Много прост debounce - използваме директно четенето с малко забавяне
  static unsigned long lastDebounceTime = 0;
  static bool lastStableReading = HIGH;
  static bool lastReading = HIGH;

  // Ако има промяна в четенето, рестартираме debounce таймера
  if (currentReading != lastReading)
  {
    lastDebounceTime = currentTime;
  }

  lastReading = currentReading;

  // Ако е изминало достатъчно време след последната промяна, приемаме текущото състояние като стабилно
  bool stableReading = lastStableReading;
  if (currentTime - lastDebounceTime >= BUTTON_DEBOUNCE_TIME)
  {
    stableReading = currentReading;
    if (lastStableReading != stableReading)
    {
      Serial.println("✅ Button stable state: " + String(stableReading ? "HIGH" : "LOW") +
                     " (was: " + String(lastStableReading ? "HIGH" : "LOW") + ")");
      lastStableReading = stableReading;
    }
  }

  bool buttonPressed = (stableReading == LOW);

  switch (buttonState)
  {
  case BUTTON_IDLE:
    if (buttonPressed)
    {
      buttonState = BUTTON_PRESSED;
      buttonPressTime = currentTime;
      lastButtonActivity = currentTime;
      Serial.println("🔘 Button pressed (IDLE -> PRESSED)");
      Serial.println("   Time since last debounce: " + String(currentTime - lastDebounceTime) + "ms");
    }
    else
    {
      // Debug - периодично показване на състоянието в IDLE
      static unsigned long lastIdleDebug = 0;
      if (currentTime - lastIdleDebug > 5000)
      {
        lastIdleDebug = currentTime;
        Serial.println("💤 IDLE: buttonPressed=" + String(buttonPressed ? "true" : "false") +
                       ", stableReading=" + String(stableReading ? "HIGH" : "LOW") +
                       ", currentReading=" + String(currentReading ? "HIGH" : "LOW"));
      }
    }
    break;

  case BUTTON_PRESSED:
    if (buttonPressed)
    {
      unsigned long holdTime = currentTime - buttonPressTime;

      if (holdTime >= BUTTON_LONG_PRESS_TIME)
      {
        // Дълго натискане - активиране на режим на управление
        buttonState = BUTTON_HELD;

        if (currentMode == NORMAL_MODE)
        {
          currentMode = CONTROL_MODE;
          controlModeActivatedTime = currentTime;
          int stripToMark = findFirstOnStrip();
          if (stripToMark < 0)
          {
            // Нито една не свети, маркираме произволна
            stripToMark = 0;
          }
          markStrip(stripToMark);
          Serial.println("🔧 Control mode activated, strip " + String(stripToMark) + " marked");
        }
      }
    }
    else
    {
      // Отпускане преди дълго натискане
      unsigned long pressDuration = currentTime - buttonPressTime;

      // Проверяваме дали е кратко натискане (не е дълго задържане)
      if (pressDuration < BUTTON_LONG_PRESS_TIME)
      {
        if (pressDuration < BUTTON_SHORT_PRESS_MAX)
        {
          // Много кратко натискане - превключване в режим на управление
          if (currentMode == CONTROL_MODE)
          {
            switchMarkedStrip();
            Serial.println("🔄 Switched to strip " + String(markedStripIndex));
          }
        }
        else
        {
          // Единично натискане в нормален режим
          if (currentMode == NORMAL_MODE)
          {
            if (hasAnyStripOn())
            {
              turnOffAllStrips();
              Serial.println("💡 All strips turned off");
            }
            else
            {
              turnOnAllStrips();
              Serial.println("💡 All strips turned on");
            }
          }
        }
      }

      buttonState = BUTTON_IDLE;
      buttonReleaseTime = currentTime;
      lastButtonActivity = currentTime;
    }
    break;

  case BUTTON_HELD:
    if (!buttonPressed)
    {
      // Отпускане след дълго натискане
      buttonState = BUTTON_RELEASED;
      buttonReleaseTime = currentTime;
      lastButtonActivity = currentTime;
      dimmingActive = false; // Спираме димиране при отпускане
    }
    else
    {
      // Все още задържаме - започваме димиране ако вече сме в режим на управление
      // Димирането се обновява в updateDimming() в loop()
      if (currentMode == CONTROL_MODE && markedStripIndex >= 0 && !dimmingActive)
      {
        // Започваме димиране надолу след малко време след активиране на режима
        unsigned long timeSinceModeActivation = currentTime - controlModeActivatedTime;
        if (timeSinceModeActivation > 100)
        {
          // Дадохме малко време след активиране на режима
          startDimming(false);
        }
      }
    }
    break;

  case BUTTON_RELEASED:
    if (buttonPressed)
    {
      // Пак натискане след отпускане - обратна посока
      buttonState = BUTTON_HELD;
      buttonPressTime = currentTime;
      lastButtonActivity = currentTime;

      if (currentMode == CONTROL_MODE && markedStripIndex >= 0)
      {
        StripData &strip = strips[markedStripIndex];
        // Обратна посока на димиране
        startDimming(!strip.lastDimDirection);
      }
    }
    else
    {
      buttonState = BUTTON_IDLE;
      dimmingActive = false;
    }
    break;
  }

  // Проверка за излизане от режим на управление
  if (currentMode == CONTROL_MODE)
  {
    unsigned long idleTime = currentTime - lastButtonActivity;
    if (idleTime >= BUTTON_IDLE_TIMEOUT)
    {
      currentMode = NORMAL_MODE;
      controlModeActivatedTime = 0;
      dimmingActive = false;
      if (markedStripIndex >= 0)
      {
        strips[markedStripIndex].isMarked = false;
        strips[markedStripIndex].visualColorIndex = 0;
      }
      markedStripIndex = -1;
      Serial.println("🏠 Exited control mode");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n✨ LED Strip Control System Starting...");
  Serial.println("Strip 1 - LED Count: " + String(LED_COUNT_1) + ", Pin: " + String(LED_PIN_1));
  Serial.println("Strip 2 - LED Count: " + String(LED_COUNT_2) + ", Pin: " + String(LED_PIN_2));
  Serial.println("Button Pin: " + String(BUTTON_PIN));

  strip1.Begin();
  strip2.Begin();
  delay(100);

  strip1.ClearTo(RgbwColor(0, 0, 0, 0));
  strip2.ClearTo(RgbwColor(0, 0, 0, 0));
  strip1.Show();
  strip2.Show();
  delay(500);

  // Конфигурираме бутона с pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  delay(100);

  // Тест на бутона - чете се няколко пъти
  Serial.println("Testing button pin " + String(BUTTON_PIN) + "...");
  for (int i = 0; i < 5; i++)
  {
    bool btnState = digitalRead(BUTTON_PIN);
    Serial.println("  Reading " + String(i + 1) + ": " + String(btnState == LOW ? "LOW (PRESSED)" : "HIGH (NOT PRESSED)"));
    delay(100);
  }

  Serial.println("✅ System ready!");
  Serial.println("Press button to control strips");
  Serial.println("Current mode: " + String(currentMode == NORMAL_MODE ? "NORMAL" : "CONTROL"));
  Serial.println("Any strips on: " + String(hasAnyStripOn() ? "YES" : "NO") + "\n");
}

void loop()
{
  static unsigned long lastDebugTime = 0;
  unsigned long currentTime = millis();

  handleButton();
  updateDimming();
  updateStrips();

  // Debug съобщение на всеки 2 секунди
  if (currentTime - lastDebugTime > 2000)
  {
    lastDebugTime = currentTime;
    bool btnState = digitalRead(BUTTON_PIN);
    int btnValue = digitalRead(BUTTON_PIN); // Четем като int за по-добра диагностика
    Serial.println("Debug - Button pin " + String(BUTTON_PIN) + " value: " + String(btnValue) +
                   " (" + String(btnState == LOW ? "LOW/PRESSED" : "HIGH/NOT PRESSED") + ")" +
                   ", Mode: " + String(currentMode == NORMAL_MODE ? "NORMAL" : "CONTROL") +
                   ", Strips on: " + String(hasAnyStripOn() ? "YES" : "NO") +
                   ", Marked: " + String(markedStripIndex >= 0 ? String(markedStripIndex) : "NONE") +
                   ", ButtonState: " + String(buttonState));
  }

  delay(10);
}
