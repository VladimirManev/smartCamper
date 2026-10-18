# AUTOTERM Air 2D — transparent bridge, етап 2

ESP32 е сериен proxy между Comfort Control и ECU на Air 2D. Препраща байтовете без промяна и в двете посоки. Логва трафика по USB.

Горивната помпа, вентилаторът, подгревната свещ, запалването, гасенето и температурните защити остават в оригиналното ECU. Този firmware не инжектира команди.

Потвърдено на тази двойка: **9600 8N1**, кадри `AA 03` (панел) / `AA 04` (печка), CRC16 Modbus.

## Връзване (мост)

Директните TX↔RX между панел и печка са **разкачени**. ESP32 ги свързва през два ADuM1201:

```
Comfort Control TX ---- ADuM#1 VIA (7) ---- VOA (2) ---- ESP32 GPIO 34 RX
Comfort Control RX ---- ADuM#1 VOB (6) ---- VIB (3) ---- ESP32 GPIO 27 TX

Air 2D TX ------------- ADuM#2 VIA (7) ---- VOA (2) ---- ESP32 GPIO 32 RX
Air 2D RX ------------- ADuM#2 VOB (6) ---- VIB (3) ---- ESP32 GPIO 33 TX
```

Захранване: `+12 V` и `GND` на панела/печката си остават.  
ADuM: VDD1 = 3.3 V, VDD2 = 5 V от ESP, общ GND (или изолирана 5 V схема, ако ползваш такава).

| Порт | UART | RX | TX |
|------|------|----|----|
| PANEL | UART1 | GPIO 34 | GPIO 27 |
| HEATER | UART2 | GPIO 32 | GPIO 33 |

## Компилация

```bash
cd esp32-modules/autoterm-sniffer
pio run -t upload
pio device monitor
```

## Какво да видиш

```text
AUTOTERM Air 2D — stage 2 transparent bridge
PANEL -> HEATER (7): AA 03 00 00 0F 58 7C
HEATER -> PANEL (26): AA 04 13 00 0F ...
```

Comfort Control трябва да работи нормално (статус, температури, бутони). Ако пише „няма връзка“, провери TX пътищата на ADuM (VIB→VOB) и масата.

## USB команди

`help`, `status`, `pins`, `baud`, `serial`, `gap`, `mode` — както преди. Не инжектират AUTOTERM команди; само настройки на моста/лога.

## Следващ етап

Command injection — ESP праща записани `AA 03 …` пакети. Все още не е в този firmware.
