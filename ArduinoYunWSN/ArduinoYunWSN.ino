/*
* Post data using curl to ThingSpeak.
* Serve a Web page in it's own server.
*
* Script update_dns.sh (/mnt/sda1/arduino) is able to update the public IP of the arduino yun
* crontab execute this script every 5 min
*/

#include <AltSoftSerial.h>
#include <interrupt.h>
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
#include <Console.h>
#include <HttpClient.h>
#include <dht.h>
#include <AltSoftSerial.h>
#include <SetXbee.h>
#include "webServerClass.h"

#define numYunIOPins	7			// Total number of digital IO pins used in Arduino Yun
#define numXbeeModules  2			// Total number of routers or endpoint connected in the mesh


//Thingspeak parameters 
#define maxFields 4		// Define maximum fields used in thingspeak
String thingspeakUpdateAPI = "http://api.thingspeak.com/update?";
String thingspeakWriteAPIKey = "key=1EQD8TANGANJHA3J";//Insert Your Write API key here 
String thingspeakField[maxFields] = {"field1", "field2", "field3", "field4"};

// Timing flags
bool sendFlag = 0;			// Indicate when is possible to send data to the server
bool retrieveFlag = 0;	// Indicate when is possible to retrieve sensor data

// Define sensor data structure
#define totalData 5
typedef struct
{
    unsigned int yunHdty;					// Filtered humidity
	unsigned int accumHdty;			// Accumulated humidty

    unsigned int yunTemp;					// Filtered temperature
	unsigned int accumTemp;			// Accumulated temperature

	float xbeeTempSensor[2];
} sensorData;
sensorData sData;

// Sensor DHT11 instance.
#define DHT11_PIN   2
dht DHT;

// Listen on default port 5555, Yun webserver will forward there all HTTP requests for us.
YunServer server;

// WebServerClass instance
byte yunPinDirs[numYunIOPins] = {1,1,1,1,1,0,0};
byte yunPinVals[numYunIOPins] = {0,0,0,0,0,0,0};	// pinVals gives the input/output values for pins D6,..., D12
int  yunAnVals[6]  = {0,0,0,0,0,0};	// anVals stores the analog input values for pins A0,..., A5
byte xbeePinDirs[numXbeeModules] = {0, 0};
bool xbeePinVals[numXbeeModules] = {0, 0};
webServerClass webServerHandler(numYunIOPins, numXbeeModules, 
								yunPinDirs, yunPinVals, yunAnVals,
								xbeePinDirs, xbeePinVals);

// Xbee instance and variables.
AltSoftSerial altSoftSerial;	// Arduino Yun use pin5->Tx and pin13->Rx
SetXbee xbee;
unsigned char cmdD4[2] = {'D','4'};	// Remote AT command request for IS and D4
const uint32_t addrXbee[] = {0x40B82646, 0x40A71859};	// Save lsb address for xbee modules

// Function prototype. Function declarations.
void postToThingspeak();
void retreiveSensorData();
float calculateXBeeTemp(unsigned int xbeeAnalog);


// Interrupt service routine for Timer1
ISR(TIMER1_COMPA_vect)
{ //<---- No problem, just intellisense complaining
	//Do not use Console.print()! Console.println(">ISR()\t");
	
	// Heart beat LED
	static boolean state = false;
	state = !state;  // toggle
	digitalWrite(3, state ? HIGH : LOW); //digitalWrite(3, digitalRead(3) ^ 1);

	static int sendCount = 0;	    // Count how many second had pass
	static int retrieveCount = 0;	// Count how many second had pass

	sendCount++;
	// Enter each 15 sec to send data to thingspeak server
	if(sendCount >= 15)
	{
		sendFlag = 1;
		sendCount = 0;
	}

	// Set retrieveFlag each 2 seconds
	retrieveCount++;
	// Enter each 15 sec to send data to thingspeak server
	if(retrieveCount >= 2)
	{
		retrieveFlag = 1;
		retrieveCount = 0;
	}
}

void setup()
{
	pinMode(3, OUTPUT);
    digitalWrite(3, LOW);

	//Initialize structure values
	sData.accumHdty = 0;
	sData.accumTemp = 0;

    // Initialize Bridge.
    Bridge.begin();
    // Initialize linux console-serial.
    Console.begin();

	// Setting up the web server. Listen for incoming connection.
	server.noListenOnLocalhost();
    server.begin();

	// Initialise digital input/output directions, set output values, read digital and analog inputs
	webServerHandler.setYunPinDirs();
    webServerHandler.setYunPinVals();
	//pinMode(5, OUTPUT);
	//digitalWrite(5, LOW);

	// Start serial communications for xbee
   	altSoftSerial.begin(9600);
	xbee.setSerialPrint(Console);	// Connection lost if serialPrint is not defined!!!!!!!!!
	xbee.setSerialXbee(altSoftSerial);

	// TODO add Xbee pins initialization and acknowledge with remote AT commands

	digitalWrite(3, HIGH);

    // Initiallize Timer1
    noInterrupts();         // Disable all interrupts
    TCCR1A = 0;             // Normal operation
    TCCR1B = bit(WGM12) | bit(CS10) | bit (CS12);   // CTC, scale to clock / 1024
    OCR1A =  16000;          // Compare A register value (1000 * clock speed / 1024)
    TIMSK1 = bit (OCIE1A);  // Interrupt on Compare A Match
    interrupts();           // Enable all interrupts
}

