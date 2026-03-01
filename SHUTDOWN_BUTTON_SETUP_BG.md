# Настройка на Shutdown Бутон за Raspberry Pi

## Описание

Хардуерен бутон за безопасно изключване на Raspberry Pi. При задържане за 2 секунди изпълнява shutdown команда. LED индикатор показва че системата работи.

## Хардуерна Конфигурация

### Необходими Компоненти

- 1x Тактилен бутон (momentary push button)
- 1x LED диод
- 1x Резистор 220-330Ω (за LED)
- Jumper wires

### Схема на Свързване

#### Бутон
- **Един край** → GPIO 3 (физически пин 5)
- **Друг край** → GND (физически пин 6, 9, 14, 20, 25, 30, 34, или 39)

**Забележка:** GPIO 3 има вграден pull-up резистор, не е нужен външен резистор.

#### LED Индикация
- **Анод (дълга ножка)** → GPIO 4 (физически пин 7) чрез резистор 220-330Ω
- **Катод (къса ножка)** → GND

**Важно:** LED трябва да има ограничаващ резистор! Без резистор може да се повреди GPIO пинът.

### Физически Пинове

```
    3.3V  [1]  [2]  5V
   GPIO2  [3]  [4]  5V
   GPIO3  [5]  [6]  GND  ← Бутон тук
   GPIO4  [7]  [8]  GPIO14  ← LED тук
     GND  [9]  [10] GPIO15
```

**Използвани пинове:**
- **GPIO 3 (физически пин 5)** - Бутон
- **GPIO 4 (физически пин 7)** - LED

## Софтуерна Инсталация

### Стъпка 1: Качване на файловете

Файловете се качват автоматично с `deploy-to-raspberry.sh` или ръчно:
- `shutdown-button.py` → `~/smartCamper/shutdown-button.py`
- `shutdown-button.service` → `~/smartCamper/shutdown-button.service`
- `install-shutdown-button.sh` → `~/smartCamper/install-shutdown-button.sh`

### Стъпка 2: Инсталация

```bash
cd ~/smartCamper
chmod +x install-shutdown-button.sh
./install-shutdown-button.sh
```

Скриптът автоматично:
- Инсталира RPi.GPIO (ако липсва)
- Настройва права
- Инсталира systemd service
- Стартира service

### Стъпка 3: Проверка

```bash
# Статус
sudo systemctl status shutdown-button.service

# Логове
sudo journalctl -u shutdown-button.service -f
```

## Как Работи

1. **LED Индикация:**
   - LED свети когато Pi е включен и service работи
   - LED се изключва преди shutdown
   - LED не свети когато Pi е изключен (GPIO без захранване)

2. **Shutdown Бутон:**
   - Натискане за < 2 секунди - няма действие
   - Задържане за 2+ секунди - изпълнява `sudo shutdown -h now`

3. **Безопасност:**
   - Защита срещу случайни натискания (2 секунди задържане)
   - Логиране на всички действия

## Решаване на Проблеми

### LED свети дори когато Pi е shutdown

**Причина:** LED е свързан директно към 3.3V/5V вместо към GPIO пин.

**Решение:** Свържи LED към GPIO 4 (физически пин 7) с ограничаващ резистор. GPIO пиновете се изключват при shutdown.

### Бутонът не работи

**Проверки:**
1. Провери свързването на бутона
2. Провери статуса: `sudo systemctl status shutdown-button.service`
3. Провери логовете: `sudo journalctl -u shutdown-button.service -n 50`

### Service не стартира

**Проверки:**
1. Провери пътя в service файла
2. Провери дали скриптът е изпълним: `ls -l ~/smartCamper/shutdown-button.py`
3. Провери дали RPi.GPIO е инсталиран: `pip3 list | grep RPi.GPIO`

## Промяна на Конфигурация

За промяна на GPIO пинове или време за задържане, редактирай `shutdown-button.py`:

```python
BUTTON_PIN = 3  # Промени GPIO пин за бутон
LED_PIN = 4     # Промени GPIO пин за LED
SHUTDOWN_HOLD_TIME = 2.0  # Промени времето в секунди
```

След промяна:
```bash
sudo systemctl restart shutdown-button.service
```

## Премахване

```bash
# Спиране
sudo systemctl stop shutdown-button.service

# Деактивиране
sudo systemctl disable shutdown-button.service

# Премахване
sudo rm /etc/systemd/system/shutdown-button.service
sudo systemctl daemon-reload
```
