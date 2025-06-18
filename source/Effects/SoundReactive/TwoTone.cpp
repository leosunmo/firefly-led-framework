#include "TwoTone.h"
#include "stdio.h"
#include <cmath>
#include <algorithm>
#include "../../kylarLEDs/Controllers/Sensors/Microphone/Microphone.h"

/**
 * ==================================================================
 * TwoTone Effect - Parameter and Mode Reference
 * ==================================================================
 * 
 * CORE PARAMETERS
 * ---------------
 * hue1: float (0.0-1.0) - Base color (displayed when audio is quiet)
 *       Default: 0.0 (red)
 *       
 * hue2: float (0.0-1.0) - Reactive color (mixed in as audio increases)
 *       Default: 0.66 (blue)
 *       
 * baseBrightness: double (0.05-0.95) - Baseline brightness level
 *       Default: 0.9
 *       
 * saturation: float (0.0-1.0) - Color saturation
 *       Default: 1.0 (full saturation)
 *       
 * pulseReactivity: float (1.0-10.0) - How strongly colors respond to audio
 *       Default: 5.0 (higher = more responsive)
 *       
 * attackRate: float (0.01-0.99) - How quickly effect responds to audio peaks
 *       Default: 0.4 (lower = faster attack)
 *       
 * decayRate: float (0.5-1.0) - How slowly effect fades when audio decreases
 *       Default: 0.95 (higher = slower decay)
 *       
 * audioThreshold: float (0.0-0.5) - Minimum audio level to trigger effects
 *       Default: 0.01
 *       
 * beatSensitivity: float (0.5-5.0) - How sensitive beat detection is
 *       Default: 1.5 (higher = more sensitive)
 *       
 * beatReaction: float (0.0-1.0) - How strongly visuals react to beats
 *       Default: 0.8
 *       
 * MODES
 * -----
 * FrequencyBand:
 *  - LOW: Bass frequencies
 *  - MID: Mid-range frequencies
 *  - HIGH: High frequencies (treble)
 *  - FULL: Full spectrum (average of all bands)
 *  Default: LOW
 * 
 * AudioCurve:
 *  - LINEAR: Raw audio input
 *  - SQUARE: Audio input squared (more responsive to peaks)
 *  - CUBIC: Audio input cubed (very responsive to peaks)
 *  - LOGARITHMIC: Log response (more detail in quiet sounds)
 *  Default: SQUARE
 * 
 * VisualizationMode:
 *  - COLOR_PULSE: Standard color pulsing between hue1 and hue2
 *  - BEAT_FLASH: Flash on beat detection (reduces saturation)
 *  - BEAT_EXPAND: Expand color transition on beat detection
 *  - SPECTRUM_FLOW: Flow through spectrum based on frequencies
 *  Default: COLOR_PULSE
 * 
 * ColorTransitionMode:
 *  - HSV_SHORTEST_PATH: Traditional HSV interpolation
 *  - RGB_DIRECT: Direct RGB interpolation for smoother transitions
 *  Default: HSV_SHORTEST_PATH (RGB_DIRECT enabled by default in ChromaWave)
 * ==================================================================
 */

