#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "esp_wifi.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include <MPU6050_light.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------------- CONFIG ----------------
#define MAX_NETWORKS 8
#define WIFI_SCAN_INTERVAL 15000
#define BLE_SCAN_INTERVAL 20000
#define BT_SCAN_INTERVAL 25000
#define BLE_SCAN_TIME 5

#define MAX_PULSES 50
#define MOTION_HOLD 200
#define MOTION_THRESHOLD 10
#define EMG_THRESHOLD 2000
#define EMG_PIN 34
#define EMG_ALPHA 0.1
#define HALL_ALPHA 0.1

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define ESP32_TX 17
#define ESP32_RX 16
#define UART_BAUD 115200

// ---------------- GLOBALS ----------------
MPU6050 mpu(Wire);
BLEScan* pBLEScan;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WiFiUDP udp;

const int udpPort = 4210;
IPAddress broadcastIp(255, 255, 255, 255);

// CSI STATE
volatile unsigned long wifiPulseTimes[MAX_PULSES];
volatile int wifiPulseIndex = 0;

float wifiSmoothedRssi = NAN;
float emgSmoothed = NAN;
float hallSmoothed = NAN;

// DEVICE STRUCT
struct Device {
  String name;
  int rssi;
  int txPower;
  bool hidden;
};

Device strongestWiFi = {"", -100, -50, false};
Device strongestBLE  = {"", -100, -50, false};
Device strongestBT   = {"", -100, -50, false};

bool motionDetected = false;

// TIMERS
unsigned long lastWiFiScan = 0;
unsigned long lastBLEScan = 0;
unsigned long lastBTScan = 0;
unsigned long lastIMU = 0;
unsigned long lastFusion = 0;

const unsigned long IMU_UPDATE_INTERVAL = 250;

// ---------------- WIFI SNIFFER ----------------
void wifi_sniffer(void* buf, wifi_promiscuous_pkt_type_t type) {
  const wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;

  int rssi = pkt->rx_ctrl.rssi;

  if (isnan(wifiSmoothedRssi))
    wifiSmoothedRssi = rssi;
  else
    wifiSmoothedRssi = wifiSmoothedRssi * 0.7 + rssi * 0.3;

  static float lastWifiRssi = NAN;

  if (!isnan(lastWifiRssi) &&
      abs(wifiSmoothedRssi - lastWifiRssi) > MOTION_THRESHOLD) {

    wifiPulseTimes[wifiPulseIndex] = millis();
    wifiPulseIndex = (wifiPulseIndex + 1) % MAX_PULSES;
  }

  lastWifiRssi = wifiSmoothedRssi;
}

// ---------------- BT CALLBACK ----------------
void btGapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  if (event == ESP_BT_GAP_DISC_RES_EVT) {

    char mac[18];
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
            param->disc_res.bda[0], param->disc_res.bda[1],
            param->disc_res.bda[2], param->disc_res.bda[3],
            param->disc_res.bda[4], param->disc_res.bda[5]);

    int rssi = -100;

    for (int i = 0; i < param->disc_res.num_prop; i++) {
      if (param->disc_res.prop[i].type == ESP_BT_GAP_DEV_PROP_RSSI)
        rssi = *(int8_t*)param->disc_res.prop[i].val;
    }

    Serial.printf("BT,%s,%d\n", mac, rssi);
  }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  Serial1.begin(UART_BAUD, SERIAL_8N1, ESP32_RX, ESP32_TX);

  Wire.begin(21, 22);
  mpu.begin();
  mpu.calcOffsets();

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) while (1);

  display.clearDisplay();
  display.display();

  // WIFI SNIFFER
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer);

  udp.begin(udpPort);

  // BLE
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);

  // BT
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_bt_controller_init(&bt_cfg);
  esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
  esp_bluedroid_init();
  esp_bluedroid_enable();
  esp_bt_gap_register_callback(btGapCallback);
}

// ---------------- LOOP ----------------
void loop() {
  unsigned long now = millis();

  // CSI PING
  static unsigned long lastPing = 0;
  if (now - lastPing >= 10) {
    lastPing = now;
    udp.beginPacket(broadcastIp, udpPort);
    udp.write((uint8_t*)"PING", 4);
    udp.endPacket();
  }

  // MOTION DETECT
  bool csiMotion = false;
  for (int i = 0; i < MAX_PULSES; i++) {
    if (wifiPulseTimes[i] > now - MOTION_HOLD)
      csiMotion = true;
  }
  motionDetected = csiMotion;

  // EMG
  int emgVal = analogRead(EMG_PIN);
  emgSmoothed = isnan(emgSmoothed)
    ? emgVal
    : emgSmoothed * (1 - EMG_ALPHA) + emgVal * EMG_ALPHA;

  // 🔥 HALL SENSOR (FIXED + WORKING)
  int hVal = hallRead();
  hallSmoothed = isnan(hallSmoothed)
    ? hVal
    : hallSmoothed * (1 - HALL_ALPHA) + hVal * HALL_ALPHA;

  // 🔥 SERIAL FUSION (includes HALL OUTPUT)
  if (now - lastFusion >= 100) {
    lastFusion = now;

    int score = (csiMotion ? 4 : 0) + (emgVal > EMG_THRESHOLD ? 6 : 0);

    Serial.printf(
      "FUSION,%d,%d,0,%.1f,%.1f\n",
      score,
      csiMotion ? 1 : 0,
      hallSmoothed,
      emgSmoothed
    );

    // 🔥 THIS IS WHAT YOUR DASHBOARD NEEDS
    Serial.printf("HALL,%d\n", (int)hallSmoothed);
  }

  // IMU
  if (now - lastIMU >= IMU_UPDATE_INTERVAL) {
    lastIMU = now;
    mpu.update();
    Serial.printf("IMU,0,0,%.2f\n", mpu.getAngleZ());
  }

  // WIFI SCAN
  if (now - lastWiFiScan >= WIFI_SCAN_INTERVAL) {
    lastWiFiScan = now;

    esp_wifi_set_promiscuous(false);
    int n = WiFi.scanNetworks();

    for (int i = 0; i < min(n, MAX_NETWORKS); i++) {
      Serial.printf("WIFI,%s,%d\n",
                    WiFi.SSID(i).c_str(),
                    WiFi.RSSI(i));
    }

    WiFi.scanDelete();
    esp_wifi_set_promiscuous(true);
  }

  // BLE SCAN
  if (now - lastBLEScan >= BLE_SCAN_INTERVAL) {
    lastBLEScan = now;

    BLEScanResults found = pBLEScan->start(BLE_SCAN_TIME, false);

    for (int i = 0; i < min((int)found.getCount(), MAX_NETWORKS); i++) {
      BLEAdvertisedDevice d = found.getDevice(i);
      Serial.printf("BLE,%s,%d\n", d.getName().c_str(), d.getRSSI());
    }

    pBLEScan->clearResults();
  }

  // BT SCAN
  if (now - lastBTScan >= BT_SCAN_INTERVAL) {
    lastBTScan = now;
    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
  }

  delay(10);
}
