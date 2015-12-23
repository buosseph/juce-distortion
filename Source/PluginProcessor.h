#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginParameter.h"
#include "Distortion.h"

/** Helper Macros
    
    All audio samples are represented in unit voltage (uV) as a float in the
    range of -1.0 to 1.0. To convert an audio sample into a decibel (dB) value
    pass the sample into `dB(x)`. The returned output can be used for audio
    analysis and dynamic procesing. To convert a decibel value back to unit
    voltage for output, pass the value into `uV(x)`. Do not output your samples
    in decibels.

    It is important to keep track of when your values are in decibels or
    unit voltage. Be sure to label your variables accordingly.
*/
#define dB(x) 20.0 * ((x) > 0.00001 ? log10(x) : -5.0)  // uV -> dB
#define uV(x) pow(10.0, (x) / 20.0)                     // dB -> uV

//==============================================================================
/**
*/
class PluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    PluginAudioProcessor();
    ~PluginAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;
    const String getInputChannelName (int channelIndex) const override;
    const String getOutputChannelName (int channelIndex) const override;
    bool isInputChannelStereoPair (int index) const override;
    bool isOutputChannelStereoPair (int index) const override;

    bool   acceptsMidi() const override;
    bool   producesMidi() const override;
    bool   silenceInProducesSilenceOut() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int  getNumPrograms() override;
    int  getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Parameters
    AudioProcessorParameter* mode;
    AudioProcessorParameter* drive;
    AudioProcessorParameter* threshold;
    AudioProcessorParameter* mix;
    
private:
    ScopedPointer<Distortion> processor;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
