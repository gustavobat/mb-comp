#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
        : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                                 .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                                 .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
) {
    compressor.attack = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Attack"));
    compressor.release = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Release"));
    compressor.threshold = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Threshold"));
    compressor.ratio = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("Ratio"));
    compressor.bypassed = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter("Bypassed"));
    jassert(compressor.attack != nullptr);
    jassert(compressor.release != nullptr);
    jassert(compressor.threshold != nullptr);
    jassert(compressor.ratio != nullptr);
    jassert(compressor.bypassed != nullptr);
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() {
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    compressor.prepare(spec);
}

void AudioPluginAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    compressor.updateCompressorSettings();
    compressor.process(buffer);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor() {
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    constexpr auto append = true;
    juce::MemoryOutputStream mos(destData, append);
    apvts.state.writeToStream(mos);
}

void AudioPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    auto treeState = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));
    if (treeState.isValid()) apvts.replaceState(treeState);
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout() {
    APVTS::ParameterLayout layout;
    using namespace juce;

    const auto thresholdRange = NormalisableRange<float>(-60, 12, 1, 1);
    const auto attackReleaseRange = NormalisableRange<float>(5, 500, 1, 1);

    layout.add(std::make_unique<AudioParameterFloat>(
            "Threshold", "Threshold", thresholdRange, 0));

    layout.add(std::make_unique<AudioParameterFloat>(
            "Attack", "Attack", attackReleaseRange, 50));

    layout.add(std::make_unique<AudioParameterFloat>(
            "Release", "Release", attackReleaseRange, 250));

    constexpr auto choices = std::array<float, 14>{1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100};
    StringArray sa;
    std::for_each(choices.begin(), choices.end(), [&sa](auto &choice) {
        sa.add(String(choice, 1));
    });

    layout.add(std::make_unique<AudioParameterChoice>(
            "Ratio", "Ratio", sa, 3));

    layout.add(std::make_unique<AudioParameterBool>("Bypassed", "Bypassed", false));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new AudioPluginAudioProcessor();
}
