#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include "MBCompProcessor.h"

class MBCompEditor : public juce::AudioProcessorEditor {
public:
    explicit MBCompEditor(MBCompProcessor &);

    ~MBCompEditor() override;

    void paint(juce::Graphics &) override;

    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MBCompProcessor &processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MBCompEditor)
};
