#ifndef __GLITCHERVST_H__
#define __GLITCHERVST_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class GlitcherVst : public VstPlug
{
public:
	GlitcherVst(audioMasterCallback audioMaster);

	virtual void getParameterName(VstInt32 index, char *text);

	virtual bool getEffectName(char *name);
	virtual bool getProductString(char *text);
};

#endif