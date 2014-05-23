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
	// pinDirs gives data direction for D2,D3, ... D12
	//	0:	output
	//	1:	input
	//	2:	input with internal pull-up enabled
	byte pinDirs[11];// = {1,1,1,1,1,1,1,1,0,0,0};

	byte pinVals[11]; //= {0,0,0,0,0,0,0,0,0,0,0};	// pinVals gives the input/output values for pins D2,..., D13
	int  anVals[6]; //= {0,0,0,0,0,0};	// anVals stores the analog input values for pins A0,..., A5
	
public:
	webServerClass(byte _pinDirs[11], byte _pinVals[11], int _anVals[6]);
	void serverHandle(YunClient client);
	void setPinDirs();
	void setPinVals();
};

#endif
