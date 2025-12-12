#include <Arduino.h>
#include "globals.h"
#ifdef WIFI_CONNECTION
#include <WiFi.h>
#ifdef MDNS_ENABLED
#include <ESPmDNS.h>
#endif // MDNS_ENABLED
#ifdef ARDUINO_OTA
#include <ArduinoOTA.h>
#endif // ARDUINO_OTA
#include "wificonnection.h"
#include "webserver.h"
#include "debug.h"

// WiFi constants based on IEEE 802.11 standard
#define WIFI_SSID_MAX_LENGTH 32
#define WIFI_PASSWORD_MAX_LENGTH 63

// WiFi credentials
char robotName[WIFI_SSID_MAX_LENGTH] = "ada"; // -- Used as WiFi host name
static char wifiSSID[WIFI_SSID_MAX_LENGTH] = "IOT";
static char wifiPassphrase[WIFI_PASSWORD_MAX_LENGTH + 1] = ""; // +1 for null terminator

// WiFi roaming variables
static unsigned long lastRoamCheck = 0;
#define ROAM_CHECK_INTERVAL 30000     // Check every 30 seconds
#define WEAK_SIGNAL_THRESHOLD -75     // dBm - consider signal weak below this
#define ROAM_IMPROVEMENT_THRESHOLD 10 // dBm - must be at least 10dB better to switch

