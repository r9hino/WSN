// Web server handler
// Date: 09-06-2014

#ifndef webServerClass_h
#define webServerClass_h

#include <YunClient.h>

class webServerClass
{
public:
	webServerClass(byte yunIOPins, byte pinDirs[7], byte pinVals[7], int anVals[6]);
	void serverHandle(YunClient client);
	void setPinDirs();
	void setPinVals();

private:
	// Number of digital IO that Arduino Yun has.
	byte _yunIOPins;

	// pinDirs gives data direction for D6,D7, ... D12
	//	0: output  1: input   2: input with internal pull-up enabled
	byte _pinDirs[7];	// = {1,1,1,0,0,0,0};
	byte _pinVals[7];	//= {0,0,0,0,0,0,0};	// pinVals gives the input/output values for pins D2,..., D13
	int  _anVals[6];	//= {0,0,0,0,0,0};	// anVals stores the analog input values for pins A0,..., A5
	};

#endif
