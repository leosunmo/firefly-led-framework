#pragma once
#include "../Effect.h"
#include "../../kylarLEDs/Utility/Timing.h"
#include "../../Effects/SoundReactive/SoundPixel.h"
#include "../../kylarLEDs/LEDInterface/colorUtil.h"
#include <vector>
#include <deque>

// Enum for frequency band selection
enum class FrequencyBand {
    LOW,    // Bass frequencies
    MID,    // Mid-range frequencies
    HIGH,   // High frequencies (treble)
    FULL    // Full spectrum (average of LOW, MID, and HIGH)
};

// Enum for audio response curve
enum class AudioCurve {
    LINEAR,     // Raw audio input (linear)
    SQUARE,     // Square the audio input (more responsive to peaks)
    CUBIC,      // Cube the audio input (very responsive to peaks)
    LOGARITHMIC // Logarithmic response (more detail in quiet sounds)
};

// Enum for visualization mode
enum class VisualizationMode {
    COLOR_PULSE,    // Standard color pulsing between hue1 and hue2
    BEAT_FLASH,     // Flash on beat detection
    BEAT_EXPAND,    // Expand color transition on beat detection
    SPECTRUM_FLOW   // Flow through spectrum based on frequencies
};

// Enum for color transition mode
enum class ColorTransitionMode {
    HSV_SHORTEST_PATH, // Traditional HSV interpolation using shortest path around the color wheel
    RGB_DIRECT         // Direct RGB interpolation for smoother transitions between colors
};

class TwoTone : public Effect {
    
public:
    using Effect::Effect;
    void run();
    void init();
    void resetSecTimer();

    ~TwoTone();

    float micVal = 0;
    float smoothingMicVal = 50;
    float hueAdd = 0;
    
    // Adjust this value to set the baseline brightness level.
    // Maximum should probably be around 0.95.
    // Minimum should be around 0.05.
    double baseBrightness = 0.9;
    float brightness = 0;
    
    // Set the two hues to cycle between
    // hue1, hue2: Hue values in the range [0,1] where:
    //   0.0 = red, 0.33 = green, 0.66 = blue
    // Note: The LEDStrip system uses a non-linear hue mapping that emphasizes green and blue
    // ranges while compressing red and yellow. These methods include an inverse remapping to
    // ensure that the colors displayed correspond to standard HSV color wheel expectations.
    void setHues(float hue1, float hue2);
    
    /**
     * Get the first hue in the color cycle (what's displayed when audio is quiet)
     * Note: The returned hue is the internal value which is inverse-mapped
     * @return The first hue value (0.0-1.0)
     */
    float getHue1() const { return hue1; }
    
    /**
     * Set the first hue in the color cycle (what's displayed when audio is quiet)
     * This applies inverse mapping to ensure colors appear as expected
     * @param value The desired hue (0.0-1.0) to display
     */
    void setHue1(float value);
    
    /**
     * Get the second hue in the color cycle (what's blended in when audio increases)
     * Note: The returned hue is the internal value which is inverse-mapped
     * @return The second hue value (0.0-1.0)
     */
    float getHue2() const { return hue2; }
    
    /**
     * Set the second hue in the color cycle (what's blended in when audio increases)
     * This applies inverse mapping to ensure colors appear as expected
     * @param value The desired hue (0.0-1.0) to display
     */
    void setHue2(float value);
    
    // Set pulse reactivity (recommended range: 1.0-3.0)
    // Higher values make the effect more responsive to audio
    float getReactivity() const { return pulseReactivity; }
    void setReactivity(float value);
    
    // Set the attack rate (0.0-1.0)
    // Lower values make the effect respond faster to audio peaks
    float getAttackRate() const { return attackRate; }
    void setAttackRate(float value);
    
    // Set the decay rate (0.0-1.0)
    // Higher values make the effect fade more slowly when audio decreases
    float getDecayRate() const { return decayRate; }
    void setDecayRate(float value);
    
    // Set the frequency band to respond to
    FrequencyBand getFrequencyBand() const { return freqBand; }
    void setFrequencyBand(FrequencyBand band);
    
    // Set the audio response curve
    AudioCurve getAudioCurve() const { return audioCurve; }
    void setAudioCurve(AudioCurve curve);
    
    // Set the color saturation (0.0-1.0)
    float getSaturation() const { return saturation; }
    void setSaturation(float value);
    
