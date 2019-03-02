#include "GlitcherVst.h"
#include "GlitcherEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	Helpers::Init();
	return new GlitcherVst(audioMaster);
}

GlitcherVst::GlitcherVst(audioMasterCallback audioMaster)
	: VstPlug(audioMaster, (int)Glitcher::ParamIndices::NumParams, 2, 2, 'Glcr', new Glitcher(), false, true)
{
	setEditor(new GlitcherEditor(this));
}

void GlitcherVst::getParameterName(VstInt32 index, char *text)
{
	switch ((Glitcher::ParamIndices)index)
	{
	case Glitcher::ParamIndices::GateAttack: vst_strncpy(text, "Gat Atk", kVstMaxParamStrLen); break;
	case Glitcher::ParamIndices::GateDecay: vst_strncpy(text, "Gat Dcy", kVstMaxParamStrLen); break;
	case Glitcher::ParamIndices::GateSustain: vst_strncpy(text, "Gat Sus", kVstMaxParamStrLen); break;
	case Glitcher::ParamIndices::GateRelease: vst_strncpy(text, "Gat Rls", kVstMaxParamStrLen); break;

	case Glitcher::ParamIndices::FilterFreq: vst_strncpy(text, "Flt Frq", kVstMaxParamStrLen); break;
	case Glitcher::ParamIndices::FilterLfoAmount: vst_strncpy(text, "Flt Lfo Amt", kVstMaxParamStrLen); break;
	case Glitcher::ParamIndices::FilterLfoRateAdjust: vst_strncpy(text, "Flt Rat Adj", kVstMaxParamStrLen); break;
	case Glitcher::ParamIndices::FilterWave: vst_strncpy(text, "Flt Lfo Wav", kVstMaxParamStrLen); break;
	}
}

bool GlitcherVst::getEffectName(char *name)
{
	vst_strncpy(name, "WaveSabre - Glitcher", kVstMaxEffectNameLen);
	return true;
}

bool GlitcherVst::getProductString(char *text)
{
	vst_strncpy(text, "WaveSabre - Glitcher", kVstMaxProductStrLen);
	return true;
}
