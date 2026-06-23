#include "RF24.h"
#include <SPI.h>
#include "esp_bt.h"
#include "esp_wifi.h"

#define CE1  16
#define CSN1 15
#define CE2  18
#define CSN2 17

#define LED_PIN 2       
#define BTN_PIN 0       
#define REG_RF_CH 0x05  

SPIClass *vSpi = nullptr;

RF24 nrf1(CE1, CSN1, 16000000);   
RF24 nrf2(CE2, CSN2, 16000000);  

int ch1 = 2;   
int ch2 = 82;  
int mode = 0;  

unsigned long btnTimer = 0;
bool btnState = false;

const uint8_t wf_l[] = {12, 17, 22, 27, 32, 37, 40}; 
const uint8_t wf_h[] = {42, 47, 52, 57, 62, 67, 72}; 

const uint8_t ble_l[] = {2, 12, 26, 38}; 
const uint8_t ble_h[] = {40, 50, 65, 80};

void handleButton() {
  if (digitalRead(BTN_PIN) == LOW) {
    if (!btnState && (millis() - btnTimer > 350)) {
      btnState = true;
      btnTimer = millis();
      mode = (mode + 1) % 5; 
      
      Serial.print("New mode: ");
      Serial.println(mode);
    }
  } else {
    btnState = false;
  }
}

void updateLed() {
  unsigned long t = millis();
  int pulse;
  
  switch (mode) {
    case 0: 
      digitalWrite(LED_PIN, HIGH); 
      break;
    case 1: 
      digitalWrite(LED_PIN, (t / 400) % 2); 
      break;
    case 2: 
      digitalWrite(LED_PIN, (t / 80) % 2); 
      break;
    case 3: 
      pulse = (t % 800) / 100; 
      digitalWrite(LED_PIN, (pulse == 0 || pulse == 2) ? HIGH : LOW); 
      break;
    case 4: 
      pulse = (t % 1000) / 100; 
      digitalWrite(LED_PIN, (pulse == 0 || pulse == 2 || pulse == 4) ? HIGH : LOW); 
      break;
  }
}

inline void setChannels(uint8_t c1, uint8_t c2) {
  digitalWrite(CE1, LOW);
  digitalWrite(CE2, LOW);
  delayMicroseconds(2); 

  digitalWrite(CSN1, LOW);
  vSpi->transfer((REG_RF_CH & 0x1F) | 0x20);
  vSpi->transfer(c1 & 0x7F);              
  digitalWrite(CSN1, HIGH);

  digitalWrite(CSN2, LOW);
  vSpi->transfer((REG_RF_CH & 0x1F) | 0x20);
  vSpi->transfer(c2 & 0x7F);              
  digitalWrite(CSN2, HIGH);

  digitalWrite(CE1, HIGH);
  digitalWrite(CE2, HIGH);

  delayMicroseconds(135); 
}

void runFullJam() {
  ch1 += 2; 
  ch2 -= 2; 
  
  if (ch1 > 42 || ch2 < 42) { 
    ch1 = 2;   
    ch2 = 82; 
  }
  setChannels(ch1, ch2);
}

void runBtJam() {
  static int bCh = 2;
  bCh++;
  if (bCh > 42) bCh = 2;
  setChannels(bCh, bCh + 40); 
}

void runBleJam() {
  static uint8_t idx = 0;
  setChannels(ble_l[idx], ble_h[idx]);
  
  idx++;
  if (idx >= 4) idx = 0; 
}

void runWifiJam() {
  static uint8_t idx = 0;
  setChannels(wf_l[idx], wf_h[idx]);
  
  idx++;
  if (idx >= 7) idx = 0; 
}

void runRcJam() {
  setChannels(random(2, 42), random(42, 83));
}

void setup() {
  Serial.begin(115200);
  delay(200);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);

  esp_bt_controller_deinit();
  esp_wifi_stop();
  esp_wifi_deinit();
  esp_wifi_disconnect();
  
  vSpi = new SPIClass(HSPI);
  vSpi->begin(14, 12, 13); 

  if (nrf1.begin(vSpi)) {
    nrf1.setAutoAck(false);
    nrf1.stopListening();
    nrf1.setRetries(0, 0);
    nrf1.setPALevel(RF24_PA_MAX, true);
    nrf1.setDataRate(RF24_2MBPS);
    nrf1.setCRCLength(RF24_CRC_DISABLED);
    nrf1.startConstCarrier(RF24_PA_MAX, ch1);
  }

  if (nrf2.begin(vSpi)) {
    nrf2.setAutoAck(false);
    nrf2.stopListening();
    nrf2.setRetries(0, 0);
    nrf2.setPALevel(RF24_PA_MAX, true);
    nrf2.setDataRate(RF24_2MBPS);
    nrf2.setCRCLength(RF24_CRC_DISABLED);
    nrf2.startConstCarrier(RF24_PA_MAX, ch2);
  }
  
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Init completed.");
}

void loop() {
  static unsigned long lastTick = 0;
  unsigned long currentMs = millis();

  if (currentMs - lastTick > 50) {
    lastTick = currentMs;
    handleButton();      
    updateLed();  
  }

  switch(mode) {
    case 0: runFullJam(); break;
    case 1: runBtJam();   break;
    case 2: runBleJam();  break;
    case 3: runWifiJam(); break;
    case 4: runRcJam();   break;
  }
}
