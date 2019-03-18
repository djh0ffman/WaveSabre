#include "GlitcherEditor.h"

#include <WaveSabreCore.h>
using namespace WaveSabreCore;

GlitcherEditor::GlitcherEditor(AudioEffect *audioEffect)
	: VstEditor(audioEffect, 550, 280, "GLITCHER")
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

	addKnob((VstInt32)Glitcher::ParamIndices::FilterType, "FLT TYPE");
	addKnob((VstInt32)Glitcher::ParamIndices::FilterFreq, "FLT FREQ");
	addKnob((VstInt32)Glitcher::ParamIndices::FilterQ, "FLT Q");
	addKnob((VstInt32)Glitcher::ParamIndices::FilterWave, "LFO WAVE");
	addKnob((VstInt32)Glitcher::ParamIndices::FilterLfoAmount, "LFO AMT");
	addKnob((VstInt32)Glitcher::ParamIndices::FilterLfoRate, "LFO RATE");
	addKnob((VstInt32)Glitcher::ParamIndices::FilterLfoPhase, "LFO PHASE");
	addKnob((VstInt32)Glitcher::ParamIndices::FilterVeloAmount, "VEL AMT");
	addKnob((VstInt32)Glitcher::ParamIndices::FilterVeloRate, "VEL RATE");
	addKnob((VstInt32)Glitcher::ParamIndices::FilterTempoSync, "TEMPO SYNC");

	startNextRow();

	addKnob((VstInt32)Glitcher::ParamIndices::RepeatLoopLength, "LOP LEN");
	addKnob((VstInt32)Glitcher::ParamIndices::RepeatLoopMode, "LOOP MODE");
	addKnob((VstInt32)Glitcher::ParamIndices::RepeatPitch, "PITCH");
	VstEditor::Open();
}
