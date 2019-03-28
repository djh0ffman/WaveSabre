#include "Smasher2Editor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

Smasher2Editor::Smasher2Editor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 330, 160, "SMASHER 2")
{
}

Smasher2Editor::~Smasher2Editor()
{
}

void Smasher2Editor::Open()
{
	addKnob((VstInt32)Smasher2::ParamIndices::InputGain, "INPUT GAIN");
	addSpacer();
	addKnob((VstInt32)Smasher2::ParamIndices::Threshold, "THRESHOLD");
	addSpacer();
	addKnob((VstInt32)Smasher2::ParamIndices::Attack, "ATTACK");
	addSpacer();
	addKnob((VstInt32)Smasher2::ParamIndices::OutputGain, "OUTPUT GAIN");

	startNextRow();

	addKnob((VstInt32)Smasher2::ParamIndices::Sidechain, "SIDECHAIN");
	addSpacer();
	addKnob((VstInt32)Smasher2::ParamIndices::Ratio, "RATIO");
	addSpacer();
	addKnob((VstInt32)Smasher2::ParamIndices::Release, "RELEASE");

	VstEditor::Open();
}