void TwoTone::init()
{
    pixels = new std::vector<SoundPixel *>();
    int stripLen = LEDs::strip(0)->num();
    for (int i = 0; i < stripLen; i++)
    {
        SoundPixel *eff = new SoundPixel();
        eff->i = i;
        eff->hue = 0.0;
        eff->hueAdd = 0.0f; // Initialize hueAdd to 0 to prevent unwanted hue changes
        eff->strip = 0;
        Effect::engine->apply(eff);
        pixels->push_back(eff);
    }
    secTimer = new Timing();
    hueTimer = new Timing();
    brightnessTimer = new Timing();
    
    // Initialize with default values
    setHue1(0.0f);            // Red (shown when quiet)
    setHue2(0.66f);           // Blue (mixed in as audio increases)
    updateRGBColors();        // Set initial RGB colors
    
    pulseReactivity = 5.0f; // Higher values mean more color change for the same audio level
    pulsePosition = 0.0f;   // Start at first hue
    
    attackRate = 0.4f;      // Fast attack for responsive peaks
    decayRate = 0.95f;      // Slow decay for natural fadeout
    
    freqBand = FrequencyBand::LOW;    // Default to bass frequencies
    audioCurve = AudioCurve::SQUARE;  // Default to square curve for better response
    saturation = 1.0f;      // Full saturation by default
    audioThreshold = 0.01f; // Minimal threshold to avoid noise
    
    // Beat detection initialization
    beatSensitivity = 1.5f;
    beatReaction = 0.8f;
    beatDetected = false;
    beatThreshold = 0.1f;
    beatDecay = 0.95f;
    framesSinceLastBeat = 0;
    minBeatInterval = 10;
    
    // Pre-fill energy history with zeros
    energyHistory.clear();
    for (int i = 0; i < 30; i++) { // 30 frames history for beat detection
        energyHistory.push_back(0.0f);
    }
    
    // Visualization mode defaults
    visMode = VisualizationMode::COLOR_PULSE;
    beatFlashIntensity = 0.0f;
    beatExpandProgress = 0.0f;
}

// New helper method to process audio based on frequency band and curve settings
float TwoTone::processAudio(float rawAudio)
{
    // Get the appropriate frequency band from the microphone
    float audioInput = 0.0f;
    
    // Use available frequency band methods
    switch (freqBand) {
        case FrequencyBand::LOW:
            audioInput = Microphone::getLowNormal();
            break;
        case FrequencyBand::MID:
            audioInput = Microphone::getMidNormal();
            break;
        case FrequencyBand::HIGH:
            audioInput = Microphone::getHighNormal();
            break;
        case FrequencyBand::FULL:
            // Use average of all available bands for full spectrum response
            audioInput = (Microphone::getLowNormal() + Microphone::getMidNormal() + Microphone::getHighNormal()) / 3.0f;
            break;
    }

    // Apply audio threshold
    if (audioInput < audioThreshold) {
        audioInput = 0.0f;
    }
    
    // Apply the selected audio curve
    float processedAudio = 0.0f;
    
    switch (audioCurve) {
        case AudioCurve::LINEAR:
            processedAudio = audioInput;
            break;
        case AudioCurve::SQUARE:
            processedAudio = audioInput * audioInput;
            break;
        case AudioCurve::CUBIC:
            processedAudio = audioInput * audioInput * audioInput;
            break;
        case AudioCurve::LOGARITHMIC:
            // Logarithmic response gives more detail to quiet sounds
            if (audioInput > 0.01f) { // Avoid log(0)
                processedAudio = 0.3f * logf(1.0f + 10.0f * audioInput);
                // Scale to approximate 0-1 range
                processedAudio = std::min(1.0f, processedAudio);
            }
            break;
    }
    
    return processedAudio;
}

// Beat detection algorithm
void TwoTone::detectBeat(float audioLevel) {
    // Reset beat detected flag at the start of each frame
    beatDetected = false;
    
    // Increment frames since last beat counter
    framesSinceLastBeat++;
    
    // Add the current energy level to our history and remove the oldest one
    energyHistory.push_back(audioLevel);
    if (energyHistory.size() > 30) { // Keep a 30-frame history
        energyHistory.pop_front();
    }
    
    // Calculate the average energy over recent history
    float sum = 0.0f;
    for (float e : energyHistory) {
        sum += e;
    }
    float averageEnergy = sum / energyHistory.size();
    
    // Adjust beat threshold with sensitivity and decay
    beatThreshold = beatThreshold * beatDecay + (1.0f - beatDecay) * averageEnergy * beatSensitivity;
    
    // If we're past the minimum beat interval and the current level exceeds the threshold
    if (framesSinceLastBeat >= minBeatInterval && audioLevel > beatThreshold && audioLevel > 0.05f) {
        beatDetected = true;
        framesSinceLastBeat = 0;
        
        // Print debug info about the beat
        printf("BEAT detected! Level: %f, Threshold: %f\n", audioLevel, beatThreshold);
    }
}

