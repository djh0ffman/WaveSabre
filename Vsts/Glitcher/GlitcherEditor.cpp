#include "GlitcherEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

GlitcherEditor::GlitcherEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 400, 200, "GLITCHER")
{
}

GlitcherEditor::~GlitcherEditor()
{
}

void GlitcherEditor::Open()
{
	addKnob((VstInt32)Glitcher::ParamIndices::GateAttack, "G ATTACK");
	addKnob((VstInt32)Glitcher::ParamIndices::GateDecay, "G DECAY");
	addKnob((VstInt32)Glitcher::ParamIndices::GateSustain, "G SUSTAIN");
	addKnob((VstInt32)Glitcher::ParamIndices::GateRelease, "G RELEASE");

	startNextRow();

	VstEditor::Open();
}
