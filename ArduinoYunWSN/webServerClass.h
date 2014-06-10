// Web server handler
// Date: 09-06-2014

#ifndef webServerClass_h
#define webServerClass_h

#include <YunClient.h>
//#include <SetXbee.h>

class webServerClass
{
public:
	webServerClass(byte yunIOPins, byte numXbeeModules,
				   byte YunPinDirs[7], byte YunPinVals[7], int YunAnVals[6],
				   byte *ptrXbeePinDirs, bool *ptrXbeePinVals);
	void serverHandle(YunClient client);
	void setYunPinDirs();
	void setYunPinVals();

private:
	// Number of digital IO that Arduino Yun has available.
	byte _yunIOPins;

	// Number of Xbee modules available.
	byte _numXbeeModules;


	// pinDirs gives data direction for D6,D7, ... D12
	//	0: output  1: input   2: input with internal pull-up enabled
	byte _YunPinDirs[7];	// = {1,1,1,0,0,0,0};
	byte _YunPinVals[7];	//= {0,0,0,0,0,0,0};	// pinVals gives the input/output values for pins D2,..., D13
	int  _YunAnVals[6];	//= {0,0,0,0,0,0};	// anVals stores the analog input values for pins A0,..., A5

	byte* _ptrXbeePinDirs;	// Direction mask array for each IO of Xbee modules.
	bool* _ptrXbeePinVals;	// Value mask array for each IO of Xbee modules.


};

#endif