// Apply the standard color pulse visualization
void TwoTone::run()
{
    double seconds = secTimer->takeSeconds();
    
    // Process audio input according to settings
    float processedAudio = processAudio(micVal);
    
    // Perform beat detection
    detectBeat(processedAudio);
    
    // Apply the current visualization mode
    applyVisualization(processedAudio);
    
    double micAdd;
    if (smoothingMicVal <= 0.0)
    {
        micAdd = processedAudio; // If smoothingMicVal is 0, use processed audio directly
    } else
    {
        micAdd = processedAudio / smoothingMicVal; // trying to even out timing
    }

    int avgLoops = 0;
    avgLoops = hueTimer->takeMsEvery(1);
    for (int i = 0; i < avgLoops; i++)
    {
        hueAdd = (hueAdd * 100.0 + micAdd) / 101.0;
    }

    if (micAdd > hueAdd)
    {
        hueAdd = micAdd;
    }

    // Scale the brightness increase based on audio level
    double tempBrightnessIncrease = processedAudio * (1.0 - baseBrightness) * baseBrightness;
    
    // Add extra brightness boost on beat detection for beat-reactive modes
    if (beatDetected && (visMode == VisualizationMode::BEAT_FLASH || visMode == VisualizationMode::BEAT_EXPAND)) {
        tempBrightnessIncrease += beatReaction * (1.0 - baseBrightness) * 0.25f;
    }

    // Set proposed brightness as the sum of baseBrightness and the scaled temporary increase
    double proposedBrightness = baseBrightness + tempBrightnessIncrease;
    proposedBrightness = std::min(1.0, proposedBrightness); // Ensure brightness doesn't exceed 1.0

    avgLoops = brightnessTimer->takeMsEvery(1);
    // Iterate through the number of ms since last to smooth out the brightness.
    for (int i = 0; i < avgLoops; i++)
    {
        brightness = (brightness * 200.0 + proposedBrightness) / 201.0;
    }

    // Check if the proposed brightness is greater than the current brightness
    // and update it if necessary.
    if (proposedBrightness > brightness)
    {
        brightness = proposedBrightness;
    }

    // Iterate through all the pixels and set their values.
    for (SoundPixel *pixel : *pixels)
    {
        pixel->brightness = brightness;
        pixel->micVal = processedAudio;
        
        // Hue and saturation are set by the current visualization mode
        // We don't need to set hueAdd here as we're explicitly setting it to 0 in each visualization method
    }
}

