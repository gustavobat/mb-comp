#include "MBCompProcessor.h"
#include "MBCompEditor.h"

MBCompEditor::MBCompEditor(MBCompProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p) {
    juce::ignoreUnused(processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 300);
}

MBCompEditor::~MBCompEditor() {
}

void MBCompEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::antiquewhite);
    g.setFont(15.0f);
    g.drawFittedText("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void MBCompEditor::resized() {
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor.
}
