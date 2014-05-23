/*
* Post data using curl to ThingSpeak.
* Serve a Web page in it's own server.
*
* Script update_dns.sh (/mnt/sda1/arduino) is able to update the public IP of the arduino yun
* crontab execute this script every 5 min
*/

#include <interrupt.h>
#include <Bridge.h>
#include <YunServer.h>
#include <YunClient.h>
// Include needed for ThingSpeak
#include <Console.h>
#include <HttpClient.h>
#include <dht.h>
#include "webServerClass.h"


//Thingspeak parameters 
#define maxFields 3		// Define maximum fields used in thingspeak
String thingspeak_update_API = "http://api.thingspeak.com/update?";
String thingspeak_write_API_key = "key=XWYK90NA07HVY9LM ";//Insert Your Write API key here 
String thingspeakfield[maxFields] = {"&field1=", "&field2=", "&field3="};
String thingspeak_sensor[maxFields] = {"Potentiometer", "Humidity", "Temperature"};

bool sendFlag;			// Indicate when is possible to send data to the server
bool retrieveFlag = 0;	// Indicate when is possible to retrieve sensor data

// Define sensor data structure
#define totalData 5
typedef struct
{
	unsigned int potentiometer;

    unsigned int humidity;				// Filtered humidity
	unsigned int accumHumidity;			// Accumulated humidty

    unsigned int temperature;			// Filtered temperature
	unsigned int accumTemperature;		// Accumulated temperature
} sensorData;
sensorData sData;

// Sensor DHT11 instance
#define DHT11_PIN   5
dht DHT;

// Listen on default port 5555, Yun webserver will forward there all HTTP requests for us.
YunServer server;

// WebServerClass instance
byte pinDirs[11] = {1,1,1,1,1,1,1,1,0,0,0};
byte pinVals[11] = {0,0,0,0,0,0,0,0,0,0,0};	// pinVals gives the input/output values for pins D2,..., D12
int  anVals[6]  = {0,0,0,0,0,0};	// anVals stores the analog input values for pins A0,..., A5
webServerClass webServerHandler(pinDirs, pinVals, anVals);

// Function prototype. Function declarations.
void postToThingspeak();
void retreiveSensorData();


// Interrupt service routine for Timer1
ISR(TIMER1_COMPA_vect)
{ //<---- No problem, just intellisense complaining
	//Do not use Console.print()! Console.println(">ISR()\t");
	
	// Toggle LED
	static boolean state = false;
	state = !state;  // toggle
	digitalWrite(13, state ? HIGH : LOW); //digitalWrite(13, digitalRead(13) ^ 1);

	static int sendCount = 0;	    // Count how many second has pass
	static int retrieveCount = 0;	// Count how many second has pass

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
	pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

	//Initialize structure values
	sData.accumHumidity = 0;
	sData.accumTemperature = 0;

    // Initialize Bridge.
    Bridge.begin();
    // Initialize linux console-serial.
    Console.begin();
	// Initialize avr serial
	//Serial.begin(115200);

	// Setting up the web server. Listen for incoming connection.
	server.noListenOnLocalhost();
    server.begin();
	// Initialise digital input/output directions
    // set output values, read digital and analog inputs
	webServerHandler.setPinDirs();
    webServerHandler.setPinVals();
	digitalWrite(13, HIGH);

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
	// Retrieve sensor data each 2 second. A total of 5 data will be get
	// before sendFlag (15sec) is set. 
	if(retrieveFlag == 1)
	{
		retrieveFlag = 0;
		retreiveSensorData();
	}

  	// Send data to ThingSpeak each 15 seconds
	if(sendFlag == 1)
	{
		//Console.println(">TS()\t");
		sendFlag = 0;
		postToThingspeak();
		//Console.println("<TS()");
	}
  
	// Get clients coming from server
	YunClient client = server.accept();
	// There is a new client?
	if (client)
	{
		//Console.println(">SH()\t");
		webServerHandler.serverHandle(client); 
		// Close connection and free resources.
		client.stop();
		//Console.println("<SH()\t");
	}
	delay(50);
}

void postToThingspeak(){
	HttpClient client;
  
	char charIn; 
	String bufferIn;
	String request_string; 
	
	//retreiveSensorData();

	request_string = thingspeak_update_API + thingspeak_write_API_key + 
			         thingspeakfield[0] + String(sData.potentiometer) + 
					 thingspeakfield[1] + String(sData.humidity) + 
					 thingspeakfield[2] + String(sData.temperature);
	// Make a HTTP request:
	client.get(request_string);
  
	// if there are incoming bytes available 
	// from the server, read them and print them:  
	while(client.available())
	{
		charIn = client.read();
		bufferIn += charIn;
	}
	if(bufferIn != "0")
	{
		Console.print(bufferIn);
		Console.println("\tUpdate Completed:");
		Console.print("\t" + thingspeak_sensor[0] + " Value: ");
		Console.println(sData.potentiometer);

		Console.print("\t" + thingspeak_sensor[1] + " Value: ");
		Console.println(sData.humidity);
			
		Console.print("\t" + thingspeak_sensor[2] + " Value: ");
		Console.println(sData.temperature);
	}
	else
	{
		Console.print(bufferIn);    
		Console.println("\tUpdate Failed!");
	}
}

void retreiveSensorData()
{
	static int countData = 0;	// Count collected data
	sData.potentiometer = analogRead(A1);

	// Read Data
	if(DHT.read11(DHT11_PIN) == DHTLIB_OK)
	{
		//Console.println("DHT11 OK.\t"); 
		countData++;
		sData.accumHumidity += DHT.humidity;
		sData.accumTemperature += DHT.temperature;
		if(countData == totalData)	// Total collected data equal to 8
		{
			sData.humidity = sData.accumHumidity/totalData;
			sData.temperature = sData.accumTemperature/totalData;
			sData.accumHumidity = 0;
			sData.accumTemperature = 0;
			countData = 0;
		}
	}
	else
	{
		Console.println("DHT11 Error,\t"); 
	}	
}