// Apply the selected visualization mode
void TwoTone::applyVisualization(float processedAudio) {
    // Select visualization mode
    switch (visMode) {
        case VisualizationMode::COLOR_PULSE:
            // Standard color pulse - existing code with some enhancements
            {
                // Scale the mic value by the reactivity
                float scaledMicValue = processedAudio * pulseReactivity;
                
                // Constrain to a reasonable range (0.0-1.0)
                scaledMicValue = std::min(1.0f, std::max(0.0f, scaledMicValue));
                
                // Quick rise on audio peaks, slower falloff when quiet
                if (scaledMicValue > pulsePosition) {
                    // Fast attack when audio increases
                    pulsePosition = pulsePosition * attackRate + scaledMicValue * (1.0f - attackRate);
                } else {
                    // Slower decay when audio decreases
                    pulsePosition = pulsePosition * decayRate;
                }
                
                // In very quiet situations, ensure it returns to default position
                if (processedAudio < audioThreshold && pulsePosition < audioThreshold) {
                    pulsePosition = 0.0f;
                }
                
                // Set colors based on the selected transition mode
                if (transitionMode == ColorTransitionMode::RGB_DIRECT) {
                    // Use RGB interpolation for smoother transitions
                    rgb_t currentRGB = lerpRGB(rgb1, rgb2, pulsePosition);
                    
                    // Set the color for all pixels using RGB mode
                    for (SoundPixel *pixel : *pixels) {
                        pixel->setRGB(currentRGB.r, currentRGB.g, currentRGB.b);
                        pixel->brightness = brightness;
                    }
                } else {
                    // Traditional HSV interpolation using shortest path
                    float hueDiff = hue2 - hue1;
                    if (hueDiff > 0.5f) hueDiff -= 1.0f;
                    if (hueDiff < -0.5f) hueDiff += 1.0f;
                    
                    // Linear interpolation between the two hues
                    float currentHue = hue1 + hueDiff * pulsePosition;
                    
                    // Normalize to [0,1] range
                    if (currentHue < 0.0f) currentHue += 1.0f;
                    if (currentHue >= 1.0f) currentHue -= 1.0f;
                    
                    // Set the color for all pixels
                    for (SoundPixel *pixel : *pixels) {
                        pixel->hue = currentHue;
                        pixel->saturation = saturation;
                        pixel->hueAdd = 0.0f; // Prevent SoundPixel from incrementing the hue we just set
                    }
                }
            }
            break;
            
        case VisualizationMode::BEAT_FLASH:
            applyBeatFlash(processedAudio);
            break;
            
        case VisualizationMode::BEAT_EXPAND:
            applyBeatExpand(processedAudio);
            break;
            
        case VisualizationMode::SPECTRUM_FLOW:
            applySpectrumFlow(processedAudio);
            break;
    }
}

// Apply beat flash visualization
void TwoTone::applyBeatFlash(float processedAudio) {
    // On beat detection, set flash intensity to maximum
    if (beatDetected) {
        beatFlashIntensity = 1.0f;
    }
    
    // Calculate the base hue using the standard color pulse technique
    float scaledMicValue = processedAudio * pulseReactivity;
    scaledMicValue = std::min(1.0f, std::max(0.0f, scaledMicValue));
    
    // Process the pulse position similar to normal mode
    if (scaledMicValue > pulsePosition) {
        // Fast attack when audio increases
        pulsePosition = pulsePosition * attackRate + scaledMicValue * (1.0f - attackRate);
    } else {
        // Slower decay when audio decreases
        pulsePosition = pulsePosition * decayRate;
    }
    
    // Decay flash intensity
    beatFlashIntensity *= 0.8f; // Fast decay for flash effect
    
    // Set the color with flash effect (moving toward white on beat)
    float flashSaturation = saturation * (1.0f - beatFlashIntensity * beatReaction);
    
    if (transitionMode == ColorTransitionMode::RGB_DIRECT) {
        // RGB interpolation
        rgb_t currentRGB = lerpRGB(rgb1, rgb2, pulsePosition);
        
        // Apply flash effect - move toward white by increasing all channels
        float flashEffect = beatFlashIntensity * beatReaction;
        rgb_t flashRGB = currentRGB;
        flashRGB.r = currentRGB.r + (1.0f - currentRGB.r) * flashEffect;
        flashRGB.g = currentRGB.g + (1.0f - currentRGB.g) * flashEffect;
        flashRGB.b = currentRGB.b + (1.0f - currentRGB.b) * flashEffect;
        
        for (SoundPixel *pixel : *pixels) {
            pixel->setRGB(flashRGB.r, flashRGB.g, flashRGB.b);
            pixel->brightness = brightness;
        }
    } else {
        // Traditional HSV interpolation
        float hueDiff = hue2 - hue1;
        if (hueDiff > 0.5f) hueDiff -= 1.0f;
        if (hueDiff < -0.5f) hueDiff += 1.0f;
        
        // Linear interpolation between the two hues
        float currentHue = hue1 + hueDiff * pulsePosition;
        
        // Normalize to [0,1] range
        if (currentHue < 0.0f) currentHue += 1.0f;
        if (currentHue >= 1.0f) currentHue -= 1.0f;
        
        for (SoundPixel *pixel : *pixels) {
            pixel->hue = currentHue;
            pixel->saturation = flashSaturation; // Lower saturation = more white
            pixel->hueAdd = 0.0f; // Prevent SoundPixel from incrementing the hue we just set
        }
    }
}

