#ifndef __SMASHER2EDITOR_H__
#define __SMASHER2EDITOR_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class Smasher2Editor : public VstEditor
{
public:
	Smasher2Editor(AudioEffect *audioEffect);
	virtual ~Smasher2Editor();

	virtual void Open();
};

#endif
