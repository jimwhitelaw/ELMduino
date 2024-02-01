#include "BluetoothSerial.h"
#include "ELMduino.h"

BluetoothSerial SerialBT;
#define ELM_PORT SerialBT
#define DEBUG_PORT Serial
bool debugMode = true;

ELM327 myELM327;

uint8_t ctoi(uint8_t value)
{
    if (value >= 'A')
        return value - 'A' + 10;
    else
        return value - '0';
}

void setup()
{
    DEBUG_PORT.begin(115200);
    // DEBUG_PORT.setPin("1234");
    ELM_PORT.begin("ArduHUD", true);

    if (!ELM_PORT.connect("ELMULATOR"))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 1");
        while (1)
            ;
    }

    if (!myELM327.begin(ELM_PORT, true, 2000))
    {
        DEBUG_PORT.println("Couldn't connect to OBD scanner - Phase 2");
        while (1)
            ;
    }

    DEBUG_PORT.println("Connected to ELM327");
}

void loop()
{
    myELM327.sendCommand_Blocking("2218E4");
    if (myELM327.nb_rx_state == ELM_SUCCESS)
    {
        uint8_t A = ctoi(myELM327.payload[6]);
        uint8_t B =  ctoi(myELM327.payload[7]);
        if (debugMode) 
        {
            DEBUG_PORT.print("Response: "); DEBUG_PORT.println(myELM327.payload);
            DEBUG_PORT.print("A: "); DEBUG_PORT.println(A);
            DEBUG_PORT.print("B: "); DEBUG_PORT.println(B);
        }
        float Clogging = ((A * 256) + B) * (1000.0 / 65535.0);
        DEBUG_PORT.print("DPF Clogging %: ");
        DEBUG_PORT.println(Clogging);
        delay(10000);
    }
    else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
    {
        myELM327.printError();
    }
}
