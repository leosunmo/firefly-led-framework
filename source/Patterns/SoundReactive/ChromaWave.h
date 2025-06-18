#pragma once
#include "../Pattern.h"
#include "../../Effects/SoundReactive/TwoTone.h"
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"
#include "../../kylarLEDs/Controllers/Controller.h"
#include "../../kylarLEDs/Input/InputManager.h"

// Structure to hold persistent settings for ChromaWave pattern
struct ChromaWaveSettings {
    double hue1 = 0;
    double hue2 = 0.66; // Default second hue (blue)
    float attackRate = 0.4f;
    float decayRate = 0.95f;
    int frequencyBand = 0; // 0=Low (bass)
    int audioCurve = 1; // 1=Square (default)
    float saturation = 1.0f;
    float audioThreshold = 0.01f;
    float beatSensitivity = 1.5f;
    float beatReaction = 0.8f;
    int visualizationMode = 0; // 0=Color Pulse (default)
    bool rgbMode = true; // Enable RGB interpolation by default for smoother transitions
    
    // Tracks whether a setting has been modified by user
    bool userModified = false;
};

class ChromaWave : public Pattern {
public:
    ChromaWave();
    void run() override;
    void init() override;
    void release() override;
    
    // Base parameters
    void setBrightness(float b);
    void setSpeed(float speed);
    
    // Set the base and reactive hues
    void setHue1(double hue);
    void setHue2(double hue);
    
    // Custom functions for pulse parameters
    void setReactivity(float value);
    
    // Audio response parameters
    
    // Set the attack rate (0.01-0.99) - lower = faster attack
    void setAttackRate(float value);
    
    // Set the decay rate (0.5-0.99) - higher = slower decay
    void setDecayRate(float value);
    
    // Set the frequency band to react to
    void setFrequencyBand(int bandIndex); // 0=Low, 1=Mid, 2=High, 3=Full
    
    // Set the audio processing curve
    void setAudioCurve(int curveIndex); // 0=Linear, 1=Square, 2=Cubic, 3=Log
    
    // Set color saturation (0.0-1.0)
    void setSaturation(float value);
    
    // Set audio threshold (0.0-0.5)
    void setAudioThreshold(float value);
    
    // Beat detection parameters
    
    // Set beat detection sensitivity (0.5-5.0)
    void setBeatSensitivity(float value);
    
    // Set beat reaction strength (0.0-1.0)
    void setBeatReaction(float value);
    
    // Set visualization mode
    // 0=Color Pulse, 1=Beat Flash, 2=Beat Expand, 3=Spectrum Flow
    void setVisualizationMode(int modeIndex);
    
    // Toggle RGB interpolation mode for smooth color transitions
    bool isRGBModeEnabled() const { return rgbMode; }
    void setRGBMode(bool enabled);
    
    // Return whether a beat was detected in the last frame
    bool isBeatDetected();
    
    const char* getName();

private:
    TwoTone* twoToneEffect;
    
    // Local cached copies of settings for the current instance
    double hue1;
    double hue2; 
    float attackRate;
    float decayRate;
    int frequencyBand;
    int audioCurve;
    float saturation;
    float audioThreshold;
    float beatSensitivity;
    float beatReaction;
    int visualizationMode;
    bool rgbMode; // Whether RGB interpolation is enabled
    
    // Static settings that persist between reinitializations
    static ChromaWaveSettings persistentSettings;
    
    // Load settings from persistent storage to local variables
    void loadSettings();
};