    // Set the audio threshold (0.0-1.0)
    // Audio values below this threshold won't trigger color changes
    float getAudioThreshold() const { return audioThreshold; }
    void setAudioThreshold(float value);
    
    // Beat detection sensitivity (1.0-5.0)
    // Higher values make beat detection more sensitive
    float getBeatSensitivity() const { return beatSensitivity; }
    void setBeatSensitivity(float value);
    
    // Beat reaction strength (0.0-1.0)
    // How strongly the visualization reacts to detected beats
    float getBeatReaction() const { return beatReaction; }
    void setBeatReaction(float value);
    
    // Set visualization mode
    VisualizationMode getVisualizationMode() const { return visMode; }
    void setVisualizationMode(VisualizationMode mode);
    
    // Check if a beat was detected in the last frame
    bool isBeatDetected() const { return beatDetected; }
        
    // Set the color transition mode
    ColorTransitionMode getColorTransitionMode() const { return transitionMode; }
    void setColorTransitionMode(ColorTransitionMode mode);
    
    /**
     * Enable or disable RGB direct interpolation mode.
     * 
     * When enabled, colors will transition directly in RGB space between hue1 and hue2,
     * avoiding the unwanted intermediate colors that can appear with HSV interpolation.
     * 
     * @param enable True to use RGB interpolation, false to use HSV
     */
    void enableRGBMode(bool enable);
    
private:
    Timing *hueTimer;
    Timing *secTimer;
    Timing *brightnessTimer;
    std::vector<SoundPixel*> *pixels;
    
    // Process audio input based on frequency band and curve settings
    float processAudio(float rawAudio);
    
    // Detect beats in audio signal
    void detectBeat(float audioLevel);
    
    // Apply the current visualization mode
    void applyVisualization(float processedAudio);
    
    // Beat flash visualization
    void applyBeatFlash(float processedAudio);
    
    // Beat expand visualization
    void applyBeatExpand(float processedAudio);
    
    // Spectrum flow visualization
    void applySpectrumFlow(float processedAudio);
      
    // Pulse transition position (0 to 1)
    float pulsePosition = 0.0f;
    
    // Two-hue pulse parameters
    float hue1 = 0.0f;             // First hue (0-1.0)
    float hue2 = 0.66f;            // Second hue (0-1.0) 
    float pulseReactivity = 2.0f;  // Audio reactivity (higher = more responsive)
    
    // Attack/decay parameters for audio response
    float attackRate = 0.4f;       // How quickly effect responds to audio peaks (lower = faster)
    float decayRate = 0.95f;       // How slowly effect fades when audio decreases (higher = slower)
    
    // Audio processing parameters
    FrequencyBand freqBand = FrequencyBand::LOW;   // Which frequency band to use
    AudioCurve audioCurve = AudioCurve::SQUARE;    // Audio response curve
    float saturation = 1.0f;       // Color saturation (1.0 = full saturation)
    float audioThreshold = 0.01f;  // Minimum audio level to trigger effects
    
    // Beat detection parameters
    float beatSensitivity = 1.5f;  // How sensitive the beat detection is
    float beatReaction = 0.8f;     // How strongly the visualization reacts to beats
    bool beatDetected = false;     // Whether a beat was detected in the current frame
    
    // Variables for beat detection algorithm
    std::deque<float> energyHistory;  // Store recent energy levels for comparison
    float beatThreshold = 0.1f;       // Adaptive threshold for beat detection
    float beatDecay = 0.95f;          // How quickly the threshold decays
    int framesSinceLastBeat = 0;      // Frames since the last detected beat
    int minBeatInterval = 10;         // Minimum frames between beats to avoid false positives
    
    // Current visualization mode
    VisualizationMode visMode = VisualizationMode::COLOR_PULSE;
    
    // Variables for visualizations
    float beatFlashIntensity = 0.0f;  // Current intensity of beat flash (0.0-1.0)
    float beatExpandProgress = 0.0f;  // Progress of beat expansion effect (0.0-1.0)
    
    // RGB colors for direct interpolation
    rgb_t rgb1; // First color in RGB space
    rgb_t rgb2; // Second color in RGB space
    
    // Color transition mode
    ColorTransitionMode transitionMode = ColorTransitionMode::HSV_SHORTEST_PATH;
    
    // RGB interpolation helper methods
    void updateRGBColors();
    rgb_t lerpRGB(const rgb_t& a, const rgb_t& b, float t) const;
};
