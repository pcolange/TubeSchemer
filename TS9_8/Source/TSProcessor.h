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
    
    void bilinearTransform(
        const std::array<float, 3>& num,
        const std::array<float, 3>& den,
        std::array<float, 3>& bz,
        std::array<float, 3>& az)
    {
        float bz0, bz1, bz2, az0, az1, az2;

        float b2 = num.at(0); 
        float b1 = num.at(1); 
        float b0 = num.at(2);
        float a2 = den.at(0); 
        float a1 = den.at(1); 
        float a0 = den.at(2);

        //where we're pinning to
        float c = 2.0f * current_sample_rate;
        float c_sq = powf(c, 2.0f);

        //apply bilinear transform
        az0 = (a2 * c_sq + a1 * c + a0);

        bz0 = (b2 * c_sq + b1 * c + b0) / az0;
        bz1 = (-2.0f * b2 * c_sq + 2.0f * b0) / az0;
        bz2 = (b2 * c_sq - b1 * c + b0) / az0;

        az1 = (-2.0f * a2 * c_sq + 2.0f * a0) / az0;
        az2 = (a2 * c_sq - a1 * c + a0) / az0;
        az0 = 1.0f;
        
        bz.at(0) = bz0; 
        bz.at(1) = bz1; 
        bz.at(2) = bz2;
        az.at(0) = az0; 
        az.at(1) = az1;
        az.at(2) = az2;
    }

    void updateFilterState() {

        {
			float R1 = 4700.0f;
			float R2 = 51.E3f + (*driveParameter) * 500.E3f;
			float C1 = 0.047E-6f;
			float C2 = 51.E-12f;
            std::array<float, 3> num = { 0.0f, C1 * R2, 0.0f };
            std::array<float, 3> den = { C1 * R1 * C2 * R2, C1 * R1 + C2 * R2, 1.0f };
            std::array<float, 3> az, bz;
            bilinearTransform(num, den, bz, az);
            auto* newCoefficients = new juce::dsp::IIR::Coefficients<float>(bz.at(0), bz.at(1), bz.at(2), az.at(0), az.at(1), az.at(2));
            filter1.coefficients = *newCoefficients;
        }
        {
            float R1 = 1000.0f;
            float R2 = 10000.0f;
            float C1 = 0.22E-6f;
            std::array<float, 3> num = { 0.0f, 0.0f, 1.0f };
            std::array<float, 3> den = { 0.0f, C1 * R1, R1 / R2 + 1.0f }; // c + s + s^2
            std::array<float, 3> az, bz;
            bilinearTransform(num, den, bz, az);
            auto* newCoefficients = new juce::dsp::IIR::Coefficients<float>(bz.at(0), bz.at(2), az.at(0), az.at(2));
            filter2.coefficients = *newCoefficients;
        }
        {
            float Rf = 1.E3f;
            float R = 20.E3f * (1.0f - *toneParameter);
            float R3 = 220.0f;
            float C2 = 0.22E-6f;
            std::array<float, 3> num = { 0.0f, C2 * Rf, 0.0f };
            std::array<float, 3> den = { 0.0f, C2 * (R + R3), 1.0f };
            std::array<float, 3> az, bz;
            bilinearTransform(num, den, bz, az);
            auto* newCoefficients = new juce::dsp::IIR::Coefficients<float>(bz.at(0), bz.at(2), az.at(0), az.at(2));
            filter3.coefficients = *newCoefficients;
        }
        {
            float R1 = model_toggle ? 470.0f : 100.0f; // true TS9, false TS808
            float R2 = model_toggle ? 100.E3f : 10.E3f;
            float C = 10.0E-6f;
            std::array<float, 3> num = { 0.0f, C * R2, 0.0f };
            std::array<float, 3> den = { 0.0f, C * (R1 + R2), 1.0f };
            std::array<float, 3> az, bz;
            bilinearTransform(num, den, bz, az);
            auto* newCoefficients = new juce::dsp::IIR::Coefficients<float>(bz.at(0), bz.at(2), az.at(0), az.at(2));
            filter4.coefficients = *newCoefficients;
        }
    };

    void toggleModel() {
        model_toggle = !model_toggle;
    }


private:
    
    float current_sample_rate = 441000.0f;

    std::atomic<float>* driveParameter = nullptr;
    std::atomic<float>* toneParameter  = nullptr;
    std::atomic<float>* levelParameter  = nullptr;

    bool model_toggle = true; // true: TS9, false: TS808

    using FilterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    dsp::IIR::Filter<float> filter1;
    dsp::IIR::Filter<float> filter2;
    dsp::IIR::Filter<float> filter3;
    dsp::IIR::Filter<float> filter4;

    AudioProcessorValueTreeState parameters;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TSAudioProcessor)
};
