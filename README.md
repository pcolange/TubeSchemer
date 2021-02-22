# Tube Schemer

This project houses various tube screamer audio plugins including replicas of original models and custom designs.

## Using the code 

All source code leverages the JUCE framework (https://juce.com/get-juce/download) which must be installed to compile and run the code. 

### Tested environment 
While JUCE is a cross-platform application framework, I have only tested and run the code on Windows 10. The current source has been tested for VST and standalone applications.

![alt text](https://github.com/philipcolangelo/TubeScreamer/blob/master/Media/Screenshot.png?raw=true)


## TODO list
- I left out an ON/OFF (power) switch, a common feature in other plugins. It might be a nice touch to add one at some point.
- Most of the tone knob audible changes occur from 9->10, this causes the processing to change rapidly and so some audible 'clicks' are heard when playing and changing the knob at the same time.
- As a VST, the plugin parameters (drive, tone, level) can be assigned to automation but the plugin lacks MIDI assignment which would be useful for external control. 
