#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <NimBLEDevice.h>

// im not putting my wifi and password here lol
const char *WIFI_SSID = "";
const char *WIFI_PASS = "";

const int TARGET_HOUR   = 9;
const int TARGET_MINUTE = 3;

const char *TZ = "EST5EDT,M3.2.0,M11.1.0";

// class ScanCB : public NimBLEScanCallbacks
// {
//     void onResult(const NimBLEAdvertisedDevice *dev) override
//     {
//         const std::vector<uint8_t> &p = dev->getPayload();
//         // keep only: service data UUID 0x08F0 (bytes F0 08) or manufacturer ID 0x5556 (bytes 56 55)
//         bool match = false;
//         for (size_t i = 0; i + 1 < p.size(); i++)
//         {
//             if ((p[i] == 0xF0 && p[i + 1] == 0x08) || (p[i] == 0x56 && p[i + 1] == 0x55))
//                 match = true;
//         }
//         if (!match)
//             return;
//
//         Serial.printf("%s RSSI %d  ", dev->getAddress().toString().c_str(), dev->getRSSI());
//
//         for (uint8_t b : p)
//             Serial.printf("%02X", b);
//
//         Serial.println();
//     }
// };
//
// static ScanCB cb;
//
// void setup()
// {
//     Serial.begin(115200);
//     NimBLEDevice::init("");
//     NimBLEScan *scan = NimBLEDevice::getScan();
//     scan->setScanCallbacks(&cb, true); // true = report duplicates
//     scan->setActiveScan(false);
//     scan->setInterval(100);
//     scan->setWindow(100);
//     scan->start(0, false);
// }

const int      BURSTS  = 2;
const uint32_t SLOT_MS = 60;

const uint8_t pktA[] = {
    0x02, 0x01, 0x02, 0x1B, 0x16, 0xF0, 0x08, 0x10, 0x00, 0x6A, 0xAF, 0xF1, 0xE1, 0x18, 0x1C, 0x20,
    0xFA, 0x72, 0xF1, 0x7E, 0xE3, 0xF6, 0x64, 0x06, 0x4E, 0x32, 0x56, 0xE4, 0x74, 0x12, 0xA5
};

const uint8_t pktB[] = {
    0x02, 0x01, 0x02, 0x1B, 0x16, 0xF0, 0x08, 0x11, 0x00, 0x6A, 0xAF, 0xF1, 0xE1, 0x18, 0x1C, 0x20,
    0xFA, 0x72, 0xF1, 0x7E, 0xE3, 0xF6, 0x64, 0x06, 0x4E, 0x32, 0x56, 0xE4, 0x74, 0xCA, 0xC8
};

void sendPacket(const uint8_t *data, size_t len, uint32_t ms)
{
    NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
    adv->stop();
    NimBLEAdvertisementData ad;
    ad.addData(data, len);
    adv->setAdvertisementData(ad);
    adv->setMinInterval(32);
    adv->setMaxInterval(48);
    adv->start();
    delay(ms);
    adv->stop();
}

void sendCommand()
{
    for (int i = 0; i < BURSTS; i++)
    {
        sendPacket(pktA, sizeof(pktA), SLOT_MS);
        sendPacket(pktB, sizeof(pktB), SLOT_MS);
    }
}

void setup()
{
    Serial.begin(115200);

    NimBLEDevice::init("");
    NimBLEDevice::getAdvertising()->setConnectableMode(BLE_GAP_CONN_MODE_NON);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi ok");

    configTzTime(TZ, "pool.ntp.org", "time.nist.gov");

    struct tm t;
    while (!getLocalTime(&t, 1000))
    {
        Serial.println("waiting for time...");
    }

    Serial.println(&t, "Time synced: %Y-%m-%d %H:%M:%S");
}

void loop()
{
    struct tm t;
    if (getLocalTime(&t, 1000))
    {
        if (t.tm_hour == TARGET_HOUR && t.tm_min == TARGET_MINUTE)
        {
            Serial.println("sending");
            sendCommand();
        }
    }
    delay(1000);
}
