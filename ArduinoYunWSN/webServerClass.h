// Web server handler
// Date: 09-06-2014

#ifndef webServerClass_h
#define webServerClass_h

#include <YunClient.h>
//#include <SetXbee.h>

class webServerClass
{
public:
	webServerClass(byte numYunIOPins, byte numXbeeModules,
				   byte *YunPinDirs, byte *YunPinVals, int *YunAnVals,
				   byte *XbeePinDirs, bool *XbeePinVals);
	void serverHandle(YunClient client);
	void setYunPinDirs();
	void setYunPinVals();

private:
	// Number of digital IO that Arduino Yun has available.
	byte _numYunIOPins;

	// Number of Xbee modules available.
	byte _numXbeeModules;


	// pinDirs gives data direction for D6,D7, ... D12
	//	0: output  1: input   2: input with internal pull-up enabled
	byte* _ptrYunPinDirs;	// Pointer to yunPinDirs array.
	byte* _ptrYunPinVals;	// Pointer to yunPinVals array.
	int* _ptrYunAnVals;		// Pointer to yunAnVals array.

	byte* _ptrXbeePinDirs;	// Direction mask array for each IO of Xbee modules.
	bool* _ptrXbeePinVals;	// Value mask array for each IO of Xbee modules.


};

#endif