// Apply beat expand visualization
void TwoTone::applyBeatExpand(float processedAudio) {
    // On beat detection, reset expand progress
    if (beatDetected) {
        beatExpandProgress = 0.0f;
    }
    
    // Progress the expansion effect
    beatExpandProgress = std::min(1.0f, beatExpandProgress + 0.05f);
    
    // Calculate base color using normal pulse
    float scaledMicValue = processedAudio * pulseReactivity;
    scaledMicValue = std::min(1.0f, std::max(0.0f, scaledMicValue));
    
    // Process the pulse position
    if (scaledMicValue > pulsePosition) {
        pulsePosition = pulsePosition * attackRate + scaledMicValue * (1.0f - attackRate);
    } else {
        pulsePosition = pulsePosition * decayRate;
    }
    
    // Set different colors along the strip to create an expanding effect
    int stripLen = pixels->size();
    for (int i = 0; i < stripLen; i++) {
        SoundPixel *pixel = (*pixels)[i];
        
        // Calculate position in strip (0.0-1.0)
        float pos = static_cast<float>(i) / stripLen;
        
        // Calculate distance from center
        float center = 0.5f;
        float distFromCenter = std::abs(pos - center);
        
        // If the pixel is within the expanding circle
        float expandRadius = beatExpandProgress * 0.5f; // Expands to full strip width
        
        // For beat expand effect - create moving wave from center
        if (transitionMode == ColorTransitionMode::RGB_DIRECT) {
            if (distFromCenter < expandRadius) {
                // Use color2 for pixels in the expanding wave
                pixel->setRGB(rgb2.r, rgb2.g, rgb2.b);
            } else {
                // Use interpolated color for other pixels
                rgb_t currentRGB = lerpRGB(rgb1, rgb2, pulsePosition);
                pixel->setRGB(currentRGB.r, currentRGB.g, currentRGB.b);
            }
            
            pixel->brightness = brightness;
        } else {
            if (distFromCenter < expandRadius) {
                // Use hue2 for pixels in the expanding wave
                pixel->hue = hue2;
            } else {
                // Use hue1 + normal pulse mapping for other pixels
                float hueDiff = hue2 - hue1;
                if (hueDiff > 0.5f) hueDiff -= 1.0f;
                if (hueDiff < -0.5f) hueDiff += 1.0f;
                
                float currentHue = hue1 + hueDiff * pulsePosition;
                if (currentHue < 0.0f) currentHue += 1.0f;
                if (currentHue >= 1.0f) currentHue -= 1.0f;
                
                pixel->hue = currentHue;
            }
            
            // Keep saturation consistent
            pixel->saturation = saturation;
            // Ensure hueAdd is zeroed out to prevent unwanted hue changes
            pixel->hueAdd = 0.0f;
        }
    }
}

