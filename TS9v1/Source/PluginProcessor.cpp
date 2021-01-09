#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TS9v1AudioProcessor::TS9v1AudioProcessor()
        : parameters (*this, nullptr, Identifier ("TS9v1"),
                      {
                          std::make_unique<AudioParameterFloat> ("drive",            // parameterID
                                                                 "Drive",            // parameter name
                                                                  1.0f,              // minimum value
                                                                  10.0f,              // maximum value
                                                                  5.0f),             // default value
                          std::make_unique<AudioParameterFloat> ("tone",            // parameterID
                                                                 "Tone",            // parameter name
                                                                  1.0f,              // minimum value
                                                                  10.0f,              // maximum value
                                                                  5.0f),             // default value
                          std::make_unique<AudioParameterFloat> ("level",            // parameterID
                                                                 "Level",            // parameter name
                                                                  1.0f,              // minimum value
                                                                  10.0f,              // maximum value
                                                                  5.0f),             // default value
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
    /*formatManager.registerBasicFormats();
    //juce::File file("C:/Users/phili/Desktop/Melodies of the Dead-Master.wav");
    juce::File file("C:/Users/phili/Desktop/Filter/AC30 Live 2.wav");
    auto* reader = formatManager.createReaderFor(file);
    if (reader != nullptr)
    {
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(new juce::AudioFormatReaderSource(reader, true));
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.reset(newSource.release());
        transportSource.start();
    }*/
}

TS9v1AudioProcessor::~TS9v1AudioProcessor()
{
}

//==============================================================================
const juce::String TS9v1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TS9v1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TS9v1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TS9v1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TS9v1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TS9v1AudioProcessor::getNumPrograms()
{
    return 1;
}

int TS9v1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void TS9v1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String TS9v1AudioProcessor::getProgramName (int index)
{
    return {};
}

void TS9v1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void TS9v1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
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

void TS9v1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TS9v1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void TS9v1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
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

    float R2 = 51.E3f + (*driveParameter / 10.0f - 1.0f/10.0f) * 500.E3f;
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
    buffer.applyGain(*levelParameter / 10.0f - 1.0f/10.0f); // ten is the max value of the level slider

    dsp::AudioBlock<float> xf3_block(buffer);
    auto xf3_block_ch0 = xf3_block.getSingleChannelBlock(0);
    dsp::ProcessContextReplacing<float> nnncontext(xf3_block_ch0);
    if(ts9_toggle || ts808_toggle)
       filter4.process(nnncontext);

    buffer.copyFrom(1, 0, buffer.getReadPointer(0), buffer.getNumSamples());
    
}

//==============================================================================
bool TS9v1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* TS9v1AudioProcessor::createEditor()
{
    return new TS9v1AudioProcessorEditor (*this, parameters);
}

//==============================================================================
void TS9v1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
        auto state = parameters.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
}

void TS9v1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new TS9v1AudioProcessor();
}
