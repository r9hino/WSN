// Web server handler
// Rest request are responded with JSON format in serverHandle()

#include "webServerClass.h"
#include <Bridge.h>
//#include <YunClient.h>

//*************************************************
// PUBLIC
webServerClass::webServerClass(byte yunIOPins, byte numXbeeModules, 
							   byte pinDirs[], byte pinVals[], int anVals[],
							   byte pinDirsXbee[], bool pinValsXbee[])
{
	_yunIOPins = yunIOPins;
	_numXbeeModules = numXbeeModules;

	// Yun variables initialization
	for(byte i=0; i < _yunIOPins; ++i)
	{
		_pinDirs[i] = pinDirs[i];
		_pinVals[i] = pinVals[i];
		
		if(i<6) _anVals[i] = anVals[i];
	}

	// Xbee variables initialization
	for(byte i=0; i < numXbeeModules; ++i)
	{
		_pinDirsXbee[i] = pinDirsXbee[i];
		_pinValsXbee[i] = pinValsXbee[i];
	}
}


void webServerClass::serverHandle(YunClient client)
{
	// Read the command
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

	// This sets the direction of data for the digital pins. 
	// See definition of pinDirs[] above returns a JSON "OK" object when finished
	if(command == "io")
	{
		command = client.readStringUntil('/');
		command.trim();

		for (byte i=0; i<command.length(); ++i)
		{
			_pinDirs[i] = byte(command.charAt(i)-48);
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

	// This sets values for the digital output pins, and returns a JSON "OK" object when finished
	if (command == "do")
	{
		command = client.readStringUntil('/');
		command.trim();

		for (byte i=0; i<command.length(); ++i)
		{
			if (command.charAt(i) != '-')
			{
				_pinVals[i] = byte(command.charAt(i)-48);
			}
			else
			{
				_pinVals[i] = 255;
			}
		}

		// Set JSON header
		client.println("Status: 200");
		client.println("Content-type: application/json");
		client.println();
		// Return ok status
		client.print("{\"ret\":\"ok\"}");

		// Update data values
		setPinVals();
	}    

	// Reads the digital IO and analog pins and returns the values as a JSON object
	if (command == "readIO")
	{
		// Update data values
		setPinVals();

		// Set JSON header
		client.println("Status: 200");
		client.println("Content-type: application/json");
		client.println();

		// Set JSON data. First give the data direction definitions
		client.print("{\"Datadir\" : [");
		for (byte i=0; i<_yunIOPins; ++i)
		{
			client.print("{\"datadir\" : "+String(_pinDirs[i])+"}");  
			if (i < _yunIOPins-1) client.print(",");
		}

		// Finish the array, then give the digital input values
		client.print("],\"DigitalYun\" : [");
		for (byte i=0; i<_yunIOPins; ++i)
		{
			if(_pinDirs[i] == 0)  // Outputs we do assign a value
			{
				client.print("{\"dataval\" : "+String(_pinVals[i])+"}");
			}
			else  // Inputs we do not assign a value, that is why we add 10
			{
				client.print("{\"dataval\" : "+String(10+_pinVals[i])+"}");
			}
			if (i<_yunIOPins-1) client.print(",");
		}  

		// Finish the array, then give the analog input values
		client.print("],\"Analog\" : [");
		for (byte i=0; i<6; ++i)
		{
			client.print("{\"dataval\" : "+String(_anVals[i])+"}");
			if (i<5) client.print(",");
		}

		// Finish the Xbee array, then give the digital input values
		client.print("],\"DigitalXbee\" : [");
		for (byte i=0; i<_numXbeeModules; ++i)
		{
			if(_pinDirsXbee[i] == 0)  // Outputs we do assign a value
			{
				client.print("{\"dataval\" : "+String(_pinValsXbee[i])+"}");
			}
			else  // Inputs we do not assign a value, that is why we add 10
			{
				client.print("{\"dataval\" : "+String(10+_pinValsXbee[i])+"}");
			}
			if (i<_numXbeeModules-1) client.print(",");
		} 

		client.print("]}");
	}
}


// Set the pin modes based on the pinDirs[] array
void webServerClass::setPinDirs()
{
	for(byte i=0; i<_yunIOPins; ++i)
	{
		if (_pinDirs[i]==0)  pinMode(6+i, OUTPUT);
		if (_pinDirs[i]==1)  pinMode(6+i, INPUT);
		if (_pinDirs[i]==2)  pinMode(6+i, INPUT_PULLUP);
	}
}


// Set the output pin values based on the pinVals[] array
// Read the digital input values and store in the pinVals[] array
// Read the analog input values and store in the anVals[] array
void webServerClass::setPinVals()
{
	for(byte i=0; i<_yunIOPins; ++i)
	{
		if (_pinDirs[i]==0 && _pinVals[i]==0) digitalWrite(6+i,LOW);
		if (_pinDirs[i]==0 && _pinVals[i]==1) digitalWrite(6+i,HIGH);    
		if (_pinDirs[i]==1 || _pinDirs[i]==2)
		{
			if (digitalRead(6+i)==LOW)   _pinVals[i]=0;
			if (digitalRead(6+i)==HIGH)  _pinVals[i]=1;
		}
	}  

	// Read analogue values.
	// The arduino uses a multiplexor for analog in with all inputs using
	// a commons ADC. This means that the multiplexor needs to switch
	// between inputs and time is required for the voltage to stabilise.
	// Multiple reads with a delay can help
	for (byte i=0; i<6; ++i)
	{
		// first read trigger the switch
		_anVals[i] = analogRead(i);  
		// delay to let voltage stabilise
		//delay(20);
		// Next read gives correct reading with no ghosting from other channels		
		_anVals[i] = analogRead(i);  
	}
}