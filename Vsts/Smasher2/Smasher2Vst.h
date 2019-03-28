#ifndef __SMASHER2VST_H__
#define __SMASHER2VST_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class Smasher2Vst : public VstPlug
{
public:
	Smasher2Vst(audioMasterCallback audioMaster);

	virtual void getParameterName(VstInt32 index, char *text);

	virtual bool getEffectName(char *name);
	virtual bool getProductString(char *text);
};

#endif