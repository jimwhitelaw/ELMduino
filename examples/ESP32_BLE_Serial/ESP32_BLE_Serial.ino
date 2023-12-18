#include "BleSerial.h"
#include "ELMduino.h"

#define ELM_PORT SerialBT
#define DEBUG_PORT Serial

BleSerial SerialBT;
ELM327 myELM327;
uint32_t rpm = 0;
uint8_t unitMACAddress[6]; // Use MAC address in BT broadcast and display
char deviceName[20];       // The serial string that is broadcast.

void setup()
{
    DEBUG_PORT.begin(115200);
    
    esp_read_mac(unitMACAddress, ESP_MAC_WIFI_STA);

    // Convert MAC address to Bluetooth MAC (add 2): https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system.html#mac-address
    unitMACAddress[5] += 2;

    // Create device name
    sprintf(deviceName, "BleBridge-%02X%02X", unitMACAddress[4], unitMACAddress[5]);

    // Init BLE Serial
    ELM_PORT.begin(deviceName);
    ELM_PORT.setTimeout(10);

    if (!ELM_PORT.available())
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
        while (1)
            ;
    }

    if (!myELM327.begin(ELM_PORT, true, 2000))
    {
        Serial.println("Couldn't connect to OBD scanner - Phase 2");
        while (1)
            ;
    }

    Serial.println("Connected to ELM327");
}

void loop()
{
    float tempRPM = myELM327.rpm();

    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
        rpm = (uint32_t)tempRPM;
        Serial.print("RPM: ");
        Serial.println(rpm);
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
        myELM327.printError();
}
