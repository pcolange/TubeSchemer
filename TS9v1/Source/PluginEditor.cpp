#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TS9v1AudioProcessorEditor::TS9v1AudioProcessorEditor (TS9v1AudioProcessor& p, AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), parameters(vts), audioProcessor (p), LookAndFeel_V4()
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(400, 600);

    // Both buttons are mutually exclusive
    addAndMakeVisible(model_toggle);
    model_toggle.setToggleState(true, NotificationType::dontSendNotification);
    model_toggle.setClickingTogglesState(true);
    model_toggle.onClick = [this] {
        audioProcessor.toggleModel();
        audioProcessor.updateFilterState();
        repaint();
    };

    addAndMakeVisible(drive_slider);
    drive_slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    drive_slider.setLookAndFeel(&TS9knobLookAndFeel);
    drive_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    drive_slider.setNormalisableRange(NormalisableRange<double>(0.0, 1.0));
    drive_slider.setValue(7.0);
    drive_slider.setDoubleClickReturnValue(true, 5);
    drive_slider.onValueChange = [this] {
		drive_value_label.setText(String::toDecimalStringWithSignificantFigures(drive_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
        audioProcessor.updateFilterState();
    };
	
    driveAttachment.reset (new AudioProcessorValueTreeState::SliderAttachment (parameters, "drive", drive_slider));

    addAndMakeVisible(drive_label);
    drive_label.setText("DRIVE", juce::NotificationType::dontSendNotification);
    drive_label.setFont(Font("Segoe UI", 28.0f, 1));
    drive_label.setColour(Label::ColourIds::textColourId, Colours::black);
    drive_label.setJustificationType(Justification::centred);

    addAndMakeVisible(drive_value_label);
    drive_value_label.setText(String::toDecimalStringWithSignificantFigures(drive_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
    drive_value_label.setFont(Font("Segoe UI", 20.0f, 1));
    drive_value_label.setColour(Label::ColourIds::textColourId, Colours::black);
    drive_value_label.setJustificationType(Justification::centred);

    addAndMakeVisible(tone_slider);
    tone_slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    tone_slider.setLookAndFeel(&TS9knobLookAndFeel);
    tone_slider.setTextValueSuffix("");
    tone_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    tone_slider.setNormalisableRange(NormalisableRange<double>(0.0, 1.0));
    tone_slider.setValue(5.0);
    tone_slider.setDoubleClickReturnValue(true, 5);
    tone_slider.onValueChange = [this] {
		tone_value_label.setText(String::toDecimalStringWithSignificantFigures(tone_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
        audioProcessor.updateFilterState();
    };

    toneAttachment.reset (new AudioProcessorValueTreeState::SliderAttachment (parameters, "tone", tone_slider));

    addAndMakeVisible(tone_label);
    tone_label.setText("TONE", juce::NotificationType::dontSendNotification);
    tone_label.setFont(Font("Segoe UI", 24.0f, 1));
    tone_label.setColour(Label::ColourIds::textColourId, Colours::black);
    tone_label.setJustificationType(Justification::centred);

    addAndMakeVisible(tone_value_label);
    tone_value_label.setText(String::toDecimalStringWithSignificantFigures(tone_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
    tone_value_label.setFont(Font("Segoe UI", 18.0f, 1));
    tone_value_label.setColour(Label::ColourIds::textColourId, Colours::black);
    tone_value_label.setJustificationType(Justification::centred);

    addAndMakeVisible(level_slider);
    level_slider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    level_slider.setLookAndFeel(&TS9knobLookAndFeel);
    level_slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    level_slider.setNormalisableRange(NormalisableRange<double>(0.0, 1.0));
    level_slider.setValue(7.0);
    level_slider.setDoubleClickReturnValue(true, 5);
    level_slider.onValueChange = [this] {
		level_value_label.setText(String::toDecimalStringWithSignificantFigures(level_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
        audioProcessor.updateFilterState();
    };

    levelAttachment.reset (new AudioProcessorValueTreeState::SliderAttachment (parameters, "level", level_slider));

    addAndMakeVisible(level_label);
    level_label.setText("LEVEL", juce::NotificationType::dontSendNotification);
    level_label.setFont(Font("Segoe UI", 28.0f, 1));
    level_label.setColour(Label::ColourIds::textColourId, Colours::black);
    level_label.setJustificationType(Justification::centred);

    addAndMakeVisible(level_value_label);
    level_value_label.setText(String::toDecimalStringWithSignificantFigures(level_slider.getValue() * 10.0, 2), NotificationType::dontSendNotification);
    level_value_label.setFont(Font("Segoe UI", 20.0f, 1));
    level_value_label.setColour(Label::ColourIds::textColourId, Colours::black);
    level_value_label.setJustificationType(Justification::centred);

    addAndMakeVisible(signature_label);
    signature_label.setText("by PHILIP COLANGELO", NotificationType::dontSendNotification);
    signature_label.setFont(Font("Inconsolata", 18.0f, Font::plain));
    signature_label.setColour(Label::ColourIds::textColourId, Colours::black);
    signature_label.setJustificationType(Justification::left);

}

TS9v1AudioProcessorEditor::~TS9v1AudioProcessorEditor()
{
}

//==============================================================================
void TS9v1AudioProcessorEditor::paint (juce::Graphics& g)
{

    Font logo_font = Font("Bebas Neue", 128.0f, Font::plain);
    String TS("TS  ");
    String MODEL;

    if (model_toggle.getToggleState()) {
        g.fillAll(juce::Colour::fromRGB(50, 168, 82));
        drive_slider.setLookAndFeel(&TS9knobLookAndFeel);
        tone_slider.setLookAndFeel(&TS9knobLookAndFeel);
        level_slider.setLookAndFeel(&TS9knobLookAndFeel);
        MODEL = "NINE";
    }
    else {
        g.fillAll(juce::Colour::fromRGB(48, 182, 116));
        drive_slider.setLookAndFeel(&TS8knobLookAndFeel);
        tone_slider.setLookAndFeel(&TS8knobLookAndFeel);
        level_slider.setLookAndFeel(&TS8knobLookAndFeel);
        MODEL = "EIGHT";
    }

    model_toggle.setAlpha(0.0f);
    
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

    model_toggle.setBounds(nine_x, text_area_y + 25, button_width, text_area_h - 60);
   
    g.setOpacity(0.9f);
    if(!model_toggle.getToggleState())
      g.drawRoundedRectangle(border, 10.0f, 15.0f);
    g.setColour(Colours::black);
    g.drawText(TS, ts_x, text_area_y, text_area_w, text_area_h, Justification::left);
    g.setColour(Colours::white);
    g.drawText(MODEL, nine_x, text_area_y, text_area_w, text_area_h, Justification::left);
}

void TS9v1AudioProcessorEditor::resized()
{
    auto r = getLocalBounds();

    //ts9_button.setBounds(r.getX() + 25, r.getY() + 25, 125, 35);
    //ts808_button.setBounds(r.getX() + 25, r.getY() + 25 + ts9_button.getHeight(), 125, 35);

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