bool connectToWiFi(const char *ssid = NULL, const char *password = NULL, const uint8_t *bssid = NULL)
{
    // Use provided credentials or fall back to stored ones
    ssid = ssid ? ssid : wifiSSID;
    password = password ? password : wifiPassphrase;
    if (bssid)
    {
        DB_PRINTF("Attempting to connect to '%s' with BSSID: %02X:%02X:%02X:%02X:%02X:%02X\n",
                  ssid, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
    }
    else
    {
        DB_PRINTF("Attempting to connect to '%s'\n", ssid);
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password, 0, bssid); // channel=0 (auto), specific BSSID

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    { // 10 second timeout
        delay(500);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        DB_PRINTF("Connected to AP '%s' with IP: %s, RSSI: %d dBm, BSSID: %s\n",
                  ssid, WiFi.localIP().toString().c_str(), WiFi.RSSI(), WiFi.BSSIDstr().c_str());
        return true;
    }
    else
    {
        DB_PRINTF("Failed to connect to AP '%s'\n", ssid);
        return false;
    }
}

void scanAndRoam()
{
    int currentRSSI = WiFi.RSSI();
    String currentBSSID = WiFi.BSSIDstr();

    DB_PRINTF("Current connection - SSID: %s, RSSI: %d dBm, BSSID: %s\n",
              WiFi.SSID().c_str(), currentRSSI, currentBSSID.c_str());

    // Only roam if signal is weak
    if (currentRSSI > WEAK_SIGNAL_THRESHOLD)
    {
        DB_PRINTF("Signal strength good (%d dBm), no roaming needed\n", currentRSSI);
        return;
    }

    DB_PRINTF("Signal weak (%d dBm), scanning for better access points...\n", currentRSSI);
    int numNetworks = WiFi.scanNetworks();
    if (numNetworks == 0)
    {
        DB_PRINTLN("No networks found during scan");
        return;
    }

    int bestRSSI = currentRSSI;
    String bestBSSID = "";
    uint8_t bestBSSIDBytes[6]; // Store BSSID as bytes for WiFi.begin()
    bool foundBetter = false;

    // Look for all access points with our target SSID
    for (int i = 0; i < numNetworks; i++)
    {
        String scannedSSID = WiFi.SSID(i);
        int scannedRSSI = WiFi.RSSI(i);
        String scannedBSSID = WiFi.BSSIDstr(i);

        // Check if this is our target SSID and has better signal
        if (scannedSSID.equals(wifiSSID))
        {
            DB_PRINTF("Found AP: SSID=%s, RSSI=%d dBm, BSSID=%s",
                      scannedSSID.c_str(), scannedRSSI, scannedBSSID.c_str());

            if (scannedBSSID.equals(currentBSSID))
            {
                DB_PRINTLN(" (current AP)");
            }
            else if (scannedRSSI > (bestRSSI + ROAM_IMPROVEMENT_THRESHOLD))
            {
                bestRSSI = scannedRSSI;
                bestBSSID = scannedBSSID;
                // Get BSSID as byte array for WiFi.begin()
                uint8_t *scannedBSSIDBytes = WiFi.BSSID(i);
                memcpy(bestBSSIDBytes, scannedBSSIDBytes, 6);
                foundBetter = true;
                DB_PRINTF(" (BETTER - improvement: %d dBm)\n", scannedRSSI - currentRSSI);
            }
            else
            {
                DB_PRINTLN(" (not significantly better)");
            }
        }
    }

    // Switch to better access point if found
    if (foundBetter)
    {
        DB_PRINTF("Roaming to better AP: RSSI %d->%d dBm (improvement: %d dBm), BSSID: %s->%s\n",
                  currentRSSI, bestRSSI, bestRSSI - currentRSSI, currentBSSID.c_str(), bestBSSID.c_str());

        WiFi.disconnect();
        delay(1000);

        // Try to connect to the specific stronger access point using BSSID
        if (connectToWiFi(wifiSSID, wifiPassphrase, bestBSSIDBytes))
        {
            String newBSSID = WiFi.BSSIDstr();
            int newRSSI = WiFi.RSSI();

            if (newBSSID.equals(bestBSSID))
            {
                DB_PRINTF("Roaming successful! Connected to target AP (BSSID: %s, RSSI: %d dBm)\n",
                          newBSSID.c_str(), newRSSI);
            }
            else
            {
                DB_PRINTF("Roaming partially successful - connected to different AP (BSSID: %s, RSSI: %d dBm)\n",
                          newBSSID.c_str(), newRSSI);
            }
        }
        else
        {
            DB_PRINTLN("Roaming to specific AP failed, trying general connection");
            // Fall back to general connection (any AP with same SSID)
            connectToWiFi();
        }
    }
    else
    {
        DB_PRINTLN("No better access points found for roaming");
    }

    WiFi.scanDelete(); // Free scan results memory
}

void WiFi_setup()
{
    // Read robot name
    preferences.getString("robot_name", robotName, sizeof(robotName));
    WiFi.setHostname(robotName);

    // Load network credentials from preferences
    preferences.getString("wifi_ssid", wifiSSID, sizeof(wifiSSID));
    preferences.getString("wifi_key", wifiPassphrase, sizeof(wifiPassphrase));
    DB_PRINTF("Loaded network credentials for SSID: %s\n", wifiSSID);

    // Connect to Wifi
    while (WiFi.status() != WL_CONNECTED)
    {
        if (!connectToWiFi())
        {
            delay(2000);
        }
    }

    // Setup for OTA updates
#ifdef ARDUINO_OTA
    ArduinoOTA.setHostname(robotName);
    ArduinoOTA
        .onStart([]()
                 {
                    String type;
                    if (ArduinoOTA.getCommand() == U_FLASH)
                    {
                        type = "sketch";
                    } else { // U_SPIFFS
                        type = "filesystem";
                        // TODO: is this needed?
                        // SPIFFS.end();
                    }
                    DB_PRINTLN("Start updating " + type); })
        .onEnd([]()
               { DB_PRINTLN("\nEnd"); })
        .onProgress([](unsigned int progress, unsigned int total)
                    { DB_PRINTF("Progress: %u%%\r", (progress / (total / 100))); })
        .onError([](ota_error_t error)
                 {
                    DB_PRINTF("Error[%u]: ", error);
                    if (error == OTA_AUTH_ERROR) DB_PRINTLN("Auth Failed");
                    else if (error == OTA_BEGIN_ERROR) DB_PRINTLN("Begin Failed");
                    else if (error == OTA_CONNECT_ERROR) DB_PRINTLN("Connect Failed");
                    else if (error == OTA_RECEIVE_ERROR) DB_PRINTLN("Receive Failed");
                    else if (error == OTA_END_ERROR) DB_PRINTLN("End Failed"); });

    ArduinoOTA.begin();
    DB_PRINTLN("Ready for OTA updates");
#endif // ARDUINO_OTA

    // Start MDNS server
#ifdef MDNS_ENABLED
    if (MDNS.begin(robotName))
    {
        DB_PRINT("MDNS responder started, name: ");
        DB_PRINTLN(robotName);
    }
    else
    {
        DB_PRINTLN("Could not start MDNS responder");
    }

    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
#endif // MDNS_ENABLED
}

void WiFi_loop()
{
    static uint8_t k = 0;

    // Check connection status
    if (WiFi.status() != WL_CONNECTED)
    {
        DB_PRINTLN(F("\nWiFi lost. Attempting to reconnect"));
        connectToWiFi();
        lastRoamCheck = millis(); // Reset roam timer after reconnection
    }
    else
    {
        // Periodically check for better networks to roam to
        unsigned long now = millis();
        if (now - lastRoamCheck >= ROAM_CHECK_INTERVAL)
        {
            lastRoamCheck = now;
            scanAndRoam();
        }
    }

    // check for OTA updates
#ifdef ARDUINO_OTA
    ArduinoOTA.handle();
#endif // ARDUINO_OTA
}

#else

void WiFi_setup() {};
void WiFi_loop() {};

#endif // WIFI_CONNECTION