// Main loop
void loop()
{  
	// Retrieve sensor data each 2 second. 
	// A total of 5 data (10 sec) will be store before sendFlag (15sec) is set. 
	if(retrieveFlag == 1)
	{
		retrieveFlag = 0;
		retreiveSensorData();
	}

  	// Send data to ThingSpeak each 15 seconds
	if(sendFlag == 1)
	{
		sendFlag = 0;
		postToThingspeak();
	}
  
	// Get clients coming from server
	YunClient client = server.accept();
	// There is a new client?
	if (client)
	{
		webServerHandler.serverHandle(client); 
		// Close connection and free resources.
		client.stop();
		// Set remote xbee pins if local pins 9 and 10 are set via JS.
		if(digitalRead(9) == 1)   xbee.sendRemoteATCmdReq(addrXbee[0], 16, OPT_APPLY_CHANGES, cmdD4, 0x05, true);
		else   xbee.sendRemoteATCmdReq(addrXbee[0], 16, OPT_APPLY_CHANGES, cmdD4, 0x04, true);

		if(digitalRead(10) == 1)   xbee.sendRemoteATCmdReq(addrXbee[1], 16, OPT_APPLY_CHANGES, cmdD4, 0x05, true);
		else   xbee.sendRemoteATCmdReq(addrXbee[1], 16, OPT_APPLY_CHANGES, cmdD4, 0x04, true);
	}
	delay(50);
}

void postToThingspeak(){
	HttpClient client;
  
	char charIn; 
	String bufferIn;
	String request_string; 

	request_string = thingspeakUpdateAPI + thingspeakWriteAPIKey + 
			         "&" + thingspeakField[0] + "=" + String(sData.yunHdty) + 
					 "&" + thingspeakField[1] + "=" + String(sData.yunTemp) +
					 "&" + thingspeakField[2] + "=" + String(sData.xbeeTempSensor[0]) +
					 "&" + thingspeakField[3] + "=" + String(sData.xbeeTempSensor[1]);
	// Make a HTTP request.
	client.get(request_string);
  
	// If there are incoming bytes available from the server, read them and print them.
	while(client.available())
	{
		charIn = client.read();
		bufferIn += charIn;
	}
	if(bufferIn != "0")
	{
		Console.print(bufferIn);
		Console.println("\tUpdate Completed:");
		Console.print("\tYun Hdty Value: "); Console.println(sData.yunHdty);
		Console.print("\tYun Temp Value: "); Console.println(sData.yunTemp);
		Console.print("\tXbee Temp1 Value: "); Console.println(sData.xbeeTempSensor[0]);
		Console.print("\tXbee Temp2 Value: "); Console.println(sData.xbeeTempSensor[1]);
	}
	else
	{
		Console.print(bufferIn);    
		Console.println("\tUpdate Failed!");
	}
}

void retreiveSensorData()
{
	// Get xbee sensor information
	xbee.readPacket();

	// Only if new data is available and complete will proceed.
	if(xbee.isRxComplete())
	{
		// Modules will sample at 6 second each, so retreiveSensorData() must be enought faster to ahndle it.
		if(xbee.getRxLsbAddr64() == addrXbee[0])
		{
			sData.xbeeTempSensor[0] = calculateXBeeTemp(xbee.getADC3());
			//xbee.sendRemoteATCmdReq(addrXbee[0], 16, OPT_APPLY_CHANGES, cmdD4, 0x05, true);
			//xbee.sendRemoteATCmdReq(addrXbee[0], 16, OPT_APPLY_CHANGES, cmdD4, 0x04, true);
			//Console.println(sData.xbeeTempSensor[0]);
		}
		else if(xbee.getRxLsbAddr64() == addrXbee[1])
		{
			sData.xbeeTempSensor[1] = calculateXBeeTemp(xbee.getADC3());
			//xbee.sendRemoteATCmdReq(addrXbee[1], 16, OPT_APPLY_CHANGES, cmdD4, 0x05, true);
			//xbee.sendRemoteATCmdReq(addrXbee[1], 16, OPT_APPLY_CHANGES, cmdD4, 0x04, true);
			//Console.println(sData.xbeeTempSensor[1]);
		}
	}

	static int countData = 0;	// Count collected data
	//sData.potentiometer = analogRead(A1);

	// Read Data
	if(DHT.read11(DHT11_PIN) == DHTLIB_OK)
	{
		//Console.println("DHT11 OK.\t"); 
		countData++;
		sData.accumHdty += DHT.humidity;
		sData.accumTemp += DHT.temperature;
		if(countData == totalData)	// Total collected data equal to 8
		{
			sData.yunHdty = sData.accumHdty/totalData;
			sData.yunTemp = sData.accumTemp/totalData;
			sData.accumHdty = 0;
			sData.accumTemp = 0;
			countData = 0;
		}
	}
	else
	{
		Console.println("DHT11 Error,\t"); 
	}	
}

// This function takes an XBee analog pin reading and converts it to a voltage value
float calculateXBeeTemp(unsigned int xbeeAnalog) {
	float volt = ((float)xbeeAnalog/1023)*1.23; //Convert the analog value to a voltage value
  
	float temp = 0;
	// Calculate temp in C, .75 volts is 25 C. 10mV per degree
	if (volt <= .75) { temp = 25 - ((.75-volt)/.01); } //if below 25 C
	else { temp = 25 + ((volt -.75)/.01); } //if above 25
	
	return temp;
}