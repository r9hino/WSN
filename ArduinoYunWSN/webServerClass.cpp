// Web server handler
// Rest request are responded with JSON format in serverHandle()

#include <Bridge.h>
#include <YunClient.h>
#include "webServerClass.h"


//*************************************************
// PUBLIC
webServerClass::webServerClass(byte _pinDirs[11], byte _pinVals[11], int _anVals[6])
{
	// Setting up the web server. Listen for incoming connection.
    //server.noListenOnLocalhost();
    //server.begin();

	for(i = 0; i < 11; i++)
	{
		pinDirs[i] = _pinDirs[i];
		pinVals[i] = _pinVals[i];
		
		if(i<6) anVals[i] = _anVals[i];
	}
}


void webServerClass::serverHandle(YunClient client)
{
	// read the command
	String command;
	command = client.readStringUntil('/');
	command.trim();        //kill whitespace

	// command = table Data
	// This retrieve system information data send it back to client in JSON format.
	// Returns a JSON "OK" object when finished.
	/*if (command == "tableData")
	{
		// Set JSON header
		client.println("Status: 200");
		client.println("Content-type: application/json");
		client.println();

		digitalWrite(10, HIGH);
		String tableJSON = "{\"status\":\
			[\
				{\
					\"descriptor\" : \"Linux Memory Used\",\
					\"value\" : 5000,\
					\"unit\" : \"MB\"\
				},\
				{\
					\"descriptor\" : \"Linux RAM Used\",\
					\"value\" : 300,\
					\"unit\" : \"kB\"\
				},\
				{\
					\"descriptor\" : \"Module IP\",\
		            \"value\" : \"200.201.126.20\",\
				    \"unit\" : \"\"\
				}\
			]}";
		client.print(tableJSON);
	}*/

	// command = io
	// This sets the direction of data for the digital pins. 
	// See definition of pinDirs[] above returns a JSON "OK" object when finished
	if(command == "io")
	{
		command = client.readStringUntil('/');
		command.trim();

		for (i=0; i<command.length(); i++)
		{
			pinDirs[i] = byte(command.charAt(i)-48);
		}

		// Set JSON header
		client.println("Status: 200");
		client.println("Content-type: application/json");
		client.println();
		// return ok status
		client.print("{\"ret\":\"ok\"}");

		// Update data directions
		setPinDirs();
	}

	// command = do
	// This sets values for the digital output pins
	// returns a JSON "OK" object when finished
	if (command == "do")
	{
		command = client.readStringUntil('/');
		command.trim();

		for (i=0; i<command.length(); i++)
		{
			if (command.charAt(i) != '-')
			{
				pinVals[i] = byte(command.charAt(i)-48);
			}
			else
			{
				pinVals[i] = 255;
			}
		}

		// set JSON header
		client.println("Status: 200");
		client.println("Content-type: application/json");
		client.println();
		// return ok status
		client.print("{\"ret\":\"ok\"}");

		// update data values
		setPinVals();
	}    

	// command = in
	// This reads the digital and analog inputs
	// and returns the values as a JSON object
	if (command == "in")
	{
		// update data values
		setPinVals();

		// set JSON header
		client.println("Status: 200");
		client.println("Content-type: application/json");
		client.println();

		// set JSON data
		// first give the data direction definitions
		client.print("{\"Datadir\" : [");
		for (i=0; i<11; i++)
		{
			client.print("{\"datadir\" : "+String(pinDirs[i])+"}");  
			if (i < 10) client.print(",");
		}
		// finish the array
		// then give the digital input values
		client.print("],\"Digital\" : [");
		for (i=0; i<11; i++)
		{
			if(pinDirs[i] == 0)  // Outputs we do assign a value
			{
				client.print("{\"dataval\" : "+String(pinVals[i])+"}");
			}
			else  // Inputs we do not assign a value, that is why we add 10
			{
				client.print("{\"dataval\" : "+String(10+pinVals[i])+"}");
			}
			if (i < 10) client.print(",");
		}  
		// finish the array
		// then give the analog input values
		client.print("],\"Analog\" : [");
		for (i=0; i<6; i++)
		{
			client.print("{\"dataval\" : "+String(anVals[i])+"}");
			if (i<5) client.print(",");
		}
		client.print("]}");
	}
}


// Set the pin modes based on the pinDirs[] array
void webServerClass::setPinDirs()
{
	for(i=0; i<11; i++)
	{
		if (pinDirs[i]==0)  pinMode(2+i, OUTPUT);
		if (pinDirs[i]==1)  pinMode(2+i, INPUT);
		if (pinDirs[i]==2)  pinMode(2+i, INPUT_PULLUP);
	}
}


// Set the output pin values based on the pinVals[] array
// Read the digital input values and store in the pinVals[] array
// Read the analog input values and store in the anVals[] array
void webServerClass::setPinVals()
{
	for(i=0; i<11; i++)
	{
		if (pinDirs[i]==0 && pinVals[i]==0) digitalWrite(2+i,LOW);
		if (pinDirs[i]==0 && pinVals[i]==1) digitalWrite(2+i,HIGH);    
		if (pinDirs[i]==1 || pinDirs[i]==2)
		{
			if (digitalRead(2+i)==LOW)  pinVals[i]=0;
			if (digitalRead(2+i)==HIGH)  pinVals[i]=1;
		}
	}  

	// Read analogue values.
	// The arduino uses a multiplexor for analog in with all inputs using
	// a commons ADC. This means that the multiplexor needs to switch
	// between inputs and time is required for the voltage to stabilise.
	// Multiple reads with a delay can help
	for (i=0; i<6; i++)
	{
		// first read trigger the switch
		anVals[i] = analogRead(i);  
		// delay to let voltage stabilise
		//delay(20);
		// Next read gives correct reading with no ghosting from other channels		
		anVals[i] = analogRead(i);  
	}
}