#ifndef __GLITCHEREDITOR_H__
#define __GLITCHEREDITOR_H__

#include <WaveSabreVstLib.h>
using namespace WaveSabreVstLib;

class GlitcherEditor : public VstEditor
{
public:
	GlitcherEditor(AudioEffect *audioEffect);
	virtual ~GlitcherEditor();

	virtual void Open();
};

#endif
