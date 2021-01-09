#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

using namespace juce; 

class KnobLookAndFeel : public LookAndFeel_V4
{
public:
	// model = true selects TS9 style knob
	// model = false selects TS808 style knob
    KnobLookAndFeel(bool model) {
		File desktop = File::getSpecialLocation(File::SpecialLocationType::userDesktopDirectory);
		File ts9_knob_image_file = desktop.getFullPathName() + "/JUCEProjects/TS9/Media/knob_ts9.png";
		File ts808_knob_image_file = desktop.getFullPathName() + "/JUCEProjects/TS9/Media/knob_ts808.png";
		if (model) {
			knob_img =ImageCache::getFromFile(ts9_knob_image_file);
	    }
		else {
			knob_img = ImageCache::getFromFile(ts808_knob_image_file);
		}
    }
  
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override {	

		if (knob_img.isValid())
		{
			const double rotation = (slider.getValue()
			- slider.getMinimum())
			/ (slider.getMaximum()
			- slider.getMinimum());
			
			const int frames = knob_img.getHeight() / knob_img.getWidth();
			const int frameId = (int)ceil(rotation * ((double)frames - 1.0));
			const float radius = jmin(width / 2.0f, height / 2.0f);
			const float centerX = x + width * 0.5f;
			const float centerY = y + height * 0.5f;
			const float rx = centerX - radius - 1.0f;
			const float ry = centerY - radius;

			float min = -145.0f * MathConstants<float>::pi / 180.0f;
			float max = 145.0f * MathConstants<float>::pi / 180.0f;
			float rad = jmap<float>(static_cast<float>(slider.getValue() / slider.getMaximum()), min, max);
			
		    AffineTransform rotator;

			g.addTransform(rotator.rotated(rad, (float)(centerX), (float)(centerY)));
			
			g.drawImage(knob_img,
				(int) rx,
				(int) ry,
				2 * (int) radius,
				2 * (int) radius,
				0,
				frameId * knob_img.getWidth(),
				knob_img.getWidth(),
				knob_img.getWidth());

		}
		else
		{
			static const float textPpercent = 0.35f;
			Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f,
			0.5f * height, width * textPpercent, 0.5f * height);
			
			g.setColour(Colours::white);
			
			g.drawFittedText(String("No Image"), text_bounds.getSmallestIntegerContainer(),
			Justification::horizontallyCentred | Justification::centred, 1);
		}
    }
    
	Image knob_img;

};

//==============================================================================
/**
*/
class TS9v1AudioProcessorEditor  : public AudioProcessorEditor, LookAndFeel_V4
{
public:
    TS9v1AudioProcessorEditor (TS9v1AudioProcessor&, AudioProcessorValueTreeState& );
    ~TS9v1AudioProcessorEditor() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
private:

    TS9v1AudioProcessor& audioProcessor;

	KnobLookAndFeel TS9knobLookAndFeel{ true };
	KnobLookAndFeel TS8knobLookAndFeel{ false };

    Slider drive_slider;
    Label drive_label;
	Label drive_value_label;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> driveAttachment;

    Slider tone_slider;
    Label tone_label;
	Label tone_value_label;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> toneAttachment;

    Slider level_slider;
    Label level_label;
	Label level_value_label;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> levelAttachment;

	Label signature_label;

    AudioProcessorValueTreeState& parameters;

	TextButton model_toggle; // State -> true: ts9, false: ts808

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TS9v1AudioProcessorEditor)
};
