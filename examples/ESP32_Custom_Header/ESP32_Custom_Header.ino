#include "SafeString.h"
// #include <SoftwareSerial.h>
#include <ELMduino.h>
#include <BluetoothSerial.h>
#include <Arduino.h>

float Clogging;

BluetoothSerial mySerial;
// SoftwareSerial mySerial(6 ,7); // RX, TX for BlueTooth HC-05
#define ELM_PORT mySerial
#define DEBUG_PORT Serial
ELM327 myELM327;

// DICHIARAZIONE SAFE THE STRING
createSafeString(dataFrame0, 14); // will also add space for the terminating null =>15 sized array
createSafeString(dataFrame1, 14);
createSafeString(dataFrame2, 14);
createSafeString(dataFrame3, 14);
createSafeString(dataFrame4, 14);
createSafeString(dataFrame5, 14);
createSafeString(dataFrame6, 14);
createSafeString(dataFrame7, 14);
createSafeString(dataFrame8, 14);

void clearData()
{
	dataFrame0.clear();
	dataFrame1.clear();
	dataFrame2.clear();
	dataFrame3.clear();
	dataFrame4.clear();
	dataFrame5.clear();
	dataFrame6.clear();
	dataFrame7.clear();
	dataFrame8.clear();
}

void addToFrame(int frame, char c)
{
	switch (frame)
	{
	case 0:
		dataFrame0 += c;
		break;
	case 1:
		dataFrame1 += c;
		break;
	case 2:
		dataFrame2 += c;
		break;
	case 3:
		dataFrame3 += c;
		break;
	case 4:
		dataFrame4 += c;
		break;
	case 5:
		dataFrame5 += c;
		break;
	case 6:
		dataFrame6 += c;
		break;
	case 7:
		dataFrame7 += c;
		break;
	case 8:
		dataFrame8 += c;
		break;
	}
}

void frameSubstr(SafeString &frame, int m, int n, SafeString &subStr)
{
	frame.substring(subStr, m, n); // SafeString substring is inclusive m to n
}

// CONVERTITORE DA HEX TO INT
int convertToInt(SafeString &dataFrame, size_t m, size_t n)
{
	// define a local SafeString on the stack for this method
	createSafeString(hexSubString, 14); // allow for taking entire frame as a substring
	frameSubstr(dataFrame, m, n, hexSubString);
	return (int)strtol(hexSubString.c_str(), NULL, 16);
}

// ACQUISITORE FRASI OBD
void parse(char *raw)
{
	int frame = -1;

	int len = strlen(raw);

	for (int i = 0; i < len; i++)
	{

		if (raw[i + 1] == ':')
		{ // start frame
			frame = (int)raw[i] - '0';
			continue;
		}

		if (raw[i] == ':')
		{
			continue;
		}

		if (frame == -1)
		{
			continue;
		}

		if (raw[i] == '>')
		{
			frame = -1;
			continue;
		}

		addToFrame(frame, raw[i]);
	}
}

void read_rawdata()
{
	// move data from OBD to Rawdata array
	char rawData[myELM327.recBytes];
	int n = 0;

	Serial.print("Payload received: ");

	for (int i = 0; i < myELM327.recBytes; i++)
	{
		rawData[n++] = myELM327.payload[i];
		Serial.print(myELM327.payload[i]); // Serial print OBD Rawdata
	}
	Serial.println();

	parse(rawData); // parse data received from OBD
}

void setup()
{
	Serial.begin(115200);
	ELM_PORT.begin("ArduHUD", true);
	if (!ELM_PORT.connect("ELMULATOR"))
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
	myELM327.sendCommand_Blocking("2218E4");
	if (myELM327.nb_rx_state == ELM_SUCCESS)
	{
		read_rawdata();
		int A = convertToInt(dataFrame0, 6, 7);
		int B = convertToInt(dataFrame0, 7, 8);
		Serial.print("A: "); Serial.println(A);
		Serial.print("B: "); Serial.println(B);
		Clogging = ((A * 256) + B) * (1000.0 / 65535.0);
		Serial.print("intasamento: ");
		Serial.print(Clogging);
		Serial.println(" % ");
		delay(1000);
		clearData();
	}
	else if (myELM327.nb_rx_state != ELM_GETTING_MSG)
	{
		myELM327.printError();
	}
}