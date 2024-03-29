#include "TSProcessor.h"
#include "TSEditor.h"

//==============================================================================
TSAudioProcessor::TSAudioProcessor()
        : parameters (*this, nullptr, Identifier ("TS"),
                      {
                          std::make_unique<AudioParameterFloat> ("drive",            // parameterID
                                                                 "Drive",            // parameter name
                                                                  NormalisableRange<float>(driveRangeMin, driveRangeMax, 0.001f, driveSkewFactor), // range and skew
                                                                  driveSkewMidPoint),     // default value
                          std::make_unique<AudioParameterFloat> ("tone",             // parameterID
                                                                 "Tone",             // parameter name
                                                                  NormalisableRange<float>(toneRangeMin, toneRangeMax, 0.001f, toneSkewFactor), // range and skew
                                                                  toneSkewMidPoint),     // default value
                          std::make_unique<AudioParameterFloat> ("level",            // parameterID
                                                                 "Level",            // parameter name
                                                                  0.0f,              // minimum value
                                                                  1.0f,              // maximum value
                                                                  0.5f),             // default value
                      }),
#ifndef JucePlugin_PreferredChannelConfigurations
      AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	driveParameter = parameters.getRawParameterValue ("drive");
	toneParameter  = parameters.getRawParameterValue ("tone");
	levelParameter  = parameters.getRawParameterValue ("level"); 
}

TSAudioProcessor::~TSAudioProcessor()
{
}

//==============================================================================
const juce::String TSAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TSAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TSAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TSAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TSAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TSAudioProcessor::getNumPrograms()
{
    return 1;
}

int TSAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TSAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TSAudioProcessor::getProgramName (int index)
{
    return {};
}

void TSAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TSAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = static_cast<float>(sampleRate);

    dsp::ProcessSpec spec{ sampleRate, uint32(samplesPerBlock), uint32(getTotalNumOutputChannels()) };
    driveFilter.reset();
    driveFilter.prepare(spec);
    toneFilter.reset();
    toneFilter.prepare(spec);
    
    updateFilterState();

    overSampler.reset();
    overSampler.initProcessing(samplesPerBlock);

    //smoothedValue.reset(currentSampleRate, 0.001);
    //smoothedValue.setTargetValue(toneSkewMidPoint);
}

void TSAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TSAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TSAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    dsp::AudioBlock<float> bufferBlock(buffer);
    auto overSampledBlock = overSampler.processSamplesUp(bufferBlock);
    AudioBuffer<float> upsampleCopy(2, buffer.getNumSamples() * overSampler.getOversamplingFactor());
    dsp::AudioBlock<float> upsampleModBlock(upsampleCopy);

    size_t channelToUse = size_t(0);
    auto overSampledBlockChannel = overSampledBlock.getSingleChannelBlock(channelToUse);
    auto upsampledModBlockChannel = upsampleModBlock.getSingleChannelBlock(channelToUse);
    dsp::ProcessContextNonReplacing<float> driveContext(overSampledBlockChannel, upsampledModBlockChannel);
    
    auto bufferBlockChannel = bufferBlock.getSingleChannelBlock(channelToUse);
    dsp::ProcessContextReplacing<float> toneContext(bufferBlockChannel);
    
    driveFilter.process(driveContext);
    
    float R2 = Rf_drive + (*driveParameter) * Rpot_drive;
    float Is = 1E-14f;
    float nvt = 26.E-3f;
          
    auto upsample_mod_block_ptr = upsampleModBlock.getChannelPointer(0);
    auto upsample_block_ptr = overSampledBlock.getChannelPointer(0);
    for (int i = 0; i < upsampleModBlock.getNumSamples(); ++i) {
		auto U = nvt * asinh(upsample_mod_block_ptr[i] / (2.0f * Is * R2));
		if (std::fabs(U) > std::fabs(upsample_mod_block_ptr[i])) {
		   U = upsample_mod_block_ptr[i];
		}
	    upsample_block_ptr[i] += U;
    }

    overSampler.processSamplesDown(bufferBlock);

    toneFilter.process(toneContext); 

    buffer.applyGain(*levelParameter); 
    buffer.copyFrom(1, 0, buffer.getReadPointer(0), buffer.getNumSamples());
}

//==============================================================================
bool TSAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TSAudioProcessor::createEditor()
{
    return new TSAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void TSAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
        auto state = parameters.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
}

void TSAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

	if (xmlState.get() != nullptr)
		if (xmlState->hasTagName (parameters.state.getType()))
			parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TSAudioProcessor();
}
