#include "Smasher2Vst.h"
#include "Smasher2Editor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	Helpers::Init();
	return new Smasher2Vst(audioMaster);
}

Smasher2Vst::Smasher2Vst(audioMasterCallback audioMaster)
	: VstPlug(audioMaster, (int)Crusher::ParamIndices::NumParams, 4, 2, 'Smh2', new Smasher2())
{
	setEditor(new Smasher2Editor(this));
}

void Smasher2Vst::getParameterName(VstInt32 index, char *text)
{
	switch ((Smasher2::ParamIndices)index)
	{
	case Smasher2::ParamIndices::Sidechain: vst_strncpy(text, "Sidchain", kVstMaxParamStrLen); break;
	case Smasher2::ParamIndices::InputGain: vst_strncpy(text, "In Gain", kVstMaxParamStrLen); break;
	case Smasher2::ParamIndices::Threshold: vst_strncpy(text, "Thres", kVstMaxParamStrLen); break;
	case Smasher2::ParamIndices::Ratio: vst_strncpy(text, "Ratio", kVstMaxParamStrLen); break;
	case Smasher2::ParamIndices::Attack: vst_strncpy(text, "Attack", kVstMaxParamStrLen); break;
	case Smasher2::ParamIndices::Release: vst_strncpy(text, "Release", kVstMaxParamStrLen); break;
	case Smasher2::ParamIndices::OutputGain: vst_strncpy(text, "Out Gain", kVstMaxParamStrLen); break;
	}
}

bool Smasher2Vst::getEffectName(char *name)
{
	vst_strncpy(name, "WaveSabre - Smasher2", kVstMaxEffectNameLen);
	return true;
}

bool Smasher2Vst::getProductString(char *text)
{
	vst_strncpy(text, "WaveSabre - Smasher2", kVstMaxProductStrLen);
	return true;
}
