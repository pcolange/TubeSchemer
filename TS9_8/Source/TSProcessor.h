#pragma once

#include <JuceHeader.h>

//==============================================================================
class TSAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    TSAudioProcessor();
    ~TSAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void updateFilterState() {

		float Fs = currentSampleRate;
		float Fs2 = std::powf(Fs, 2.0f);
        
        {
            float Rdrive = Rpot_drive * (*driveParameter) + Rf_drive;

            float bz2 =  4.0f * C1 * Cf * R1 * Rdrive * Fs2 + 2.0f * C1 * R1 * Fs + 2.0f * C1 * Rdrive * Fs + 2.0f * Cf * Rdrive * Fs + 1.0f;
            float bz1 = -8.0f * C1 * Cf * R1 * Rdrive * Fs2 + 2.0f;
            float bz0 =  4.0f * C1 * Cf * R1 * Rdrive * Fs2 - 2.0f * C1 * R1 * Fs - 2.0f * C1 * Rdrive * Fs - 2.0f * Cf * Rdrive * Fs + 1.0f;

            float az2 =  4.0f * C1 * Cf * R1 * Rdrive * Fs2 + 2.0f * C1 * R1 * Fs + 2.0f * Cf * Rdrive * Fs + 1.0f;
            float az1 = -8.0f * C1 * Cf * R1 * Rdrive * Fs2 + 2.0f;
            float az0 =  4.0f * C1 * Cf * R1 * Rdrive * Fs2 - 2.0f * C1 * R1 * Fs - 2.0f * Cf * Rdrive * Fs + 1.0f;

            bz2 /= az2;
            bz1 /= az2;
            bz0 /= az2;
            az1 /= az2;
            az0 /= az2;
            az2 = 1.0f;

            auto* newCoefficients = new juce::dsp::IIR::Coefficients<float>(bz2, bz1, bz0, az2, az1, az0);
            driveFilter.coefficients = *newCoefficients;

        }

        {
           // float smoothedTone = smoothedValue.getCurrentValue();
           //smoothedValue.setTargetValue(*toneParameter);
           // float Rpot1 = Rpot_tone * smoothedTone;
           // float Rpot2 = Rpot_tone * (1.0f - smoothedTone);
            float Rpot1 = Rpot_tone * (*toneParameter);
            float Rpot2 = Rpot_tone * (toneRangeMax - *toneParameter);
            
            float bz2 =  2.0f * Ctone * R10k * R220 * Rpot1 * Fs + 2.0f * Ctone * R10k * R220 * Rpot2 * Fs + 2.0f * Ctone * R10k * Rf_tone * Rpot1 * Fs + 2.0f * Ctone * R10k * Rpot1 * Rpot2 * Fs + R10k * Rpot1 + R10k * Rpot2;
            float bz1 =  2.0f * R10k * Rpot1 + 2.0f * R10k * Rpot2;
            float bz0 = -2.0f * Ctone * R10k * R220 * Rpot1 * Fs - 2.0f * Ctone * R10k * R220 * Rpot2 * Fs - 2.0f * Ctone * R10k * Rf_tone * Rpot1 * Fs - 2.0f * Ctone * R10k * Rpot1 * Rpot2 * Fs + R10k * Rpot1 + R10k * Rpot2;

            float az2 =  4.0f * C4 * Ctone * R10k * R1k * R220 * Rpot1 * Fs2 + 4.0f * C4 * Ctone * R10k * R1k * R220 * Rpot2 * Fs2 + 4.0f * C4 * Ctone * R10k * R1k * Rpot1 * Rpot2 * Fs2 + 2.0f * C4 * R10k * R1k * Rpot1 * Fs + 2.0f * C4 * R10k * R1k * Rpot2 * Fs + 2.0f * Ctone * R10k * R1k * Rpot2 * Fs + 2.0f * Ctone * R10k * R220 * Rpot1 * Fs + 2.0f * Ctone * R10k * R220 * Rpot2 * Fs + 2.0f * Ctone * R10k * Rpot1 * Rpot2 * Fs + 2.0f * Ctone * R1k * R220 * Rpot1 * Fs + 2.0f * Ctone * R1k * R220 * Rpot2 * Fs + 2.0f * Ctone * R1k * Rpot1 * Rpot2 * Fs + R10k * Rpot1 + R10k * Rpot2 + R1k * Rpot1 + R1k * Rpot2;
            float az1 = -8.0f * C4 * Ctone * R10k * R1k * R220 * Rpot1 * Fs2 - 8.0f * C4 * Ctone * R10k * R1k * R220 * Rpot2 * Fs2 - 8.0f * C4 * Ctone * R10k * R1k * Rpot1 * Rpot2 * Fs2 + 2.0f * R10k * Rpot1 + 2 * R10k * Rpot2 + 2.0f * R1k * Rpot1 + 2.0f * R1k * Rpot2;
            float az0 =  4.0f * C4 * Ctone * R10k * R1k * R220 * Rpot1 * Fs2 + 4.0f * C4 * Ctone * R10k * R1k * R220 * Rpot2 * Fs2 + 4.0f * C4 * Ctone * R10k * R1k * Rpot1 * Rpot2 * Fs2 - 2.0f * C4 * R10k * R1k * Rpot1 * Fs - 2.0f * C4 * R10k * R1k * Rpot2 * Fs - 2.0f * Ctone * R10k * R1k * Rpot2 * Fs - 2.0f * Ctone * R10k * R220 * Rpot1 * Fs - 2.0f * Ctone * R10k * R220 * Rpot2 * Fs - 2.0f * Ctone * R10k * Rpot1 * Rpot2 * Fs - 2.0f * Ctone * R1k * R220 * Rpot1 * Fs - 2.0f * Ctone * R1k * R220 * Rpot2 * Fs - 2.0f * Ctone * R1k * Rpot1 * Rpot2 * Fs + R10k * Rpot1 + R10k * Rpot2 + R1k * Rpot1 + R1k * Rpot2;
            
            bz2 /= az2;
            bz1 /= az2;
            bz0 /= az2;
            az1 /= az2;
            az0 /= az2;
            az2 = 1.0f;

            auto* newCoefficients = new juce::dsp::IIR::Coefficients<float>(bz2, bz1, bz0, az2, az1, az0);
            toneFilter.coefficients = *newCoefficients;
        }
    };
    
    const float driveRangeMin = 0.0f;
    const float driveRangeMax = 1.0f;
    const float driveSkewMidPoint = 0.5f;
    const float driveSkewFactor = std::log (0.5f) / std::log ((driveSkewMidPoint - driveRangeMin) / (driveRangeMax - driveRangeMin));
    
    const float toneRangeMin = 0.0f;
    const float toneRangeMax = 1.0f;
    const float toneSkewMidPoint = 0.8f;
    const float toneSkewFactor = std::log (0.5f) / std::log ((toneSkewMidPoint - toneRangeMin) / (toneRangeMax - toneRangeMin));


private:

    // Circuit values for the OpAmp drive section
    float Rpot_drive = 550E3f; // normally 500 
    float Rf_drive = 51E3f;
    float Cf = 51E-12f;
    float R1 = 4700.0f;
    float C1 = 0.047E-6f;

    // Circuit values for the OpAmp tone section
    float Rf_tone = 1E3f;
    float Rpot_tone = 20E3f;
    float R10k = 10E3f;
    float R1k = 1E3f;
    float R220 = 220.0f;
    float C4 = 0.22E-6f;
    float Ctone = 0.22E-6f;
    
    float currentSampleRate = 44100.0f;

    const int overSampleRatio = 1;

    dsp::Oversampling<float> overSampler{ getTotalNumOutputChannels(), overSampleRatio, dsp::Oversampling<float>::filterHalfBandFIREquiripple, true };

    std::atomic<float>* driveParameter = nullptr;
    std::atomic<float>* toneParameter  = nullptr;
    std::atomic<float>* levelParameter  = nullptr;

    dsp::IIR::Filter<float> driveFilter;
    dsp::IIR::Filter<float> toneFilter;

    AudioProcessorValueTreeState parameters;

    LinearSmoothedValue<float> smoothedValue;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TSAudioProcessor)
};
