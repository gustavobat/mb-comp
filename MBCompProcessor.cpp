#include "MBCompProcessor.h"

MBCompProcessor::MBCompProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
) {

    using namespace Params;
    const auto &params = GetParamNames();

    auto get_float_param = [&apvts = this->apvts, &params](auto &param, const auto &paramName) {
        param = dynamic_cast<juce::AudioParameterFloat *>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    auto get_choice_param = [&apvts = this->apvts, &params](auto &param, const auto &paramName) {
        param = dynamic_cast<juce::AudioParameterChoice *>(
            apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    auto get_bool_param = [&apvts = this->apvts, &params](auto &param, const auto &paramName) {
        param = dynamic_cast<juce::AudioParameterBool *>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    get_float_param(compressor.attack, Names::AttackL);
    get_float_param(compressor.release, Names::ReleaseL);
    get_float_param(compressor.threshold, Names::ThresholdL);
    get_choice_param(compressor.ratio, Names::RatioL);
    get_bool_param(compressor.bypassed, Names::BypassedL);
}

MBCompProcessor::~MBCompProcessor() = default;

const juce::String MBCompProcessor::getName() const {
    return JucePlugin_Name;
}

bool MBCompProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool MBCompProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool MBCompProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double MBCompProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int MBCompProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int MBCompProcessor::getCurrentProgram() {
    return 0;
}

void MBCompProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String MBCompProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void MBCompProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

void MBCompProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    compressor.prepare(spec);
}

void MBCompProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool MBCompProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
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

void MBCompProcessor::processBlock(juce::AudioBuffer<float> &buffer,
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

bool MBCompProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *MBCompProcessor::createEditor() {
    return new juce::GenericAudioProcessorEditor(*this);
}

void MBCompProcessor::getStateInformation(juce::MemoryBlock &destData) {
    constexpr auto append = true;
    juce::MemoryOutputStream mos(destData, append);
    apvts.state.writeToStream(mos);
}

void MBCompProcessor::setStateInformation(const void *data, int sizeInBytes) {
    auto treeState = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));
    if (treeState.isValid()) apvts.replaceState(treeState);
}

juce::AudioProcessorValueTreeState::ParameterLayout
MBCompProcessor::createParameterLayout() {

    APVTS::ParameterLayout layout;

    using namespace juce;
    using namespace Params;
    const auto & params = GetParamNames();


    const auto thresholdRange = NormalisableRange<float>(-60, 12, 1, 1);
    const auto attackReleaseRange = NormalisableRange<float>(5, 500, 1, 1);

    layout.add(std::make_unique<AudioParameterFloat>(
        params.at(Names::ThresholdL),
        params.at(Names::ThresholdL),
        thresholdRange, 0));

    layout.add(std::make_unique<AudioParameterFloat>(
        params.at(Names::AttackL),
        params.at(Names::AttackL),
        attackReleaseRange, 50));

    layout.add(std::make_unique<AudioParameterFloat>(
        params.at(Names::ReleaseL),
        params.at(Names::ReleaseL),
        attackReleaseRange, 250));

    constexpr auto choices = std::array<float, 14>{
        1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100
    };

    StringArray ratio_options;
    std::for_each(choices.begin(), choices.end(), [&ratio_options](auto &choice) {
        ratio_options.add(String(choice, 1));
    });
    const auto default_ratio_index = 3;
    layout.add(std::make_unique<AudioParameterChoice>(
        params.at(Names::RatioL),
        params.at(Names::RatioL),
        ratio_options, default_ratio_index));

    layout.add(std::make_unique<AudioParameterBool>(
        params.at(Names::BypassedL),
        params.at(Names::BypassedL),
        false));

    return layout;
}

// This creates new instances of the plugin.
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new MBCompProcessor();
}
