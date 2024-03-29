#include "TSProcessor.h"
#include "TSEditor.h"

//==============================================================================
TSAudioProcessorEditor::TSAudioProcessorEditor (TSAudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), parameters(vts), audioProcessor (p), LookAndFeel_V4()
{
    setSize(400, 600);

    drive_slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    drive_slider.setLookAndFeel(&TS8knobLookAndFeel);
    drive_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    drive_slider.setValue(audioProcessor.driveSkewMidPoint);
    drive_slider.setDoubleClickReturnValue(true, audioProcessor.driveSkewMidPoint);
    drive_slider.onValueChange = [this] {
        auto value = drive_slider.getValue();
        auto valueToDisplay = drive_slider.valueToProportionOfLength(value) * 10.0;
		drive_value_label.setText(String::toDecimalStringWithSignificantFigures(valueToDisplay, 2), NotificationType::dontSendNotification);
        audioProcessor.updateFilterState();
    };
	
    tone_slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    tone_slider.setLookAndFeel(&TS8knobLookAndFeel);
    tone_slider.setTextValueSuffix("");
    tone_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    tone_slider.setValue(audioProcessor.toneSkewMidPoint);
    tone_slider.setDoubleClickReturnValue(true, audioProcessor.toneSkewMidPoint);
    tone_slider.onValueChange = [this] {
        auto value = tone_slider.getValue();
        auto valueToDisplay = tone_slider.valueToProportionOfLength(value) * 10.0;
		tone_value_label.setText(String::toDecimalStringWithSignificantFigures(valueToDisplay, 2), NotificationType::dontSendNotification);
        audioProcessor.updateFilterState();
    };

    level_slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    level_slider.setLookAndFeel(&TS8knobLookAndFeel);
    level_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    level_slider.setNormalisableRange(NormalisableRange<double>(0.0, 1.0));
    level_slider.setValue(0.5);
    level_slider.setDoubleClickReturnValue(true, 0.5);
    level_slider.onValueChange = [this] {
		level_value_label.setText(String::toDecimalStringWithSignificantFigures(level_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
        audioProcessor.updateFilterState();
    };

    drive_label.setText("DRIVE", juce::NotificationType::dontSendNotification);
    drive_label.setFont(Font("Segoe UI", 28.0f, 1));
    drive_label.setColour(Label::ColourIds::textColourId, Colours::black);
    drive_label.setJustificationType(Justification::centred);

    tone_label.setText("TONE", juce::NotificationType::dontSendNotification);
    tone_label.setFont(Font("Segoe UI", 24.0f, 1));
    tone_label.setColour(Label::ColourIds::textColourId, Colours::black);
    tone_label.setJustificationType(Justification::centred);

    level_label.setText("LEVEL", juce::NotificationType::dontSendNotification);
    level_label.setFont(Font("Segoe UI", 28.0f, 1));
    level_label.setColour(Label::ColourIds::textColourId, Colours::black);
    level_label.setJustificationType(Justification::centred);

    double driveValue = drive_slider.getValue();
    float driveValueToDisplay = drive_slider.valueToProportionOfLength(driveValue) * 10.0f;
    drive_value_label.setText(String::toDecimalStringWithSignificantFigures(driveValueToDisplay, 2), NotificationType::dontSendNotification);
    drive_value_label.setFont(Font("Segoe UI", 20.0f, 1));
    drive_value_label.setColour(Label::ColourIds::textColourId, Colours::black);
    drive_value_label.setJustificationType(Justification::centred);

    double toneValue = tone_slider.getValue();
    float toneValueToDisplay = tone_slider.valueToProportionOfLength(toneValue) * 10.0f;
    tone_value_label.setText(String::toDecimalStringWithSignificantFigures(toneValueToDisplay, 2), NotificationType::dontSendNotification);
    tone_value_label.setFont(Font("Segoe UI", 18.0f, 1));
    tone_value_label.setColour(Label::ColourIds::textColourId, Colours::black);
    tone_value_label.setJustificationType(Justification::centred);

    level_value_label.setText(String::toDecimalStringWithSignificantFigures(level_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
    level_value_label.setFont(Font("Segoe UI", 20.0f, 1));
    level_value_label.setColour(Label::ColourIds::textColourId, Colours::black);
    level_value_label.setJustificationType(Justification::centred);

    addAndMakeVisible(drive_slider);
    addAndMakeVisible(drive_label);
    addAndMakeVisible(drive_value_label);
    addAndMakeVisible(tone_slider);
    addAndMakeVisible(tone_label);
    addAndMakeVisible(tone_value_label);
    addAndMakeVisible(level_slider);
    addAndMakeVisible(level_label);
    addAndMakeVisible(level_value_label);
   
    driveAttachment.reset (new AudioProcessorValueTreeState::SliderAttachment (parameters, "drive", drive_slider));
    toneAttachment.reset (new AudioProcessorValueTreeState::SliderAttachment (parameters, "tone", tone_slider));
    levelAttachment.reset (new AudioProcessorValueTreeState::SliderAttachment (parameters, "level", level_slider));
    
    /*addAndMakeVisible(signature_label);
    signature_label.setText("by PHILIP COLANGELO", NotificationType::dontSendNotification);
    signature_label.setFont(Font("Inconsolata", 18.0f, Font::plain));
    signature_label.setColour(Label::ColourIds::textColourId, Colours::black);
    signature_label.setJustificationType(Justification::left);*/

}

TSAudioProcessorEditor::~TSAudioProcessorEditor()
{
}

//==============================================================================
void TSAudioProcessorEditor::paint (juce::Graphics& g)
{

    Font logo_font = Font("Bebas Neue", 86.0f, Font::plain);
    String TS("Tube  ");
    String MODEL("Schemer");

	g.fillAll(juce::Colour::fromRGB(48, 182, 116));

    File desktop = File::getSpecialLocation(File::SpecialLocationType::userDesktopDirectory);
    File textureImageFile = desktop.getFullPathName() + "/JUCEProjects/TubeSchemer/Media/MetalTexture.png";
    Image bgImg = ImageCache::getFromFile(textureImageFile);
    Rectangle<float> targetArea(getWidth(), getHeight());
    g.setOpacity(0.20f);
    g.drawImage(bgImg, targetArea, RectanglePlacement::Flags::xLeft | RectanglePlacement::Flags::yTop);

    g.setColour (juce::Colours::white);

    g.setFont(logo_font);
    float border_w = getWidth() * 1.0f;
    float border_h = getHeight() * 1.0f;
    float border_x = 0;
    float border_y = 0;
    Rectangle<float> border(border_x, border_y, border_w, border_h);

    float button_width = logo_font.getStringWidthFloat(MODEL);
    float text_width = logo_font.getStringWidthFloat(TS) + button_width;
    
    float ts_x = (getWidth() - text_width) / 2.0f;
    float nine_x = ts_x + logo_font.getStringWidthFloat(TS);

    float text_area_w = getWidth() * 0.80f;
    float text_area_h = getHeight() * 0.25f;
    //float text_area_x = (getWidth() - text_area_w) / 2.0f; 
    float text_area_y = getHeight() * 0.025f;

    g.setOpacity(0.9f);
    g.drawRoundedRectangle(border, 10.0f, 15.0f);
    g.setColour(Colours::black);
    g.drawText(TS, ts_x, text_area_y, text_area_w, text_area_h, Justification::left);
    g.setColour(Colours::white);
    g.drawText(MODEL, nine_x, text_area_y, text_area_w, text_area_h, Justification::left);
}

void TSAudioProcessorEditor::resized()
{
    auto r = getLocalBounds();

    int knob_y_offset = 75;

    drive_slider.setSize(125, 125);
    drive_slider.setCentrePosition(r.getWidth() / 4, r.getHeight() / 3 + knob_y_offset);

    drive_label.setSize(90, 50);
    drive_label.setCentrePosition(drive_slider.getX() + drive_slider.getWidth() / 2,
        drive_slider.getY() + drive_slider.getHeight() + 10);

    drive_value_label.setSize(50, 50);
    drive_value_label.setCentrePosition(drive_slider.getX() + drive_slider.getWidth() / 2, 
        drive_slider.getY() - 10);

    tone_slider.setSize(100, 100);
    tone_slider.setCentrePosition(r.getWidth() / 2, r.getHeight() / 2 + tone_slider.getHeight() / 2 + knob_y_offset);

    tone_label.setSize(90, 50);
    tone_label.setCentrePosition(tone_slider.getX() + tone_slider.getWidth() / 2,
        tone_slider.getY() + tone_slider.getHeight() + 10);

    tone_value_label.setSize(50, 50);
    tone_value_label.setCentrePosition(tone_slider.getX() + tone_slider.getWidth() / 2, 
        tone_slider.getY() - 10);

    level_slider.setSize(125, 125);
    level_slider.setCentrePosition(r.getWidth() * 3 / 4, r.getHeight() / 3 + knob_y_offset);

    level_label.setSize(90, 50);
    level_label.setCentrePosition(level_slider.getX() + level_slider.getWidth() / 2,
        level_slider.getY() + level_slider.getHeight() + 10);
    
    level_value_label.setSize(50, 50);
    level_value_label.setCentrePosition(level_slider.getX() + level_slider.getWidth() / 2, 
        level_slider.getY() - 10);

    auto sig_bounds = r.removeFromBottom(40);
    sig_bounds = sig_bounds.removeFromRight(r.getWidth() - 15);
    signature_label.setBounds(sig_bounds);
    
}
