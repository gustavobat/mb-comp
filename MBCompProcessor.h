#pragma once

#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace Params {
    enum Names {
        LM_CrossOverFreq,
        MH_CrossOverFreq,
        AttackL,
        AttackM,
        AttackH,
        ReleaseL,
        ReleaseM,
        ReleaseH,
        RatioL,
        RatioM,
        RatioH,
        BypassedL,
        BypassedM,
        BypassedH,
    };

    inline const std::map<Names, juce::String> &GetParamNames() {
        static std::map<Names, juce::String> params = {
            {LM_CrossOverFreq, "Low-Mid Crossover Frequency"},
            {MH_CrossOverFreq, "Mid-High Crossover Frequency"},
            {AttackL,          "Low Band Threshold"},
            {AttackM,          "Mid Band Threshold"},
            {AttackH,          "High Band Threshold"},
            {ReleaseL,         "Low Band Release"},
            {ReleaseM,         "Mid Band Release"},
            {ReleaseH,         "High Band Release"},
            {RatioL,           "Low Band Ratio"},
            {RatioM,           "Mid Band Ratio"},
            {RatioH,           "High Band Ratio"},
            {BypassedL,        "Low Band Bypassed"},
            {BypassedM,        "Mid Band Bypassed"},
            {BypassedH,        "High Band Bypassed"},
        };
    }
}

struct CompressorBand {
    // Parameter cache
    juce::AudioParameterFloat *attack{};
    juce::AudioParameterFloat *release{};
    juce::AudioParameterFloat *threshold{};
    juce::AudioParameterChoice *ratio{};
    juce::AudioParameterBool *bypassed{};

    void prepare(const juce::dsp::ProcessSpec &spec) { compressor.prepare(spec); }

    void updateCompressorSettings() {
        compressor.setAttack(attack->get());
        compressor.setRelease(release->get());
        compressor.setThreshold(threshold->get());
        compressor.setRatio(ratio->getCurrentChoiceName().getFloatValue());
    }

    void process(juce::AudioBuffer<float> &buffer) {
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);

        context.isBypassed = bypassed->get();
        compressor.process(context);
    }

private:
    juce::dsp::Compressor<float> compressor;
};

class MBCompProcessor : public juce::AudioProcessor {
public:
    MBCompProcessor();

    ~MBCompProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor *createEditor() override;

    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;

    bool producesMidi() const override;

    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;

    int getNumPrograms() override;

    int getCurrentProgram() override;

    void setCurrentProgram(int index) override;

    const juce::String getProgramName(int index) override;

    void changeProgramName(int index, const juce::String &newName) override;

    void getStateInformation(juce::MemoryBlock &destData) override;

    void setStateInformation(const void *data, int sizeInBytes) override;

    using APVTS = juce::AudioProcessorValueTreeState;

    static APVTS::ParameterLayout createParameterLayout();

    APVTS apvts{*this, nullptr, "Parameters", createParameterLayout()};

private:
    CompressorBand compressor;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MBCompProcessor)
};