// Apply spectrum flow visualization
void TwoTone::applySpectrumFlow(float processedAudio) {
    // Get levels from different frequency bands
    float lowLevel = Microphone::getLowNormal();
    float midLevel = Microphone::getMidNormal();
    float highLevel = Microphone::getHighNormal();
    
    // No need for randomization now that we have real mid-range data
    
    // Square them for better response
    lowLevel = lowLevel * lowLevel;
    midLevel = midLevel * midLevel;
    highLevel = highLevel * highLevel;
    
    // Map each frequency band to a different color
    float lowHue = 0.0f;       // Red for bass
    float midHue = 0.33f;      // Green for mids
    float highHue = 0.66f;     // Blue for highs
    
    // Process the pulse position similar to normal mode
    float scaledMicValue = processedAudio * pulseReactivity;
    scaledMicValue = std::min(1.0f, std::max(0.0f, scaledMicValue));
    
    if (scaledMicValue > pulsePosition) {
        pulsePosition = pulsePosition * attackRate + scaledMicValue * (1.0f - attackRate);
    } else {
        pulsePosition = pulsePosition * decayRate;
    }
    
    // Set different colors for different sections of the strip
    int stripLen = pixels->size();
    int lowSection = stripLen / 3;
    int midSection = 2 * lowSection;
    
    for (int i = 0; i < stripLen; i++) {
        SoundPixel *pixel = (*pixels)[i];
        
        // Assign different frequency bands to different sections
        if (transitionMode == ColorTransitionMode::RGB_DIRECT) {
            // Convert hues to RGB for each section
            hsv_t lowHsv = {lowHue, saturation, 1.0f};
            hsv_t midHsv = {midHue, saturation, 1.0f};
            hsv_t highHsv = {highHue, saturation, 1.0f};
            
            rgb_t lowRgb = ColorUtil::hsv2rgb(lowHsv);
            rgb_t midRgb = ColorUtil::hsv2rgb(midHsv);
            rgb_t highRgb = ColorUtil::hsv2rgb(highHsv);
            
            if (i < lowSection) {
                // Low frequency section
                pixel->setRGB(lowRgb.r, lowRgb.g, lowRgb.b);
            } 
            else if (i < midSection) {
                // Mid frequency section
                pixel->setRGB(midRgb.r, midRgb.g, midRgb.b);
            } 
            else {
                // High frequency section
                pixel->setRGB(highRgb.r, highRgb.g, highRgb.b);
            }
            
            pixel->brightness = brightness;
        } else {
            if (i < lowSection) {
                // Low frequency section - set hue and make brightness reactive to bass
                pixel->hue = lowHue;
                pixel->saturation = saturation;
                pixel->hueAdd = 0.0f; // Prevent SoundPixel from incrementing the hue
            } 
            else if (i < midSection) {
                // Mid frequency section
                pixel->hue = midHue;
                pixel->saturation = saturation;
                pixel->hueAdd = 0.0f; // Prevent SoundPixel from incrementing the hue
            } 
            else {
                // High frequency section
                pixel->hue = highHue;
                pixel->saturation = saturation;
                pixel->hueAdd = 0.0f; // Prevent SoundPixel from incrementing the hue
            }
        }
    }
}

void TwoTone::resetSecTimer()
{
    secTimer->reset();
}

void TwoTone::setHues(float hue1, float hue2)
{
    setHue1(hue1);
    setHue2(hue2);
}

void TwoTone::setHue1(float value)
{
    // Normalize hue value to [0,1] range
    hue1 = value < 0 ? fmod(value, 1.0f) + 1.0f : fmod(value, 1.0f);
    
    // Update RGB colors whenever hue1 changes
    updateRGBColors();
}

void TwoTone::setHue2(float value)
{
    // Normalize hue value to [0,1] range
    hue2 = value < 0 ? fmod(value, 1.0f) + 1.0f : fmod(value, 1.0f);
    
    // Update RGB colors whenever hue2 changes
    updateRGBColors();
}

void TwoTone::setReactivity(float value)
{
    // Clamp reactivity to [0,10] range (allowing much higher values for stronger response)
    pulseReactivity = std::min(10.0f, std::max(0.0f, value));
}

// Parameter methods
void TwoTone::setAttackRate(float value)
{
    // Lower values = faster attack, but should stay in reasonable range
    attackRate = std::min(0.99f, std::max(0.01f, value));
}

void TwoTone::setDecayRate(float value)
{
    // Higher values = slower decay
    // Constrain input to [0.5, 1.0] range first
    float constrainedValue = std::min(1.0f, std::max(0.5f, value));
    
    // Apply non-linear mapping: 
    // Map input range [0.5, 1.0] to effective decay range [0.85, 0.99]
    // This makes lower values still have a reasonable decay rather than instant drops
    float minOutput = 0.85f;  // Minimum effective decay rate (at value=0.5)
    float maxOutput = 0.99f;  // Maximum effective decay rate (at value=1.0)
    
    // Normalize input to [0,1] range and apply linear mapping to output range
    float normalizedInput = (constrainedValue - 0.5f) / 0.5f;
    decayRate = minOutput + normalizedInput * (maxOutput - minOutput);
}

