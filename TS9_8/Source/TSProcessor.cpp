#include "TSProcessor.h"
#include "TSEditor.h"

//==============================================================================
TSAudioProcessor::TSAudioProcessor()
        : parameters (*this, nullptr, Identifier ("TS"),
                      {
                          std::make_unique<AudioParameterFloat> ("drive",            // parameterID
                                                                 "Drive",            // parameter name
                                                                  0.0f,              // minimum value
                                                                  1.0f,              // maximum value
                                                                  0.7f),             // default value
                          std::make_unique<AudioParameterFloat> ("tone",            // parameterID
                                                                 "Tone",            // parameter name
                                                                  0.0f,              // minimum value
                                                                  1.0f,              // maximum value
                                                                  0.5f),             // default value
                          std::make_unique<AudioParameterFloat> ("level",            // parameterID
                                                                 "Level",            // parameter name
                                                                  0.0f,              // minimum value
                                                                  1.0f,              // maximum value
                                                                  0.7f),             // default value
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
    current_sample_rate = static_cast<float>(sampleRate);

    dsp::ProcessSpec spec{ sampleRate, uint32(samplesPerBlock), uint32(getTotalNumOutputChannels()) };

    filter1.reset();
    filter1.prepare(spec);
    filter2.reset();
    filter2.prepare(spec);
    filter3.reset();
    filter3.prepare(spec);
    filter4.reset();
    filter4.prepare(spec);
    updateFilterState();

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

    AudioBuffer<float> yf1_buffer(1, buffer.getNumSamples());

    dsp::AudioBlock<float> xf1_block(buffer);
    dsp::AudioBlock<float> yf1_block(yf1_buffer);
    
    auto xf1_block_ch0 = xf1_block.getSingleChannelBlock(0);
    auto yf1_block_ch0 = yf1_block.getSingleChannelBlock(0);
    dsp::ProcessContextNonReplacing<float> context(xf1_block_ch0, yf1_block_ch0);
    filter1.process(context);

    float R2 = 51.E3f + (*driveParameter) * 500.E3f;
    float Is = 1E-14f;
    float nvt = 26.E-3f;

    auto* sig = buffer.getReadPointer(0);
	auto* vdi = yf1_buffer.getWritePointer(0);
    for (int i = 0; i < buffer.getNumSamples(); ++i) {
		auto U = nvt * asinh(vdi[i] / (2.0f * Is * R2));
		if (std::fabs(U) > std::fabs(vdi[i])) {
		   U = vdi[i];
		}
		vdi[i] = U + sig[i];
    }
    
    yf1_block_ch0 = yf1_block.getSingleChannelBlock(0);
    dsp::ProcessContextReplacing<float> ncontext(yf1_block_ch0);
    filter2.process(ncontext); // produces xf1
    
    xf1_block_ch0 = xf1_block.getSingleChannelBlock(0);
    dsp::ProcessContextNonReplacing<float> nncontext(yf1_block_ch0, xf1_block_ch0);
    filter3.process(nncontext); // produces xf2
    
    // The final output is xf1+xf2
    buffer.addFrom(0, 0, yf1_buffer.getReadPointer(0), buffer.getNumSamples(), 1.0f);
    buffer.applyGain(*levelParameter); // ten is the max value of the level slider

    dsp::AudioBlock<float> xf3_block(buffer);
    auto xf3_block_ch0 = xf3_block.getSingleChannelBlock(0);
    dsp::ProcessContextReplacing<float> nnncontext(xf3_block_ch0);
    filter4.process(nnncontext);

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
