# Библиотеки

Здесь находятся необходимые библиотеки для проекта 24JAMMER:

- **RF24** — библиотека для работы с модулями nRF24L01+
- **Arduino ESP32** — встроенная поддержка ESP32 в Arduino IDE

## Установка в Arduino IDE

1. Sketch → Include Library → Manage Libraries...
2. Поиск: `RF24` (автор TMRh20)
3. Установить последнюю версию
4. Для ESP32: Boards → Install "esp32" by Espressif Systems

## Установка в PlatformIO

Добавить в `platformio.ini`:
```ini
lib_deps = 
    https://github.com/nRF24/RF24.git
```
