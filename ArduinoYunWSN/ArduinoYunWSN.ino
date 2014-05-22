/*
* Shows how to post data using curl to ThingSpeak.
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
bool sendFlag;		// Indicate when is possible to send data to the server

unsigned int sensorData[maxFields];	// Store sensor data: 0->Potentiometer, 1->Humidity, 2->Temperature

// Sensor DHT11 instance
#define DHT11_PIN   5
dht DHT;

// Listen on default port 5555, the webserver on the Yun
// will forward there all the HTTP requests for us.
YunServer server;

// WebServerClass instance
byte pinDirs[12] = {1,1,1,1,1,1,0,1,1,0,0,0};
byte pinVals[12] = {0,0,0,0,0,0,0,0,0,0,0,0};	// pinVals gives the input/output values for pins D2,..., D13
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
	digitalWrite(8, state ? HIGH : LOW); //digitalWrite(pin8, digitalRead(pin8) ^ 1);

	static int sec_count = 0;	// Count how many second has pass
	sec_count++;
	// Enter each 15 sec to send data to thingspeak server
	if(sec_count >= 15)
	{
		sendFlag = 1;
		sec_count = 0;
	}
}

void setup()
{
	pinMode(13, OUTPUT);
    digitalWrite(13, LOW);

    // Initialize Bridge.
    Bridge.begin();
    // Initialize linux console-serial.
    Console.begin();
	// Initialize avr serial
	//Serial.begin(115200);

	// Setting up the web server. Listen for incoming connection.
	server.noListenOnLocalhost();
    server.begin();
	// initialise digital input/output directions
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
  	// Send data to ThingSpeak each 15 seconds
	if(sendFlag == 1)
	{
		//Console.println(">TS()\t");
		sendFlag = 0;
		postToThingspeak();
		//Console.println("<TS()");
	}
  
	// Run web server handler
	// Get clients coming from server
	YunClient client = server.accept();
	// There is a new client?
	if (client)
	{
		//Console.println(">SH()\t");
		webServerHandler.serverHandle(client); 
		//Console.println("<SH()\t");
	}
	delay(50);
}

void postToThingspeak(){
	HttpClient client;
  
	char charIn; 
	String bufferIn;
	String request_string; 
	
	retreiveSensorData();

	request_string = thingspeak_update_API + thingspeak_write_API_key + 
			         thingspeakfield[0] + String(sensorData[0]) + 
					 thingspeakfield[1] + String(sensorData[1]) + 
					 thingspeakfield[2] + String(sensorData[2]);
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
		Console.println(sensorData[0]);

		Console.print("\t" + thingspeak_sensor[1] + " Value: ");
		Console.println(sensorData[1]);
			
		Console.print("\t" + thingspeak_sensor[2] + " Value: ");
		Console.println(sensorData[2]);
	}
	else
	{
		Console.print(bufferIn);    
		Console.println("\tUpdate Failed!");
	}
}

void retreiveSensorData()
{
	sensorData[0] = analogRead(A1);

	// Read Data
	if(DHT.read11(DHT11_PIN) == DHTLIB_OK)
	{
		//Console.println("DHT11 OK.\t"); 
		sensorData[1] = DHT.humidity;
		sensorData[2] = DHT.temperature;
	}
	else
	{
		Console.println("DHT11 Error,\t"); 
	}	
}
