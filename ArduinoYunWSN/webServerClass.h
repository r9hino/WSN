// Web server handler
// Date: 08-05-2014

#ifndef webServerClass_h
#define webServerClass_h

#include <YunClient.h>

class webServerClass
{
private:
	int i;	
	
	// general loop variable
	// pinDirs gives data direction for D2,D3, ... D13
	//	0:	output
	//	1:	input
	//	2:	input with internal pull-up enabled
	// D13 initialised as output to drive on-board LED
	// use internal pull-up with care when making D13 input! 
	byte pinDirs[12];// = {1,1,1,1,1,1,1,1,1,0,0,0};

	byte pinVals[12]; //= {0,0,0,0,0,0,0,0,0,0,0,0};	// pinVals gives the input/output values for pins D2,..., D13
	int  anVals[6]; //= {0,0,0,0,0,0};	// anVals stores the analog input values for pins A0,..., A5
	
public:
	webServerClass(byte _pinDirs[12], byte _pinVals[12], int _anVals[6]);
	void serverHandle(YunClient client);
	void setPinDirs();
	void setPinVals();
};

#endif