void TwoTone::setFrequencyBand(FrequencyBand band)
{
    freqBand = band;
}

void TwoTone::setAudioCurve(AudioCurve curve)
{
    audioCurve = curve;
}

void TwoTone::setSaturation(float value)
{
    // Clamp saturation to [0,1] range
    saturation = std::min(1.0f, std::max(0.0f, value));
    
    // Update RGB colors when saturation changes
    updateRGBColors();
}

void TwoTone::setAudioThreshold(float value)
{
    // Clamp threshold to [0,0.5] range (higher would make effect unresponsive)
    audioThreshold = std::min(0.5f, std::max(0.0f, value));
}

// New parameter methods for beat detection
void TwoTone::setBeatSensitivity(float value)
{
    // Clamp beat sensitivity to [0.5,5.0] range
    beatSensitivity = std::min(5.0f, std::max(0.5f, value));
}

void TwoTone::setBeatReaction(float value)
{
    // Clamp beat reaction strength to [0,1] range
    beatReaction = std::min(1.0f, std::max(0.0f, value));
}

void TwoTone::setVisualizationMode(VisualizationMode mode)
{
    visMode = mode;
    
    // Reset state when changing modes
    beatFlashIntensity = 0.0f;
    beatExpandProgress = 0.0f;
    pulsePosition = 0.0f;
}

// Enable or disable RGB mode for direct color interpolation
void TwoTone::enableRGBMode(bool enable)
{
    setColorTransitionMode(enable ? ColorTransitionMode::RGB_DIRECT : ColorTransitionMode::HSV_SHORTEST_PATH);
    
    // Print debug info
    printf("TwoTone: %s RGB interpolation mode\n", enable ? "Enabled" : "Disabled");
}

// Update RGB colors from HSV values
void TwoTone::updateRGBColors()
{
    // Convert hue1, saturation, full brightness to RGB
    hsv_t hsv1 = {hue1, saturation, 1.0f};
    rgb1 = ColorUtil::hsv2rgb(hsv1);
    
    // Convert hue2, saturation, full brightness to RGB
    hsv_t hsv2 = {hue2, saturation, 1.0f};
    rgb2 = ColorUtil::hsv2rgb(hsv2);
    
    printf("TwoTone: Updated RGB colors from HSV - RGB1(%.2f,%.2f,%.2f), RGB2(%.2f,%.2f,%.2f)\n",
           rgb1.r, rgb1.g, rgb1.b, rgb2.r, rgb2.g, rgb2.b);
}

// Linear interpolation between two RGB colors
rgb_t TwoTone::lerpRGB(const rgb_t& a, const rgb_t& b, float t) const
{
    rgb_t result;
    
    // Clamp t to [0,1] range
    t = std::min(1.0f, std::max(0.0f, t));
    
    // Linear interpolation for each color channel
    result.r = a.r + t * (b.r - a.r);
    result.g = a.g + t * (b.g - a.g);
    result.b = a.b + t * (b.b - a.b);
    
    return result;
}

// Set color transition mode
void TwoTone::setColorTransitionMode(ColorTransitionMode mode)
{
    if (transitionMode != mode) {
        transitionMode = mode;
        
        // If switching to RGB mode, make sure RGB colors are updated
        if (mode == ColorTransitionMode::RGB_DIRECT) {
            updateRGBColors();
            
            printf("TwoTone: Switched to RGB direct interpolation mode\n");
        } else {
            printf("TwoTone: Switched to HSV shortest path interpolation mode\n");
        }
    }
}

TwoTone::~TwoTone()
{
    delete (secTimer);
    delete (hueTimer);
    delete (brightnessTimer);
    delete (pixels);
}
